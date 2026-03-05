# 🐍 Classic Snake Game (C / DOSBox)

<p align="left">
  <img src="https://img.shields.io/badge/C-A8B9CC?style=for-the-badge&logo=c&logoColor=white" alt="C Language" />
  <img src="https://img.shields.io/badge/DOSBox-000000?style=for-the-badge&logo=dosbox&logoColor=white" alt="DOSBox" />
  <img src="https://img.shields.io/badge/Low--Level-Programming-green?style=for-the-badge" alt="Low Level" />
</p>

## 📌 Project Overview
A low-level implementation of the classic arcade game **Snake**, developed in **C**. The project was built to run in a 16-bit environment (DOSBox), showcasing real-time computing constraints, direct memory manipulation, and efficient hardware resource management.

## 📢 Features

**✅ Real-Time Game Engine**
* Implemented core execution loops to handle continuous game state updates and frame rendering.
* Integrated hardware interrupts to capture real-time user keyboard input without pausing the game.

**✅ Advanced Memory & Data Management**
* Utilized complex **C pointers** for system-level memory allocation and direct manipulation.
* Designed efficient data structures to track the snake's dynamic length and coordinate history.

**✅ Classic Graphics & UI**
* Developed custom rendering logic for a 16-bit text-based interface.
* Implemented a score-tracking system and collision detection for walls and self-obstacles.

## 🛠 Tech Stack

**🖥 Language & Environment**
* **C:** The primary language for low-level logic and hardware interaction.
* **DOSBox:** Emulator used to provide a 16-bit DOS environment for execution.

**⚙️ Core Concepts**
* **Pointer Arithmetic:** Used for efficient coordinate management.
* **Hardware Interrupts:** Facilitating non-blocking keyboard input.

## 📐 Logic Flow



### 💻 How to Run

1. **Install DOSBox:** Ensure you have DOSBox installed on your system.
2. **Clone the repository:**
   ```bash
   git clone [https://github.com/rober-saliba/Snake-Game-C.git](https://github.com/rober-saliba/Snake-Game-C.git)
   ```
3. **Mount the Directory:** Open DOSBox and mount your project folder as a drive:
   ```dos
   MOUNT C C:\path\to\your\folder
   C:
   ```
4. **Run the Game:**
   ```dos
   SNAKE.EXE
   ```
