# DarkZone: Abandoned Laboratory

## About the Project

DarkZone: Abandoned Laboratory is a first-person horror game developed as the final project for the Computer Graphics course at the National University of Engineering (UNI).

The player explores an abandoned scientific laboratory and must restore the power supply by activating two control boxes located throughout the map. While completing objectives, the player must avoid a hostile creature that patrols the facility. The game creates a tense atmosphere through dynamic lighting, a limited flashlight battery system, environmental audio, and a night skybox.

---

## Authors

* Aura Abihail Robleto Valdivia
* Idhe Isabel Cross Ramírez
* Harisema Milagros Ampie Amaya
* Edith María Rayo Rocha

---

## Technologies Used

* C++
* OpenGL
* GLFW
* GLAD
* GLM
* Assimp
* stb_image
* stb_easy_font
* miniaudio
* Blender

---

## Main Features

* First-person horror gameplay
* Dynamic lighting system
* Flashlight with battery management
* Interactive doors and key system
* Power box cable-connection minigame
* Enemy AI with multiple behavior states
* Animated 3D models with skeletal animation
* Collision system using AABB zones
* Night skybox environment
* Main menu, pause menu, victory and game over screens

---

## Project Structure

```text
Project/
│
├── Main.cpp
├── Model.h / Model.cpp
├── Mesh.h / Mesh.cpp
├── Enemy.h / Enemy.cpp
├── Menu.h / Menu.cpp
├── TextRenderer.h / TextRenderer.cpp
├── Interaction.h / Interaction.cpp
├── Shaders/
├── Models/
├── Textures/
├── Audio/
└── README.md
```

## How to Clone the Repository

```bash
git clone https://github.com/USERNAME/REPOSITORY.git
```

Replace `USERNAME` and `REPOSITORY` with the corresponding GitHub repository information.

---

## How to Run the Project

1. Clone the repository.
2. Open the project in Visual Studio.
3. Make sure all required libraries are properly configured:

   * GLFW
   * GLAD
   * GLM
   * Assimp
   * miniaudio
4. Build the solution.
5. Run the executable.

---

## Controls

| Key     | Action             |
| ------- | ------------------ |
| W A S D | Move               |
| Shift   | Run                |
| Ctrl    | Crouch             |
| Space   | Jump               |
| F       | Toggle Flashlight  |
| E       | Interact           |
| R G B   | Power Box Minigame |
| P       | Cancel Minigame    |
| ESC     | Pause Menu         |
| ALT     | Lock/Unlock Cursor |

---

## Screenshots

Add screenshots of:

* Main Menu
* Laboratory Environment
* Enemy Encounter
* Power Box Minigame
* Victory Screen

---

## Demo Video

YouTube/Vimeo Link:



---

## Academic Purpose

This project was created for educational purposes as part of the Computer Graphics course at the National University of Engineering (UNI), Nicaragua.

