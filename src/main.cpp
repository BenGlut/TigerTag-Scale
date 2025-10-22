/*
 * @file main.cpp
 * @brief TigerTagScale - Balance connectée ESP32 avec portail captif
 * @version 1.0.0
 */

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HX711.h>
#include <MFRC522.h>
#include <SPI.h>

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

void displayWeight(float weight, const String& uid) {
    display.clearDisplay();
    
     // En-tête avec titre et statut WiFi
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("TigerTagScale");
    
    display.setTextSize(1);
    display.setCursor(100, 0);
    display.println(wifiConnected ? "WiFi" : "----");
    
    // Poids au centre (grande taille) — entier uniquement
    int wInt = (int)(weight + (weight >= 0 ? 0.5f : -0.5f));
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print(wInt);
    display.println(" g");
    
    // ⭐ UID
    if (uid.length() > 0) {
        display.setTextSize(1);
        display.setCursor(0, 45);
        display.print("UID:");
        display.println(uid);
    }
    
    // ⭐ IP en dessous de l'UID
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
        "MODE CONFIG",
        "WiFi: TigerScale",
        "IP: 192.168.4.1",
        "Configurez le WiFi"
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
    
    displayMessage("Connexion WiFi...", "Patientez...");
    
    if (!wm.autoConnect("TigerScale-Setup")) {
        displayMessage("ERREUR WiFi", "Redemarrage...");
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
        "WiFi Connecte!",
        WiFi.SSID(),
        WiFi.localIP().toString(),
        cloudOK ? "Cloud: OK" : "Cloud: FAIL"
    );
    delay(2000);
}

// ============================================================================
// SERVEUR WEB & API
// ============================================================================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TigerTagScale</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #fff;
            padding: 20px;
        }
        .container { 
            max-width: 600px; 
            margin: 0 auto; 
            background: rgba(255,255,255,0.1);
            border-radius: 15px;
            padding: 30px;
            backdrop-filter: blur(10px);
        }
        h1 { text-align: center; margin-bottom: 30px; font-size: 2em; }
        .card { 
            background: rgba(255,255,255,0.2);
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .weight { font-size: 3em; text-align: center; margin: 20px 0; }
        .uid { text-align: center; font-size: 1.2em; opacity: 0.9; }
        input, button { 
            width: 100%;
            padding: 12px;
            margin: 10px 0;
            border-radius: 8px;
            border: none;
            font-size: 1em;
        }
        button { 
            background: #fff;
            color: #667eea;
            cursor: pointer;
            font-weight: bold;
            transition: 0.3s;
        }
        button:hover { transform: scale(1.05); }
        button.danger { background: #e74c3c; color: #fff; }
        .info { text-align: center; opacity: 0.8; margin-top: 10px; }
        .cloud-row { text-align:center; margin:-10px 0 20px 0; opacity:0.95; }
        .cloud-dot { display:inline-block; width:10px; height:10px; border-radius:50%; margin-left:8px; vertical-align:middle; background:#bbb; }
        .cloud-ok { background:#2ecc71 !important; }
        .cloud-down { background:#e74c3c !important; }
    </style>
</head>
<body>
    <div class="container">
        <h1>TigerTagScale</h1>
        <div class="cloud-row">Cloud: <span id="cloudState">unknown</span><span id="cloudDot" class="cloud-dot"></span></div>
        <div class="card">
            <h2>Current weight</h2>
            <div class="weight" id="weight">-- g</div>
            <div class="uid" id="uid">No tag detected</div>
        </div>
        
        <div class="card">
            <h2>Change API Key</h2>
            <input type="text" id="newApiKey" placeholder="New API Key" value="">
            <button onclick="updateApiKey()">Update</button>
        </div>

        <div class="card">
            <h2>Send weight to TigerTag</h2>
            <input type="number" id="manualWeight" placeholder="Weight in grams (integer)" inputmode="numeric" step="1">
            <button onclick="sendWeight()">Send</button>
            <div class="info">Uses current UID read by the RFID scanner</div>
        </div>

        <div class="card">
          <h2>Calibrate scale</h2>
          <div class="info">Current factor: <span id="calFactor">—</span></div>
          <input type="number" id="newCalFactor" placeholder="Calibration factor" inputmode="decimal" step="0.1">
          <div style="display:flex; gap:10px">
            <button onclick="tareScale()">Tare</button>
            <button onclick="updateCalibration()">Update factor</button>
          </div>
        </div>
        <div class="card">
          <h2>Auto-calc factor</h2>
          <div class="info">Place a known weight, wait for the reading to stabilize, then compute.</div>
          <input type="number" id="knownWeight" placeholder="Known weight (g)" inputmode="decimal" step="0.1">
          <button onclick="computeFactor()">Compute from reading</button>
          <div class="info" id="calcHint">Formula: new = current × (displayed / known)</div>
        </div>
        
        <div class="card">
            <h2>Change Wi‑Fi network</h2>
            <button onclick="resetWiFi()">Reconfigure Wi‑Fi</button>
            <div class="info">Restarts captive portal</div>
        </div>
        
        <div class="card">
            <h2>Factory reset</h2>
            <button class="danger" onclick="factoryReset()">Factory Reset</button>
            <div class="info">Erases all data</div>
        </div>
    </div>
    
    <script>
        const ws = new WebSocket('ws://' + window.location.hostname + '/ws');
        let latestWeight = null;
        let latestCalFactor = null;
        function setCloud(state) {
            const txt = document.getElementById('cloudState');
            const dot = document.getElementById('cloudDot');
            if (!txt || !dot) return;
            if (state === 'ok') {
                txt.innerText = 'OK';
                dot.classList.add('cloud-ok');
                dot.classList.remove('cloud-down');
            } else if (state === 'down') {
                txt.innerText = 'DOWN';
                dot.classList.add('cloud-down');
                dot.classList.remove('cloud-ok');
            } else {
                txt.innerText = 'unknown';
                dot.classList.remove('cloud-down');
                dot.classList.remove('cloud-ok');
            }
        }
        // Populate initial status (API key, weight, uid) without using template processor
        fetch('/api/status').then(r => r.json()).then(s => {
            if (s.weight !== undefined) {
                document.getElementById('weight').innerText = s.weight + ' g';
            }
            if (s.uid !== undefined && s.uid !== '') {
                const el = document.getElementById('uid');
                el.innerText = s.uid; // decimal
                if (s.uid_hex) el.title = 'HEX: ' + s.uid_hex;
            }
            const apiKeyInput = document.getElementById('newApiKey');
            if (apiKeyInput && typeof s.apiKey === 'string') {
                apiKeyInput.value = s.apiKey;
            }
            if (typeof s.calibrationFactor !== 'undefined') {
                document.getElementById('calFactor').innerText = Number(s.calibrationFactor).toFixed(4);
                const cfInput = document.getElementById('newCalFactor');
                if (cfInput && !cfInput.value) cfInput.value = s.calibrationFactor;
            }
            if (typeof s.weight !== 'undefined') {
                latestWeight = Number(s.weight);
            }
            if (typeof s.calibrationFactor !== 'undefined') {
                latestCalFactor = Number(s.calibrationFactor);
            }
            if (typeof s.cloud === 'string') {
                setCloud(s.cloud);
            }
        }).catch(() => {});
        
        ws.onmessage = function(event) {
            const data = JSON.parse(event.data);
            document.getElementById('weight').innerText = data.weight + ' g';
            document.getElementById('uid').innerText = data.uid || 'No tag detected';
            latestWeight = Number(data.weight);
        };

        // Refresh cloud status every 30s with a cheap /api/status call
        setInterval(() => {
            fetch('/api/status').then(r => r.json()).then(s => {
                if (typeof s.cloud === 'string') setCloud(s.cloud);
            }).catch(() => {});
        }, 30000);
        
        function updateApiKey() {
            const key = document.getElementById('newApiKey').value;
            fetch('/api/config', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({apiKey: key})
            }).then(() => alert('API key updated!'));
        }
        
        function sendWeight() {
            const w = parseInt(document.getElementById('manualWeight').value, 10);
            if (isNaN(w)) { alert('Please enter a valid integer weight.'); return; }
            fetch('/api/push-weight', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ weight: w })
            })
            .then(r => r.json())
            .then(j => {
                if (j && j.status === 'ok') {
                    alert('Weight sent to TigerTag.');
                } else {
                    alert('Failed to send: ' + (j && j.error ? j.error : 'unknown error'));
                }
            })
            .catch(e => alert('Network error'));
        }

        function tareScale() {
          fetch('/api/tare', {method: 'POST'})
            .then(r => r.json())
            .then(_ => { document.getElementById('weight').innerText = '0 g'; })
            .catch(_ => alert('Tare failed'));
        }

        function updateCalibration() {
          const f = parseFloat(document.getElementById('newCalFactor').value);
          if (isNaN(f) || f === 0) { alert('Enter a valid calibration factor'); return; }
          fetch('/api/calibration', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ factor: f })
          })
          .then(r => r.json())
          .then(j => {
            if (j && j.status === 'ok') {
              document.getElementById('calFactor').innerText = f.toFixed(4);
              alert('Calibration factor updated');
            } else {
              alert('Failed to update calibration factor');
            }
          })
          .catch(_ => alert('Network error'));
        }

        function computeFactor() {
          const known = parseFloat(document.getElementById('knownWeight').value);
          if (isNaN(known) || known <= 0) { alert('Enter a valid known weight (g)'); return; }
          if (latestWeight === null || isNaN(latestWeight) || latestWeight <= 0) { alert('Waiting for a valid reading...'); return; }
          const shownCal = parseFloat(document.getElementById('calFactor').innerText);
          const current = (!isNaN(shownCal) && shownCal > 0) ? shownCal : (latestCalFactor || 1);
          const newF = current * (latestWeight / known);
          const out = document.getElementById('newCalFactor');
          if (out) out.value = newF.toFixed(4);
          const hint = document.getElementById('calcHint');
          if (hint) hint.innerText = `Displayed=${latestWeight.toFixed(2)} g, Known=${known.toFixed(2)} g → New factor=${newF.toFixed(4)}`;
        }
                
        function resetWiFi() {
            if (confirm('Reconfigure Wi‑Fi network?')) {
                fetch('/api/reset-wifi', {method: 'POST'})
                .then(() => alert('Rebooting into config mode...'));
            }
        }
        
        function factoryReset() {
            if (confirm('WARNING: All data will be erased!')) {
                fetch('/api/factory-reset', {method: 'POST'})
                .then(() => alert('Reset done. Rebooting...'));
            }
        }
    </script>
</body>
</html>
)rawliteral";

String processor(const String& var) {
    if (var == "API_KEY") return apiKey;
    return String();
}

AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("WebSocket client #%u connected\n", client->id());
    }
}

void setupWebServer() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    
        request->send_P(200, "text/html", index_html);
    });
    
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
    json += "\"uid\":\"" + lastUID + "\",";           // ← ajoute le décimal
    json += "\"uid_hex\":\"" + lastUIDHex + "\",";     // ← l'hex reste
    json += "\"wifi\":\"" + WiFi.SSID() + "\",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"cloud\":\"" + String(cloudOK ? "ok" : "down") + "\",";
    json += "\"apiKey\":\"" + apiKey + "\",";
    json += "\"calibrationFactor\":" + String(calibrationFactor, 4);
    json += "}";
    request->send(200, "application/json", json);
});
    
    server.on("/api/push-weight", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            // Parse minimal JSON: {"weight":123.4}
            String body = String((const char*)data).substring(0, len);
            int wp = body.indexOf("weight");
            if (wp < 0) { request->send(400, "application/json", "{\"error\":\"missing weight\"}"); return; }
            // Extract number after ':'
            int colon = body.indexOf(':', wp);
            if (colon < 0) { request->send(400, "application/json", "{\"error\":\"bad json\"}"); return; }
            String num = body.substring(colon+1);
            // Trim trailing chars (comma, braces, spaces)
            num.trim();
            // Remove trailing non-number chars
            while (num.length() && (num[num.length()-1] < '0' || num[num.length()-1] > '9') && num[num.length()-1] != '.' ) num.remove(num.length()-1);
            // Remove leading non-number chars
            while (num.length() && ( (num[0] < '0' || num[0] > '9') && num[0] != '-' && num[0] != '.' )) num.remove(0,1);
            float w = num.toFloat();
            int wi = (int)(w + (w >= 0 ? 0.5f : -0.5f));
            if (w <= 0 && num.indexOf('0') != 0 && num.indexOf('.') != 0) { request->send(400, "application/json", "{\"error\":\"invalid weight\"}"); return; }

            if (apiKey.length() == 0) { request->send(400, "application/json", "{\"error\":\"missing apiKey\"}"); return; }
            if (lastUID.length() == 0) { request->send(400, "application/json", "{\"error\":\"missing uid (present a tag)\"}"); return; }

            // Forward to TigerTag Cloud Function
            HTTPClient http;
            const char* url = "https://us-central1-tigertag-connect.cloudfunctions.net/setSpoolWeightByRfid";
            if (!http.begin(url)) { request->send(500, "application/json", "{\"error\":\"http begin failed\"}"); return; }
            http.addHeader("Content-Type", "application/json");
            http.addHeader("x-api-key", apiKey); // send API key in header per Cloud Function
            String payload = String("{\"uid\":\"") + lastUID + "\",\"weight\":" + String(wi) + "}";
            int code = http.POST(payload);
            String resp = http.getString();
            http.end();

            if (code >= 200 && code < 300) {
                currentWeight = (float)wi;
                // Feedback on OLED
                displayMessage("Synced \xE2\x9C\x93", String(wi) + " g", "to cloud");
                delay(700);
                // Clear UID so a new tag is required for next send
                lastUID = "";
                lastPushedWeight = NAN;
                stableSinceMs = 0;
                stableCandidate = NAN;
                // Notify UI (empty uid means: no tag detected)
                char buf[64];
                snprintf(buf, sizeof(buf), "{\"weight\":%d,\"uid\":\"%s\"}", wi, lastUID.c_str());
                ws.textAll(buf);
                // Return to normal screen with no UID line
                displayWeight(currentWeight, lastUID);
                request->send(200, "application/json", "{\"status\":\"ok\"}");
            } else {
                String err = String("{\"error\":\"upstream ") + code + "\",\"body\":" + '"' + resp + '"' + "}";
                request->send(502, "application/json", err);
            }
        }
    );

    // Tare la balance
server.on("/api/tare", HTTP_POST, [](AsyncWebServerRequest *request){
    scale.tare();
    currentWeight = 0.0f;
    char buf[64];
    snprintf(buf, sizeof(buf), "{\"weight\":%.2f,\"uid\":\"%s\"}", currentWeight, lastUID.c_str());
    ws.textAll(buf);
    request->send(200, "application/json", "{\"status\":\"ok\"}");
});

// Met à jour le calibration factor
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
    server.begin();
    Serial.println("Serveur web demarre");
}

// Helper: push weight to TigerTag Cloud Function; returns true on success
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

    // Ignore tiny values and missing UID/API key
    if (w < MIN_WEIGHT_TO_SEND_G) { stableSinceMs = 0; stableCandidate = NAN; return; }
    if (apiKey.length() == 0 || lastUID.length() == 0 || !WiFi.isConnected()) { return; }

    // Initialize candidate
    if (isnan(stableCandidate)) {
        stableCandidate = w;
        stableSinceMs = now;
    }

    // If current reading deviates too much, reset window to new candidate
    if (fabs(w - stableCandidate) > STABLE_EPSILON_G) {
        stableCandidate = w;
        stableSinceMs = now;
        return;
    }

    // Check time window
    if (now - stableSinceMs < STABLE_WINDOW_MS) return;

    // Throttle: avoid resending same (or near-same) weight too often
    if (!isnan(lastPushedWeight)) {
        if (fabs(w - lastPushedWeight) < RESEND_DELTA_G) return;
        if (now - lastPushMs < RESEND_COOLDOWN_MS) return;
    }

    // Display sending status
    displayMessage("Sending...", String("UID ") + lastUID, String(w, 1) + " g");

    bool ok = pushWeightToCloud(w);
    if (ok) {
        int wInt = (int)(w + (w >= 0 ? 0.5f : -0.5f));
        lastPushedWeight = w;
        lastPushMs = now;
        displayMessage("Synced \xE2\x9C\x93", String(wInt) + " g", "to cloud");
        delay(700);
        // Clear UID so next send requires a new tag presentation
        lastUID = "";
        lastPushedWeight = NAN;
        stableSinceMs = 0;
        stableCandidate = NAN;
        // Notify UI (empty uid)
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"weight\":%d,\"uid\":\"%s\"}", wInt, lastUID.c_str());
        ws.textAll(buf);
        // Return to normal screen with no UID
        displayWeight((float)wInt, lastUID);
    } else {
        displayMessage("Sync failed", "Check Wi‑Fi/API", String(w, 1) + " g");
        delay(700);
        // Return to normal screen
        displayWeight(w, lastUID);
    }
}

// ============================================================================
// mDNS LIFECYCLE HELPERS
// ============================================================================

void startMDNS() {
    // Safely restart mDNS on current STA interface
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
    
    displayMessage("Balance OK", "Tare effectuee");
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
    displayMessage("RFID OK", "RC522 pret");
    delay(1000);
}

String readRFID() {
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
        return "";
    }

    // Build HEX (uppercase, no separators) and DEC (big-endian) representations
    String hexStr; hexStr.reserve(rfid.uid.size * 2);
    uint64_t decVal = 0ULL;
    for (byte i = 0; i < rfid.uid.size; i++) {
        byte b = rfid.uid.uidByte[i];
        if (b < 0x10) hexStr += '0';
        hexStr += String(b, HEX);
        decVal = (decVal << 8) | b; // big-endian accumulation
    }
    hexStr.toUpperCase();

    lastUIDHex = hexStr;
    String decStr = u64ToDec(decVal);

    rfid.PICC_HaltA();
    return decStr; // return DECIMAL string
}

// ============================================================================
// SETUP & LOOP
// ============================================================================

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    Wire.begin(21, 22); // SDA=21, SCL=22 (ESP32 defaults)
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("Erreur OLED"));
        while (1);
    }
    
    displayMessage("TigerTagScale", "Demarrage...", "v1.0.0");
    delay(2000);
    
    prefs.begin("config", true);
    apiKey = prefs.getString("apiKey", "");
    calibrationFactor = prefs.getFloat("calFactor", calibrationFactor);
    prefs.end();
    
    // Register Wi‑Fi events early so we see GOT_IP after WiFiManager connects
    WiFi.onEvent(onWiFiEvent);
    setupWiFi();
    if (WiFi.isConnected()) {
        startMDNS();
    }
    
    setupWebServer();
    setupScale();
    setupRFID();
    
    displayMessage(
        "PRET!",
        "IP: " + WiFi.localIP().toString(),
        "tigerscale.local",
        "Posez un objet..."
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
    lastUID = uid; // decimal
    Serial.println("UID detected (DEC): " + lastUID + "  (HEX): " + lastUIDHex);
}
    
    float weight = readWeight();
    if (millis() - lastUpdate > 500) {
        displayWeight(weight, lastUID);
        
        int wInt = (int)(weight + (weight >= 0 ? 0.5f : -0.5f));
        String json = "{\"weight\":" + String(wInt) + 
                      ",\"uid\":\"" + lastUID + "\"}";
        ws.textAll(json);
        
        lastUpdate = millis();
    }

    // Auto-detect stability and push to cloud if needed
    handleAutoPush(weight);
    
    delay(10);
}
// ============================================================================
// CLOUD HEALTH CHECK
// ============================================================================

bool checkServerHealth() {
    HTTPClient http;
    http.setTimeout(1500); // 1.5s
    // Use the lightweight public health endpoint
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
