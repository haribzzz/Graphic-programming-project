# DarkZone: Abandoned Laboratory

## About the Project

DarkZone: Abandoned Laboratory is a first-person horror game developed as the final project for the Computer Graphics course at the National University of Engineering (UNI, Nicaragua).

The player explores an abandoned scientific laboratory and must restore the power supply by activating two control boxes located throughout the map. While completing objectives, the player must avoid a hostile creature that patrols the facility. The game creates a tense atmosphere through dynamic lighting, a limited flashlight battery system, environmental audio, and a night skybox.

---

## Authors

* Robleto Valdivia Aura Abihail 
* Cross Ramírez Idhe Isabel
* Ampie Amaya Harisema Milagros 
* Rayo Rocha Edith María 

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
├── Origen/
        ├── Main.cpp
        ├── Model.cpp
        ├── Mesh.cpp
        ├── Enemy.cpp
        ├── Menu.cpp
        ├── TextRenderer.cpp
        ├── Interaction.cpp
        ├── glad.c
├── Encabezado/
        ├── Model.h
        ├── Mesh.h  
        ├── Enemy.h  
        ├── Menu.h 
        ├── TextRenderer.h 
        ├── Interaction.h 
├── Shaders/
        ├── skybox.frag
        ├── skybox.vert 
        ├── default.frag  
        ├── default.vert
├── Librerias/
        ├── stb_image/
        ├── Assimp/ 
        ├── GLAD/ 
        ├── GLFW/
        ├── glm/
├── Textures/
├── Sonidos/
├── README_ES.md
└── README.md
```

## How to Clone the Repository

```bash
https://github.com/haribzzz/Graphic-programming-project.git
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
  <img width="1365" height="694" alt="image" src="https://github.com/user-attachments/assets/5ce6527c-2f9c-4f6c-bfcc-1e4c635c6b7e" />

* Laboratory Environment
  <img width="1365" height="693" alt="image" src="https://github.com/user-attachments/assets/f1e42108-81e1-4ab0-9f34-72eadd3281c9" />

* Enemy Encounter
  <img width="1365" height="697" alt="image" src="https://github.com/user-attachments/assets/27fffaa6-55c5-413a-82df-ca75d016cef9" />

* Power Box Minigame
  <img width="1365" height="698" alt="image" src="https://github.com/user-attachments/assets/d79b18e4-b2dd-44b8-b415-117bde203ef0" />

* Victory Screen
  <img width="1365" height="693" alt="image" src="https://github.com/user-attachments/assets/09d8717b-0974-4bbe-a4a9-ae57dd44e3b6" />


---

## Demo Video

YouTube/Vimeo Link:


---

## Academic Purpose

This project was created for educational purposes as part of the Computer Graphics course at the National University of Engineering (UNI), Nicaragua.

