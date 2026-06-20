# DarkZone: Laboratorio Abandonado

## Acerca del Proyecto

DarkZone: Laboratorio Abandonado es un videojuego de terror en primera persona desarrollado como proyecto final de la asignatura de Programación Gráfica de la Universidad Nacional de Ingeniería (UNI Nicaragua).

El jugador explora un laboratorio científico abandonado y debe restaurar el suministro eléctrico activando dos cajas de control distribuidas por el mapa. Mientras completa los objetivos, deberá evitar ser capturado por una criatura hostil que patrulla las instalaciones. El juego crea una atmósfera de tensión mediante iluminación dinámica, un sistema de batería limitada para la linterna, audio ambiental y un skybox nocturno.

---

## Integrantes

* Aura Abihail Robleto Valdivia
* Idhe Isabel Cross Ramírez
* Harisema Milagros Ampie Amaya
* Edith María Rayo Rocha

---

## Tecnologías Utilizadas

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

## Características Principales

* Jugabilidad de terror en primera persona.
* Sistema de iluminación dinámica.
* Linterna con administración de batería.
* Sistema interactivo de puertas y llaves.
* Minijuego de conexión de cables en los powerboxes.
* Inteligencia artificial del enemigo con múltiples estados de comportamiento.
* Modelos 3D animados mediante animación esquelética.
* Sistema de colisiones basado en zonas AABB.
* Entorno con skybox nocturno.
* Menú principal, menú de pausa, pantalla de victoria y pantalla de derrota.

---

## Estructura del Proyecto

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
├── Origen/
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
├── Models/
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

## Cómo Clonar el Repositorio

```bash
git clone https://github.com/haribzzz/Graphic-programming-project.git
```

Luego ingrese a la carpeta del proyecto:

```bash
cd Graphic-programming-project
```

---

## Cómo Ejecutar el Proyecto

1. Clonar el repositorio.

2. Abrir el proyecto en Visual Studio.

3. Asegurarse de que las siguientes bibliotecas estén correctamente configuradas:

   * GLFW
   * GLAD
   * GLM
   * Assimp
   * miniaudio

4. Compilar la solución.

5. Ejecutar el programa.

---

## Controles

| Tecla   | Acción                     |
| ------- | -------------------------- |
| W A S D | Movimiento                 |
| Shift   | Correr                     |
| Ctrl    | Agacharse                  |
| Espacio | Saltar                     |
| F       | Encender / Apagar linterna |
| E       | Interactuar                |
| R G B   | Minijuego de PowerBox      |
| P       | Cancelar minijuego         |
| ESC     | Menú de pausa              |
| ALT     | Bloquear / Liberar cursor  |

---

## Capturas de Pantalla

Agregar capturas de:

* Menú principal
  <img width="1365" height="694" alt="image" src="https://github.com/user-attachments/assets/5a394aa7-d1c9-422d-9f5e-fcc7b1380843" />

* Entorno del laboratorio
* Encuentro con el enemigo
* Minijuego de PowerBox
* Pantalla de victoria

---

## Video Demo

Enlace de YouTube o Vimeo:

(Pegar enlace aquí)

---

## Propósito Académico

Este proyecto fue desarrollado con fines académicos como parte de la asignatura de Programación Gráfica de la Universidad Nacional de Ingeniería (UNI), Nicaragua.
