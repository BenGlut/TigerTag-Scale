# 🐯 TigerScale — Smart RFID Filament Scale

> [!IMPORTANT]
> **This is the end of the V2 line. It is no longer being developed.**
>
> Work has moved to **[TigerScale V3](https://github.com/TigerTag-Project/Tiger-Scale-V3)**,
> which is different hardware — an ESP32-S3 with a 3.5" colour touchscreen and two
> PN532 readers. **The two firmwares are not interchangeable**: this one will not
> run on a V3 board, and the V3 firmware will not run on yours.
>
> Nothing here stops working. The web installer, the documentation and the
> releases stay online, and a V2 you already built keeps talking to the TigerTag
> cloud exactly as before. What it will not get is new features or new releases.
>
> Building one today? Build a V3:
> **[repository](https://github.com/TigerTag-Project/Tiger-Scale-V3)** ·
> **[install it from your browser](https://tigertag-project.github.io/Tiger-Scale-V3/)**
>
> **This repository is archived and read-only, so its issue tracker is closed.**
> If you run a V2 and hit a problem, come to
> **[Discord](https://discord.gg/3Qv5TSqnJH)** — that is where V2 questions are
> still answered. Everything here stays readable, cloneable and forkable.

> **Open-source IoT weighing scale for 3D printer filament spools**, with RFID tag detection, OLED display, automatic spool tracking, and TigerTag cloud sync.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/Build-PlatformIO-orange.svg)](https://platformio.org/)
[![Discord](https://img.shields.io/badge/Discord-Join-5865F2.svg)](https://discord.gg/3Qv5TSqnJH)

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)"  srcset="web-installer/img/logo_tigertag_contouring-dark.svg">
    <source media="(prefers-color-scheme: light)" srcset="web-installer/img/logo_tigertag_contouring.svg">
    <img src="web-installer/img/logo_tigertag_contouring.svg" alt="TigerTag logo" width="160">
  </picture>
</p>

<p align="center">
  <a href="https://tigertag-project.github.io/Tiger-Scale/">
    <img src="assets/svg/flash_web_installer.svg" width="420" height="68" alt="Open Web Installer — Flash your ESP32 in Chrome or Edge">
  </a>
</p>

<p align="center"><sub>Plug in your ESP32 → click Install → done. No toolchain, no command line.</sub></p>

---

## ✨ What is it?

TigerScale is a **DIY smart scale** that knows which spool sits on it. Drop a spool with a TigerTag (NFC sticker) on the platform — the scale reads the tag, weighs the spool, computes the **net filament weight** (subtracting the empty cardboard/plastic spool), and syncs the result to your TigerTag account in real time.

It is designed to be **3D-printable, hackable, and cheap** (~30 € BoM). The cloud service that synchronises your inventory across devices is provided by **TigerTag** — an account is free.

### 🎯 Key features

- 🏷️ **Dual RFID readers** (RC522 13.56 MHz) for twin-tag spools
- ⚖️ **Precision weighing** via HX711 + 5 kg load cell
- 📺 **Live OLED display** with weight, rack position, and spool metadata
- ⚙️ **Continuous servo** that gently rotates the spool to find the second tag
- ☁️ **Real-time cloud sync** (Firestore) — see your inventory from any device
- 🔐 **Google Sign-In** via OAuth bridge (no passwords stored on device)
- 📱 **Modern web UI** served by the ESP32 itself, mobile-friendly — with live **workflow badge** (Scanning / Stabilizing / Sending / Ready)
- 🌍 **9-language UI** (EN / FR / DE / ES / IT / PL / PT-BR / PT / ZH) with flag-based switcher in header and login modal, locale files in `data/www/locales/`
- 🔐 **Full in-modal auth** — Google Sign-In, email/password sign-in, **account creation** and **password reset** directly in the UI (no redirect)
- 📟 **Firmware version + uptime** always visible under the TigerScale title, uptime advances client-side between WebSocket frames
- 👤 **Account avatar** with user initials + display name, collapsible account card
- 🧰 **Toolbox toggle** — collapse all settings below TARE for a cleaner read-only interface
- 🗄️ **Local brand & material DB** (LittleFS) — RFID lookups are instant, no internet required per scan; auto-updated from GitHub every 24 h
- 🔄 **Auto-tare**, **EMA + median filtering**, **twin-tag pairing** (§6.1 spec)
- ⚡ **WebSocket delta compression** — only changed fields broadcast at 10 Hz; full snapshot on connect
- 🛠️ **No closed binary blobs** — everything compiles from source

---

## 🚀 Quick start

### Option A — Web Installer (easiest, no toolchain)

<p align="center">
  <a href="https://tigertag-project.github.io/Tiger-Scale/">
    <img src="assets/svg/flash_web_installer.svg" width="420" height="68" alt="Open Web Installer — Flash your ESP32 in Chrome or Edge">
  </a>
</p>

Plug your ESP32 in via USB and click **Install**. The browser flashes the firmware and the filesystem in 60 seconds. No Arduino IDE, no PlatformIO, no command line.

### Option B — PlatformIO (recommended for developers)

```bash
git clone https://github.com/TigerTag-Project/Tiger-Scale.git
cd Tiger-Scale
pio run -t upload     # firmware
pio run -t uploadfs   # web UI (LittleFS)
pio device monitor    # serial console
```

### Option C — Arduino IDE (for tinkerers)

1. Open `src/main.cpp` in Arduino IDE 2.x (rename it to `TigerScale.ino` first)
2. Install the libraries listed in [docs/INSTALLATION.md](docs/INSTALLATION.md#libraries)
3. Board: `ESP32 Dev Module` · Partition: `Default 4MB with spiffs`
4. Compile + upload + use the LittleFS uploader for `data/`

Detailed instructions: **[docs/INSTALLATION.md](docs/INSTALLATION.md)**

---

## 🧰 Hardware

| Component | Where to buy | ~Price |
|---|---|---|
| ESP32 DevKit (KEYESTUDIO ESP32-WROOM-32) | [amzn.to/4dhJV3u](https://amzn.to/4dhJV3u) | 8 € |
| 2× RC522 RFID readers (×5 set) | [amzn.to/47mqpQt](https://amzn.to/47mqpQt) | 6 € |
| HX711 + 5 kg load cell (Wishiot kit) | [amzn.to/3KZIOLl](https://amzn.to/3KZIOLl) | 7 € |
| SSD1306 128×64 OLED I²C (ELEGOO 0.96") | [amzn.to/3Rul05f](https://amzn.to/3Rul05f) | 4 € |
| FS90R continuous servo (Wishiot 360°) | [amzn.to/4tRzpGZ](https://amzn.to/4tRzpGZ) | 5 € |
| 3D-printed enclosure | [hardware/3d-models/](hardware/3d-models/) | filament cost |
| **Total** | | **~30 €** |

Full Bill of Materials: **[hardware/BOM.md](hardware/BOM.md)** (with affiliate links)
Wiring diagram: **[docs/HARDWARE.md](docs/HARDWARE.md)**
Print settings: **[hardware/3d-models/README.md](hardware/3d-models/README.md)**

### 🖨️ Ready-to-slice project file

Download **[Tiger_Scale_V2.3mf](hardware/3d-models/Tiger_Scale_V2.3mf)** — a single `.3mf` file pre-configured for your slicer (Bambu Studio / PrusaSlicer / Orca). It contains four build variants, pick the one that matches your hardware:

| Variant | Motor | RFID readers | |
|---|---|---|---|
| **TigerScale Light** | ✗ No motor | 1 reader | |
| **TigerScale Light** | ✗ No motor | 2 readers | ⭐ **Recommended** — best value for money |
| **TigerScale Plus** | ✓ Motor | 1 reader | |
| **TigerScale Plus** | ✓ Motor | 2 readers | |

Open the file, select your variant plate, and hit **Slice**. No manual orientation or support setup needed.

> 💡 **Buy a kit** — the [Web Installer page](https://tigertag-project.github.io/Tiger-Scale/) includes a one-click shopping list of every component with Amazon affiliate links. Buying through them at no extra cost supports the project and keeps the TigerTag cloud free.

---

## 🌐 First-time setup

When you boot a fresh TigerScale, it creates a Wi-Fi access point named `Setup-TigerScale-XXXX`.

1. **Connect** your phone to that network (no password)
2. **Captive portal** opens automatically → enter your home Wi-Fi credentials
3. Scale reboots, joins your network, displays its **local IP** + **mDNS name** (`tigerscale-XXXX.local`) on the OLED
4. Open `http://tigerscale-XXXX.local` (or the IP) in your browser
5. **Login modal** opens — sign in with **Google** (one click), **email/password**, or **create a new account** directly in the UI
6. ✅ You're done — start placing TigerTagged spools on the scale

Detailed walkthrough: **[docs/INSTALLATION.md](docs/INSTALLATION.md#first-boot)**

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      TigerScale Hardware                    │
│   ESP32 ←→ 2× RC522 (SPI shared) ←→ HX711 ←→ Load Cell      │
│       ↘ OLED (I²C)        ↘ Servo (PWM)                     │
└─────────────────────────────────────────────────────────────┘
                            │
              Wi-Fi + HTTPS │ Firebase REST API
                            ▼
┌─────────────────────────────────────────────────────────────┐
│  TigerTag Cloud (Firebase) — central service                │
│   • Firebase Auth (Google, Apple, Email)                    │
│   • Firestore (per-user inventory + scale state)            │
│   • OAuth bridge at tigertag-cdn.web.app/scale-auth.html    │
└─────────────────────────────────────────────────────────────┘
                            │
                   any phone or browser
                            ▼
            View your filament inventory anywhere
```

Architectural details: **[docs/FIRMWARE.md](docs/FIRMWARE.md)**

---

## 📚 Documentation

| Doc | What's inside |
|---|---|
| [docs/INSTALLATION.md](docs/INSTALLATION.md) | Toolchain setup (PlatformIO, Arduino IDE) + flashing |
| [docs/HARDWARE.md](docs/HARDWARE.md) | Pinout, wiring diagram, photos |
| [docs/FIRMWARE.md](docs/FIRMWARE.md) | Code architecture, state machines, REST API |
| [docs/FIREBASE_SETUP.md](docs/FIREBASE_SETUP.md) | Use TigerTag cloud (default) — how it works |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | Common issues and fixes |
| [hardware/BOM.md](hardware/BOM.md) | Parts list with vendor links |
| [CHANGELOG.md](CHANGELOG.md) | Version history |
| [CONTRIBUTING.md](CONTRIBUTING.md) | How to contribute |
| [CLAUDE.md](CLAUDE.md) | Full API reference (HTTP, WebSocket, Firestore) — used by AI agents |

---

## 🤖 AI Integration

TigerScale ships with a **Claude Code skill** that makes any AI agent a TigerScale integration expert — generating connection code, sending remote commands, and keeping documentation in sync with the firmware.

Install it in [Claude Code](https://claude.ai/claude-code):

> Drag [`agents/tigerscale-integration.skill`](agents/tigerscale-integration.skill) into Claude Code, or go to **Settings → Skills → Install from file**.

Once installed, just ask Claude to integrate your app with the scale, debug a connection, or add a new command — it reads [`CLAUDE.md`](CLAUDE.md) automatically and knows the full API.

→ Details in [`agents/README.md`](agents/README.md)

---

## 🤝 Contributing

Contributions are welcome! Whether you want to:

- Report a bug or suggest an improvement → [open an issue](../../issues)
- Submit a code/documentation PR → see [CONTRIBUTING.md](CONTRIBUTING.md)
- Translate the UI to another language → edit the JSON files in `data/www/locales/` (one file per language)
- Print and improve the case → fork [hardware/3d-models/](hardware/3d-models/)

Join us on **[Discord](https://discord.gg/3Qv5TSqnJH)** for help, ideas, and community builds.

---

## 🏭 Make & Sell

**Anyone can manufacture and sell TigerScale hardware — no license fee, no registration.**

1. Follow the BOM and wiring in [`hardware/BOM.md`](hardware/BOM.md)
2. Flash the latest official firmware via the [Web Installer](https://tigertag-project.github.io/Tiger-Scale/) (always up to date)
3. Ship it — you're done

The only condition to call your product **"TigerScale"** is running the official firmware
unmodified, so every unit works consistently with the TigerTag ecosystem.
Forks are welcome — just use a different name. Full details: **[TRADEMARK.md](TRADEMARK.md)**

---

## 📜 License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) for details.

The **"TigerTag"** name (RFID protocol + cloud service) and **"TigerScale"** product name
are trademarks of the TigerTag Project. The code is free; the names have conditions.
See **[TRADEMARK.md](TRADEMARK.md)** for the full policy (short version: build and sell freely,
use official firmware, keep the name).

Third-party firmware dependencies retain their own licenses — three are LGPL
(ESPAsyncWebServer, AsyncTCP, ESP32Servo), satisfied by the fully open build system.
Full details: **[THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md)**

---

## 🙏 Acknowledgements

- **[Yoto](https://yotoplay.com)** for the user-experience inspiration
- **[ESP32 community](https://github.com/espressif/arduino-esp32)** for the framework
- **[ESP Web Tools](https://esphome.github.io/esp-web-tools/)** for the in-browser flasher
- All the contributors who build, test, and improve this project

---

<p align="center">
  Built with ❤️ by the <a href="https://tigertag.io">TigerTag</a> community.
</p>
