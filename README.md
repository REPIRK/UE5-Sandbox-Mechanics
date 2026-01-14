# 🏗️ UE5 Physics Sandbox Mechanics

**Technical showcase from the game "Fractura" (Released on Steam).**
This repository contains the core C++ backend and system configurations for a physics-based destruction sandbox built on Unreal Engine 5.

🎥 **[Watch Mechanics Showreel](СЮДА_ВСТАВЬ_ССЫЛКУ_НА_ЮТУБ)** *(Link your video here later)*
🎮 **[View on Steam](СЮДА_ВСТАВЬ_ССЫЛКУ_НА_СТИМ)**

---

## 🛠️ Key Features

### 1. Hybrid Architecture (C++ & Blueprints)
Performance-critical logic is implemented in C++, exposed to Blueprints for gameplay flexibility.
*   **Math Library (`SandboxUtils`):** Custom raycasting logic with Grid Snapping and Surface Normal Alignment.
*   **Optimized Save System (`SandboxSaveGame`):** Handles serialization of thousands of dynamic objects using lightweight structs instead of heavy actor references.
*   **Data-Oriented Design (`SandboxItemData`):** Uses `TSoftClassPtr` and Async Loading to manage memory efficient item spawning.

### 2. Gameplay Systems
*   **Construction System:** Raycast-based placement with validation (Green/Red ghost visualization) and overlap checks.
*   **Chaos Physics Integration:** Custom handling of `Geometry Collections` and `Field Systems` for C4 explosives.
*   **Dynamic Inputs:** Runtime key rebinding using UE5 Enhanced Input subsystem.

### 3. Optimization Techniques
*   **Async Asset Loading:** Prevents frame drops when selecting new items.
*   **Time-Sliced Processing:** Heavy loops (like loading a massive save file) are distributed across multiple frames.
*   **PSO Caching:** Configured for Shipping builds to eliminate shader stutter.

---

## 📂 Code Structure

*   `Source/SANDBOX/Public/SandboxUtils.h` - Core static function library.
*   `Source/SANDBOX/Public/SandboxSaveGame.h` - Save data structures.
*   `Source/SANDBOX/Public/SandboxItemData.h` - Primary Data Asset definition.

---

## 💻 Tech Stack
*   **Engine:** Unreal Engine 5.7
*   **Languages:** C++, Blueprints
*   **IDE:** Visual Studio 2022
*   **Version Control:** Git

---

*Note: This repository contains source code samples only. Full game assets are proprietary.*
