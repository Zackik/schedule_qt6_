# SmartSchedule: Native Qt6 Desktop AI Companion

SmartSchedule has evolved from a simple calendar manager into a full-fledged **Native Desktop AI Companion**. It completely abandons Web, Electron, Node.js, and browser technologies in favor of high-performance, native OS integration via **C++17 and Qt6**.

## 🌟 Core Features

- **Floating AI Chibi Mascot:** A cute, frameless, transparent-background anime character that floats on your desktop, completely independent of the main application window.
- **Proactive AI Notifications:** The assistant will natively interrupt (politely) with a dynamic Chat Bubble above its head to remind you of upcoming tasks or Deep Work sessions.
- **Dynamic Animation System:** The character breathes, thinks, gets excited, and talks. Uses `QTimer` and `QPainter` for smooth, low-CPU rendering without heavy browser engines.
- **Global Chat & Hotkey:** Press `Ctrl + Space` anywhere in your OS (while coding in VS Code, browsing Chrome, etc.) to instantly summon the AI companion.
- **Resource Monitoring:** Measures native RAM usage via OS-level APIs (e.g. `/proc/self/status` on Linux) and calculates SQLite/Cache byte storage explicitly.
- **Google Calendar Sync & Gemini AI:** Native REST integrations using `Qt6::Network`.

## 🏗️ Architecture

- `src/ui/companion/AIChibiWidget`: Top-level `QWidget` using `Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint`.
- `src/ui/companion/ChatBubbleWidget`: Floating bubble UI featuring rounded rectangles, drop shadows, and typing-effect animations.
- `src/services/companion/AnimationController`: A lightweight state machine (`Idle`, `Thinking`, `Talking`, `Reminder`) that drives the drawing frames.
- `src/services/companion/AssistantService`: The brain coordinating the mascot, Global Hotkey, and proactive schedules.

## 🚀 Build Requirements
- C++17 Compiler (GCC/Clang/MSVC)
- CMake 3.16+
- Qt 6.2+ (Core, Gui, Widgets, Sql, Network, NetworkAuth)

## 🛠️ Building & Running
```bash
cmake --preset release
cmake --build --preset release
./build/SmartSchedule
```
*Note: This is a 100% Native Desktop Application. When launched, the main window will open alongside a floating Chibi AI Mascot in the bottom-right corner of your desktop.*

## 🔒 CI/CD & Security Architecture
[![CI](https://github.com/Zackik/schedule_qt6_/actions/workflows/ci.yml/badge.svg)](https://github.com/Zackik/schedule_qt6_/actions/workflows/ci.yml)
[![Cross-Platform Build](https://github.com/Zackik/schedule_qt6_/actions/workflows/build.yml/badge.svg)](https://github.com/Zackik/schedule_qt6_/actions/workflows/build.yml)
[![Security Analysis](https://github.com/Zackik/schedule_qt6_/actions/workflows/security.yml/badge.svg)](https://github.com/Zackik/schedule_qt6_/actions/workflows/security.yml)
[![Release](https://github.com/Zackik/schedule_qt6_/actions/workflows/release.yml/badge.svg)](https://github.com/Zackik/schedule_qt6_/actions/workflows/release.yml)

The project leverages robust CI/CD pipelines via GitHub Actions:
- **Format & Linting:** `clang-format` and `clang-tidy` ensure code consistency and safety.
- **Cross-Platform Matrix:** Automatic builds for Windows (MSVC), Linux (GCC), and macOS (Clang).
- **Unit Testing:** Integrated `GoogleTest` and `CTest` run on every push and PR.
- **Security & DevSecOps:** GitHub CodeQL static analysis and strict dependency audits.
- **CD / Packaging:** Automatic CPack generation producing `.msi` (Windows), `.deb` (Linux), and `.dmg` (macOS) on release tags.

```text
Developer
   ↓
GitHub
   ↓
CI
   ├── Format (clang-format)
   ├── Build (CMake/Ninja)
   ├── Test (CTest + GTest)
   └── Security (CodeQL)
          ↓
       Release (Tag vX.Y.Z)
          ↓
 ┌────────┼────────┐
Windows  Linux    macOS
   ↓       ↓        ↓
 MSI      DEB      DMG
```

