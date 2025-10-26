/*
 * @file main.cpp
 * @brief TigerTagScale - Balance connectée ESP32 avec portail captif
 * @version 1.1.0 - Interface web servie depuis LittleFS
 */

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <MFRC522.h>
#include <SPI.h>
#include <LittleFS.h>  // ← AJOUTÉ pour filesystem

// ============================================================================
// CONFIGURATION MATERIELLE
// ============================================================================

// OLED (I2C)
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

// RFID RC522 (SPI)
#define RC522_SS    5
#define RC522_RST   27

// HX711 Balance
#define HX711_DOUT  32
#define HX711_SCK   33

// LED Heartbeat
#define LED_PIN     2

// WebSocket update interval (ms)
#define WS_UPDATE_INTERVAL_MS 250

// mDNS
#define MDNS_NAME   "tigerscale"

// mDNS lifecycle helpers
void startMDNS();
void onWiFiEvent(WiFiEvent_t event);

// ============================================================================
// OBJETS GLOBAUX
// ============================================================================

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
HX711 scale;
MFRC522 rfid(RC522_SS, RC522_RST);
AsyncWebServer server(80);
Preferences prefs;
WiFiManager wm;

// ============================================================================
// VARIABLES DE CONFIGURATION
// ============================================================================

String apiKey = "";
String apiDisplayName = "";     // cached display name for validated API key
bool apiValid = false;          // last known validation state
uint32_t lastApiBroadcastMs = 0; // WS broadcast throttle for apiStatus
float calibrationFactor = 406;
float currentWeight = 0.0;
String lastUID = "";       // decimal UID for API/UI
String lastUIDHex = "";    // hex UID for logs/debug

bool wifiConnected = false;
bool cloudOK = false; // true if health endpoint returns {"ok":true}

// --- Auto push configuration ---
const float STABLE_EPSILON_G = 1.0f;        // max delta considered stable (g)
const uint32_t STABLE_WINDOW_MS = 1500;     // time window to be stable before sending
const float MIN_WEIGHT_TO_SEND_G = 5.0f;    // ignore tiny weights
const float RESEND_DELTA_G = 2.0f;          // change required to resend (g)
const uint32_t RESEND_COOLDOWN_MS = 15000;  // minimal delay between sends (ms)

// State
float lastPushedWeight = NAN;
uint32_t stableSinceMs = 0;
float stableCandidate = NAN;
uint32_t lastPushMs = 0;

// ============================================================================
// AFFICHAGE OLED
// ============================================================================

void displayMessage(String line1, String line2 = "", String line3 = "", String line4 = "") {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println(line1);

    if (line2.length() > 0) {
        display.setCursor(0, 16);
        display.println(line2);
    }

    if (line3.length() > 0) {
        display.setCursor(0, 32);
        display.println(line3);
    }

    if (line4.length() > 0) {
        display.setCursor(0, 48);
        display.println(line4);
    }

    display.display();
}

void displayWeight(float weight, const String& uid = "");

bool checkServerHealth();
bool pushWeightToCloud(float w);
void handleAutoPush(float w);
bool validateApiKeyFirmware(const String& key, String& displayNameOut);

void displayWeight(float weight, const String& uid) {
    display.clearDisplay();
    
     // En-tête avec titre et statut WiFi
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Tiger-Scale");
    
    display.setTextSize(1);
    display.setCursor(100, 0);
    display.println(wifiConnected ? "WiFi" : "----");
    
    // Poids au centre (grande taille) — entier uniquement
    int wInt = (int)(weight + (weight >= 0 ? 0.5f : -0.5f));
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(wInt);
    display.println(" g");
    
    // UID
    if (uid.length() > 0) {
        display.setTextSize(1);
        display.setCursor(0, 45);
        display.print("UID:");
        display.println(uid);
    }
    
    // IP en dessous de l'UID
    display.setTextSize(1);
    display.setCursor(0, 56);
    if (wifiConnected) {
        display.print("IP: ");
        display.println(WiFi.localIP().toString().c_str());
    }
    
    display.display();
}

// ============================================================================
// PORTAIL CAPTIF & CONFIGURATION
// ============================================================================

void configModeCallback(WiFiManager *myWiFiManager) {
    displayMessage(
        "CONFIG MODE",
        "Connect to WiFi",
        "TigerScale-Setup"
    );
}

void saveConfigCallback() {
    displayMessage("Saving...", "Wi‑Fi config OK", "Reconnecting...");
    delay(800);
}

void setupWiFi() {
    WiFiManagerParameter custom_api_key("apikey", "API Key (optionnel)", apiKey.c_str(), 64);
    
    wm.addParameter(&custom_api_key);
    wm.setAPCallback(configModeCallback);
    wm.setSaveConfigCallback(saveConfigCallback);
    wm.setConfigPortalTimeout(180);
    
    displayMessage("Connecting to WiFi...", "Waiting...");
    
    if (!wm.autoConnect("TigerScale-Setup")) {
        displayMessage("WiFi ERROR", "Restarting...");
        delay(3000);
        ESP.restart();
    }
    
    apiKey = custom_api_key.getValue();
    if (apiKey.length() > 0) {
        prefs.begin("config", false);
        prefs.putString("apiKey", apiKey);
        prefs.end();
    }
    if (WiFi.isConnected()) {
        startMDNS();
    }
    wifiConnected = true;

    // Check TigerTag cloud health (lightweight)
    cloudOK = checkServerHealth();

    displayMessage(
        "WiFi Connected!",
        WiFi.SSID(),
        WiFi.localIP().toString(),
        cloudOK ? "Cloud: OK" : "Cloud: FAIL"
    );
    delay(2000);
}

// ============================================================================
// LITTLEFS : Initialisation et debug
// ============================================================================

void setupFileSystem() {
    Serial.println("\n[LITTLEFS] Initialisation...");
    
    if (!LittleFS.begin(true)) {  // true = format si échec
        Serial.println("❌ [LITTLEFS] Échec montage!");
        displayMessage("ERROR", "Filesystem FAIL", "Check data/");
        delay(3000);
        return;
    }
    
    Serial.println("✅ [LITTLEFS] Monté avec succès");
    
    // Debug : lister les fichiers disponibles
    File root = LittleFS.open("/www");
    if (!root) {
        Serial.println("⚠️  [LITTLEFS] Dossier /www introuvable!");
        Serial.println("    → Uploadez le filesystem: pio run --target uploadfs");
        return;
    }
    
    Serial.println("\n📂 [LITTLEFS] Fichiers dans /www/:");
    File file = root.openNextFile();
    while (file) {
        Serial.printf("   📄 %s (%d bytes)\n", file.name(), file.size());
        file = root.openNextFile();
    }
    Serial.println();
}

// Validate API key against TigerTag CDN (firmware-side)
bool validateApiKeyFirmware(const String& key, String& displayNameOut) {
    displayNameOut = "";
    if (key.length() == 0) return false;
    HTTPClient http;
    http.setTimeout(3000);
    String url = String("https://cdn.tigertag.io/pingbyapikey?key=") + key;
    if (!http.begin(url)) {
        Serial.println("[APIKEY] http.begin failed");
        return false;
    }
    int code = http.GET();
    bool ok = false;
    if (code == 200) {
        String body = http.getString();
        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, body);
        if (!err) {
            ok = doc["success"] | false;
            if (ok && doc["displayName"].is<const char*>()) {
                displayNameOut = String(doc["displayName"].as<const char*>());
            }
        } else {
            Serial.printf("[APIKEY] JSON parse error: %s\n", err.c_str());
        }
    } else {
        Serial.printf("[APIKEY] HTTP %d\n", code);
    }
    http.end();
    return ok;
}

// ============================================================================
// SERVEUR WEB & API
// ============================================================================

// ⚠️ SUPPRIMÉ : const char index_html[] PROGMEM = R"rawliteral(...
// Les fichiers HTML sont maintenant servis depuis LittleFS

AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket client #%u connected\n", client->id());
        // Send an immediate snapshot so the UI updates right away on connect
        int wIntSnap = (int)(currentWeight + (currentWeight >= 0 ? 0.5f : -0.5f));
        char snap[96];
        snprintf(snap, sizeof(snap), "{\"weight\":%d,\"uid\":\"%s\"}", wIntSnap, lastUID.c_str());
        client->text(snap);
        // Also push current API status so the UI reflects it immediately on fresh load
        {
            StaticJsonDocument<192> out;
            out["type"] = "apiStatus";
            out["valid"] = apiValid;
            if (apiValid && apiDisplayName.length()) out["displayName"] = apiDisplayName;
            String outStr; serializeJson(out, outStr);
            client->text(outStr);
        }
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (!info->final || info->opcode != WS_TEXT) return; // handle simple single-frame text only
        String msg = String((const char*)data).substring(0, len);

        StaticJsonDocument<256> doc;
        DeserializationError err = deserializeJson(doc, msg);
        if (err) {
            Serial.printf("[WS] bad JSON: %s\n", err.c_str());
            return;
        }
        const char* mtype = doc["type"] | "";
        if (strcmp(mtype, "updateApiKey") == 0) {
            String newKey = String(doc["value"] | "");
            newKey.trim();
            if (newKey.length() == 0) {
                displayMessage("API key FAIL", "Check key");
                delay(600);
                displayWeight(currentWeight, lastUID);
                client->text("{\"type\":\"apiStatus\",\"valid\":false}");
                return;
            }
            String displayName;
            bool ok = validateApiKeyFirmware(newKey, displayName);
            if (ok) {
                // Persist only if valid
                apiKey = newKey;
                apiValid = true;
                apiDisplayName = displayName;
                prefs.begin("config", false);
                prefs.putString("apiKey", apiKey);
                prefs.putString("apiName", apiDisplayName);
                prefs.end();
                // Notify UI
                displayMessage("API key OK", apiDisplayName);
                delay(600);
                displayWeight(currentWeight, lastUID);
                StaticJsonDocument<192> out;
                out["type"] = "apiStatus";
                out["valid"] = true;
                out["displayName"] = apiDisplayName;
                String outStr; serializeJson(out, outStr);
                client->text(outStr);
                // Optional: also echo the stored key (if UI needs to sync)
                // client->text(String("{\"type\":\"apiKey\",\"value\":\"") + apiKey + "\"}");
            } else {
                displayMessage("API key FAIL", "Check key");
                delay(600);
                displayWeight(currentWeight, lastUID);
                client->text("{\"type\":\"apiStatus\",\"valid\":false}");
            }
        }
    }
}

void setupWebServer() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    
    // ============================================
    // SERVIR FICHIERS STATIQUES DEPUIS LITTLEFS
    // ============================================
    
    // Page principale (index.html, fallback to .gz)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists("/www/index.html")) {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/www/index.html", "text/html");
            response->addHeader("Cache-Control", "no-store");
            request->send(response);
            return;
        }
        if (LittleFS.exists("/www/index.html.gz")) {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/www/index.html.gz", "text/html");
            response->addHeader("Content-Encoding", "gzip");
            response->addHeader("Cache-Control", "no-store");
            request->send(response);
            return;
        }
        request->send(404, "text/plain", "index.html(.gz) not found - uploadfs required");
    });
    
    // CSS (style.css, fallback to .gz), cache 24h
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists("/www/style.css")) {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/www/style.css", "text/css");
            response->addHeader("Cache-Control", "max-age=86400");
            request->send(response);
            return;
        }
        if (LittleFS.exists("/www/style.css.gz")) {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/www/style.css.gz", "text/css");
            response->addHeader("Content-Encoding", "gzip");
            response->addHeader("Cache-Control", "max-age=86400");
            request->send(response);
            return;
        }
        request->send(404, "text/plain", "style.css(.gz) not found");
    });
    
    // JavaScript (app.js, fallback to .gz), no-store
    server.on("/app.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (LittleFS.exists("/www/app.js")) {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/www/app.js", "application/javascript");
            response->addHeader("Cache-Control", "no-store");
            request->send(response);
            return;
        }
        if (LittleFS.exists("/www/app.js.gz")) {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/www/app.js.gz", "application/javascript");
            response->addHeader("Content-Encoding", "gzip");
            response->addHeader("Cache-Control", "no-store");
            request->send(response);
            return;
        }
        request->send(404, "text/plain", "app.js(.gz) not found");
    });
    
    // ============================================
    // API ENDPOINTS (inchangés)
    // ============================================
    
    server.on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String body = String((char*)data).substring(0, len);
            int keyStart = body.indexOf("\"apiKey\":\"") + 10;
            int keyEnd = body.indexOf("\"", keyStart);
            apiKey = body.substring(keyStart, keyEnd);
            
            prefs.begin("config", false);
            prefs.putString("apiKey", apiKey);
            prefs.end();
            
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );
    
    server.on("/api/reset-wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"resetting\"}");
        delay(1000);
        wm.resetSettings();
        ESP.restart();
    });
    
    server.on("/api/factory-reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"factory reset\"}");
        delay(1000);
        prefs.begin("config", false);
        prefs.clear();
        prefs.end();
        wm.resetSettings();
        ESP.restart();
    });
    
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{";
        {
            int wInt = (int)(currentWeight + (currentWeight >= 0 ? 0.5f : -0.5f));
            json += "\"weight\":" + String(wInt) + ",";
        }
        json += "\"uid\":\"" + lastUID + "\",";
        json += "\"uid_hex\":\"" + lastUIDHex + "\",";
        json += "\"wifi\":\"" + WiFi.SSID() + "\",";
        json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
        json += "\"cloud\":\"" + String(cloudOK ? "ok" : "down") + "\",";
        json += "\"apiKey\":\"" + apiKey + "\",";
        json += "\"apiValid\":" + String(apiValid ? "true" : "false") + ",";
        json += "\"displayName\":\"" + apiDisplayName + "\",";
        json += "\"calibrationFactor\":" + String(calibrationFactor, 4);
        json += "}";
        request->send(200, "application/json", json);
    });
    
    server.on("/api/push-weight", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            String body = String((const char*)data).substring(0, len);
            int wp = body.indexOf("weight");
            if (wp < 0) { request->send(400, "application/json", "{\"error\":\"missing weight\"}"); return; }
            int colon = body.indexOf(':', wp);
            if (colon < 0) { request->send(400, "application/json", "{\"error\":\"bad json\"}"); return; }
            String num = body.substring(colon+1);
            num.trim();
            while (num.length() && (num[num.length()-1] < '0' || num[num.length()-1] > '9') && num[num.length()-1] != '.' ) num.remove(num.length()-1);
            while (num.length() && ( (num[0] < '0' || num[0] > '9') && num[0] != '-' && num[0] != '.' )) num.remove(0,1);
            float w = num.toFloat();
            int wi = (int)(w + (w >= 0 ? 0.5f : -0.5f));
            if (w <= 0 && num.indexOf('0') != 0 && num.indexOf('.') != 0) { request->send(400, "application/json", "{\"error\":\"invalid weight\"}"); return; }

            if (apiKey.length() == 0) { request->send(400, "application/json", "{\"error\":\"missing apiKey\"}"); return; }
            if (lastUID.length() == 0) { request->send(400, "application/json", "{\"error\":\"missing uid (present a tag)\"}"); return; }

            HTTPClient http;
            const char* url = "https://us-central1-tigertag-connect.cloudfunctions.net/setSpoolWeightByRfid";
            if (!http.begin(url)) { request->send(500, "application/json", "{\"error\":\"http begin failed\"}"); return; }
            http.addHeader("Content-Type", "application/json");
            http.addHeader("x-api-key", apiKey);
            String payload = String("{\"uid\":\"") + lastUID + "\",\"weight\":" + String(wi) + "}";
            int code = http.POST(payload);
            String resp = http.getString();
            http.end();

            if (code >= 200 && code < 300) {
                currentWeight = (float)wi;
                displayMessage("Synced \xE2\x9C\x93", String(wi) + " g", "to cloud");
                delay(700);
                lastUID = "";
                lastPushedWeight = NAN;
                stableSinceMs = 0;
                stableCandidate = NAN;
                char buf[64];
                snprintf(buf, sizeof(buf), "{\"weight\":%d,\"uid\":\"%s\"}", wi, lastUID.c_str());
                ws.textAll(buf);
                displayWeight(currentWeight, lastUID);
                request->send(200, "application/json", "{\"status\":\"ok\"}");
            } else {
                String err = String("{\"error\":\"upstream ") + code + "\",\"body\":" + '"' + resp + '"' + "}";
                request->send(502, "application/json", err);
            }
        }
    );

    server.on("/api/tare", HTTP_POST, [](AsyncWebServerRequest *request){
        scale.tare();
        currentWeight = 0.0f;
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"weight\":%.2f,\"uid\":\"%s\"}", currentWeight, lastUID.c_str());
        ws.textAll(buf);
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    });

    server.on("/api/calibration", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            String body = String((const char*)data).substring(0, len);
            int p = body.indexOf("factor");
            if (p < 0) { request->send(400, "application/json", "{\"error\":\"missing factor\"}"); return; }
            int colon = body.indexOf(':', p);
            if (colon < 0) { request->send(400, "application/json", "{\"error\":\"bad json\"}"); return; }
            String num = body.substring(colon+1); num.trim();
            while (num.length() && (num[num.length()-1] < '0' || num[num.length()-1] > '9') && num[num.length()-1] != '.' && num[num.length()-1] != '-') num.remove(num.length()-1);
            while (num.length() && ((num[0] < '0' || num[0] > '9') && num[0] != '-' && num[0] != '.')) num.remove(0,1);
            float f = num.toFloat();
            if (f == 0.0f) { request->send(400, "application/json", "{\"error\":\"invalid factor\"}"); return; }

            calibrationFactor = f;
            scale.set_scale(calibrationFactor);
            prefs.begin("config", false);
            prefs.putFloat("calFactor", calibrationFactor);
            prefs.end();
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );
    
    // Page 404
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "404 Not Found");
    });
    
    server.begin();
    Serial.println("✅ Serveur web démarré sur port 80");
}

// Helper: push weight to TigerTag Cloud Function
bool pushWeightToCloud(float w) {
    if (!wifiConnected || !WiFi.isConnected()) return false;
    if (apiKey.length() == 0 || lastUID.length() == 0) return false;

    HTTPClient http;
    const char* url = "https://us-central1-tigertag-connect.cloudfunctions.net/setSpoolWeightByRfid";
    if (!http.begin(url)) return false;
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", apiKey);
    int wInt = (int)(w + (w >= 0 ? 0.5f : -0.5f));
    String payload = String("{\"uid\":\"") + lastUID + "\",\"weight\":" + String(wInt) + "}";
    int code = http.POST(payload);
    String resp = http.getString();
    http.end();
    if (code >= 200 && code < 300) {
        return true;
    }
    Serial.printf("[AutoPush] Upstream error %d: %s\n", code, resp.c_str());
    return false;
}

void handleAutoPush(float w) {
    const uint32_t now = millis();

    if (w < MIN_WEIGHT_TO_SEND_G) { stableSinceMs = 0; stableCandidate = NAN; return; }
    if (apiKey.length() == 0 || lastUID.length() == 0 || !WiFi.isConnected()) { return; }

    if (isnan(stableCandidate)) {
        stableCandidate = w;
        stableSinceMs = now;
    }

    if (fabs(w - stableCandidate) > STABLE_EPSILON_G) {
        stableCandidate = w;
        stableSinceMs = now;
        return;
    }

    if (now - stableSinceMs < STABLE_WINDOW_MS) return;

    if (!isnan(lastPushedWeight)) {
        if (fabs(w - lastPushedWeight) < RESEND_DELTA_G) return;
        if (now - lastPushMs < RESEND_COOLDOWN_MS) return;
    }

    displayMessage("Sending...", String("UID ") + lastUID, String(w, 1) + " g");

    bool ok = pushWeightToCloud(w);
    if (ok) {
        int wInt = (int)(w + (w >= 0 ? 0.5f : -0.5f));
        lastPushedWeight = w;
        lastPushMs = now;
        displayMessage("Synced \xE2\x9C\x93", String(wInt) + " g", "to cloud");
        delay(700);
        lastUID = "";
        lastPushedWeight = NAN;
        stableSinceMs = 0;
        stableCandidate = NAN;
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"weight\":%d,\"uid\":\"%s\"}", wInt, lastUID.c_str());
        ws.textAll(buf);
        displayWeight((float)wInt, lastUID);
    } else {
        displayMessage("Sync failed", "Check Wi‑Fi/API", String(w, 1) + " g");
        delay(700);
        displayWeight(w, lastUID);
    }
}

// ============================================================================
// mDNS LIFECYCLE HELPERS
// ============================================================================

void startMDNS() {
    MDNS.end();
    delay(50);
    if (WiFi.isConnected()) {
        if (MDNS.begin(MDNS_NAME)) {
            MDNS.addService("http", "tcp", 80);
            Serial.println("[mDNS] started: http://" + String(MDNS_NAME) + ".local");
        } else {
            Serial.println("[mDNS] start failed");
        }
    }
}

void onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
#ifdef SYSTEM_EVENT_STA_GOT_IP
        case SYSTEM_EVENT_STA_GOT_IP:
#endif
            wifiConnected = true;
            Serial.println("[WiFi] GOT_IP: " + WiFi.localIP().toString());
            startMDNS();
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
#ifdef SYSTEM_EVENT_STA_DISCONNECTED
        case SYSTEM_EVENT_STA_DISCONNECTED:
#endif
            wifiConnected = false;
            Serial.println("[WiFi] DISCONNECTED");
            MDNS.end();
            break;
        default:
            break;
    }
}

// ============================================================================
// GESTION BALANCE
// ============================================================================

void setupScale() {
    scale.begin(HX711_DOUT, HX711_SCK);
    scale.set_scale(calibrationFactor);
    scale.tare();
    
    displayMessage("Scale OK", "Tare done");
    delay(1000);
}

float readWeight() {
    if (scale.is_ready()) {
        currentWeight = scale.get_units(5);
        return currentWeight;
    }
    return 0.0;
}

// ============================================================================
// GESTION RFID
// ============================================================================

static String u64ToDec(uint64_t v) {
    if (v == 0) return String("0");
    char buf[21];
    buf[20] = '\0';
    int i = 20;
    while (v > 0 && i > 0) {
        uint64_t q = v / 10ULL;
        uint8_t r = (uint8_t)(v - q * 10ULL);
        buf[--i] = '0' + r;
        v = q;
    }
    return String(&buf[i]);
}

void setupRFID() {
    SPI.begin();
    rfid.PCD_Init();
    displayMessage("RFID OK", "RC522 ready");
    delay(1000);
}

String readRFID() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
        return "";
    }

    String hexStr; hexStr.reserve(rfid.uid.size * 2);
    uint64_t decVal = 0ULL;
    for (byte i = 0; i < rfid.uid.size; i++) {
        byte b = rfid.uid.uidByte[i];
        if (b < 0x10) hexStr += '0';
        hexStr += String(b, HEX);
        decVal = (decVal << 8) | b;
    }
    hexStr.toUpperCase();

    lastUIDHex = hexStr;
    String decStr = u64ToDec(decVal);

    rfid.PICC_HaltA();
    return decStr;
}

// ============================================================================
// CLOUD HEALTH CHECK
// ============================================================================

bool checkServerHealth() {
    HTTPClient http;
    http.setTimeout(1500);
    const char* url = "https://healthz-s3bqq5xmtq-uc.a.run.app/";
    if (!http.begin(url)) {
        Serial.println("[HEALTHZ] begin() failed");
        return false;
    }
    int code = http.GET();
    bool ok = false;
    if (code == 200) {
        String body = http.getString();
        ok = (body.indexOf("\"ok\":true") >= 0);
        Serial.printf("[HEALTHZ] 200 body=%s\n", body.c_str());
    } else {
        Serial.printf("[HEALTHZ] HTTP %d\n", code);
    }
    http.end();
    Serial.println(ok ? "✅ Server health OK" : "❌ Server health FAIL");
    return ok;
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    Wire.begin(21, 22);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("Erreur OLED"));
        while (1);
    }
    
    displayMessage("TigerTagScale", "Starting...", "v1.1.0");
    delay(2000);
    
    prefs.begin("config", true);
    apiKey = prefs.getString("apiKey", "");
    calibrationFactor = prefs.getFloat("calFactor", calibrationFactor);
    apiDisplayName = prefs.getString("apiName", "");
    prefs.end();
    
    WiFi.onEvent(onWiFiEvent);
    setupWiFi();
    if (WiFi.isConnected()) {
        startMDNS();
    }

    // On boot: validate existing API key once
    if (apiKey.length() > 0 && WiFi.isConnected()) {
        String dn;
        apiValid = validateApiKeyFirmware(apiKey, dn);
        if (apiValid) {
            if (dn.length()) apiDisplayName = dn;
            prefs.begin("config", false);
            prefs.putString("apiName", apiDisplayName);
            prefs.end();
        }
    }
    
    setupFileSystem();  // ← AJOUTÉ : Monte LittleFS
    setupWebServer();
    setupScale();
    setupRFID();
    
    displayMessage(
        "READY!",
        "IP: " + WiFi.localIP().toString(),
        "tigerscale.local",
        "Place an Spool.."
    );
}

void loop() {
    static unsigned long lastUpdate = 0;
    static unsigned long lastBlink = 0;
    
    if (millis() - lastBlink > 1000) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastBlink = millis();
    }
    
    String uid = readRFID();
    if (uid.length() > 0 && uid != lastUID) {
        lastUID = uid;
        Serial.println("UID detected (DEC): " + lastUID + "  (HEX): " + lastUIDHex);
    }
    
    float weight = readWeight();
    if (millis() - lastUpdate > WS_UPDATE_INTERVAL_MS) {
        displayWeight(weight, lastUID);
        
        int wInt = (int)(weight + (weight >= 0 ? 0.5f : -0.5f));
        String json = "{\"weight\":" + String(wInt) + 
                      ",\"uid\":\"" + lastUID + "\"}";
        ws.textAll(json);
        ws.cleanupClients();
        
        lastUpdate = millis();
    }

    // Periodically rebroadcast API status so late joiners / stale UIs sync automatically
    if (millis() - lastApiBroadcastMs > 5000) { // every 5s
        if (ws.count() > 0) {
            StaticJsonDocument<192> out;
            out["type"] = "apiStatus";
            out["valid"] = apiValid;
            if (apiValid && apiDisplayName.length()) out["displayName"] = apiDisplayName;
            String outStr; serializeJson(out, outStr);
            ws.textAll(outStr);
        }
        lastApiBroadcastMs = millis();
    }

    handleAutoPush(weight);
    
    delay(10);
}