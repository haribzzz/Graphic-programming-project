#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image.h>
#include "Interaction.h"
#include <iostream>
#include "Model.h"
#include "stb_easy_font.h"
#include "Menu.h"
#include "TextRenderer.h"
#include <vector>
#include <unordered_map>
#include <cmath>
#include "Enemy.h"

ma_engine audioEngine;
ma_sound  footstepsSound;
bool      footstepsInitialized = false;

float stepTimer = 0.0f;
// =====================================
// SHADERS
// =====================================
const char* vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in ivec4 aBoneIDs;
layout(location = 4) in vec4  aWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool animated;
uniform mat4 boneMatrices[100];

void main()
{
    vec4 pos    = vec4(aPos, 1.0);
    vec3 normal = aNormal;

    if (animated)
    {
        vec4 skinnedPos = vec4(0.0);
        vec3 skinnedNrm = vec3(0.0);

        for (int i = 0; i < 4; i++)
        {
            if (aBoneIDs[i] == -1) continue;
            mat4 bm = boneMatrices[aBoneIDs[i]];
            skinnedPos += aWeights[i] * (bm * vec4(aPos, 1.0));
            skinnedNrm += aWeights[i] * (mat3(bm) * aNormal);
        }
        pos    = skinnedPos;
        normal = skinnedNrm;
    }

    FragPos  = vec3(model * pos);
    Normal   = mat3(transpose(inverse(model))) * normal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// ─────────────────────────────────────────────────────────────
// FRAGMENT SHADER
// ─────────────────────────────────────────────────────────────
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform bool useMultiTexture;

uniform vec3 lightPos;
uniform vec3 lightDir;
uniform bool flashlightOn;

uniform vec3 lampPos;
uniform vec3 lampPos2;
uniform vec3 lampPos3;
uniform vec3 lampPos4;
uniform vec3 lampPos5;
uniform vec3 lampPos6;

uniform bool lampara1On;
uniform bool lampara2On;
uniform bool lampara3On;
uniform bool lampara4On;
uniform bool lampara5On;
uniform bool lampara6On;

void main()
{
    vec3 color;
    if (useMultiTexture)
    {
        vec3 diffuse = texture(texture1, TexCoord).rgb;
        float ao = texture(texture2, TexCoord).r;
        float metal = texture(texture4, TexCoord).r;
        color = diffuse * ao;
        color = mix(color, color * 0.5, metal);
    }
    else
    {
        color = texture(texture1, TexCoord).rgb;
    }

    vec3 norm = normalize(Normal);
    vec3 ambient = color * 0.08;

    vec3 flashlight = vec3(0.0);
    if (flashlightOn)
    {
        vec3 toFrag = normalize(FragPos - lightPos);
        float theta = dot(toFrag, normalize(lightDir));
        float innerCutoff = 0.978;
        float outerCutoff = 0.956;
        float coneInt = smoothstep(outerCutoff, innerCutoff, theta);
        vec3 ldir = normalize(lightPos - FragPos);
        float diff = abs(dot(norm, ldir));
        float dist = length(lightPos - FragPos);
        float att = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);
        flashlight = diff * coneInt * att * 3.0 * vec3(1.0, 0.98, 0.9);
    }

    vec3 lampLight = vec3(0.0);
    vec3 lampLight2 = vec3(0.0);
    vec3 lampLight3 = vec3(0.0);
    vec3 lampLight4 = vec3(0.0);
    vec3 lampLight5 = vec3(0.0);
    vec3 lampLight6 = vec3(0.0);

    if (lampara1On) {
        vec3 d = normalize(lampPos - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara2On) {
        vec3 d = normalize(lampPos2 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos2 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight2 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara3On) {
        vec3 d = normalize(lampPos3 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos3 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight3 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara4On) {
        vec3 d = normalize(lampPos4 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos4 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight4 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara5On) {
        vec3 d = normalize(lampPos5 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos5 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight5 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }
    if (lampara6On) {
        vec3 d = normalize(lampPos6 - FragPos);
        float df = abs(dot(norm, d));
        float ds = length(lampPos6 - FragPos);
        float a = 1.0 / (1.0 + 0.015 * ds + 0.003 * ds * ds);
        lampLight6 = df * a * 2.2 * vec3(1.0, 0.95, 0.8);
    }

    vec3 result = (ambient + flashlight + lampLight + lampLight2 + lampLight3 + lampLight4 + lampLight5 + lampLight6) * color;
    FragColor = vec4(result, 1.0);
}
)";

// =====================================
// SKYBOX SHADERS
// =====================================
const char* skyboxVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = vec4(pos.xy, pos.w, pos.w);
}
)";

const char* skyboxFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}
)";

// =====================================
// MAZE SCALE
// =====================================
const float MAZE_SCALE = 2.0f;
const float MAZE_FLOOR_Y = 2.800f * MAZE_SCALE;   // 5.60
const float MAZE_CEILING_Y = 5.200f * MAZE_SCALE;   // 10.40
const float GROUND_LEVEL = MAZE_FLOOR_Y + 2.0f;   // 7.60
const float CEILING_LIMIT = MAZE_CEILING_Y - 0.3f; // 10.10

// =====================================
// COLLISION MESH (con grilla espacial para rendimiento)
// =====================================
struct Triangle {
    glm::vec3 a, b, c;
    float minX, maxX, minZ, maxZ, minY, maxY;
};
std::vector<Triangle> collisionMesh;

// Grilla espacial: divide el mundo en celdas de CELL_SIZE unidades.
// Cada celda guarda los indices de los triangulos que la tocan.
const float CELL_SIZE = 2.0f;
std::unordered_map<long long, std::vector<int>> collisionGrid;

inline long long CellKey(int cx, int cz)
{
    // Combina dos enteros en una sola clave de 64 bits
    return ((long long)(cx + 100000) << 32) | (unsigned int)(cz + 100000);
}

inline int ToCell(float coord)
{
    return (int)std::floor(coord / CELL_SIZE);
}

void LoadCollisionMesh(const char* path, float scale)
{
    Assimp::Importer imp;
    const aiScene* scene = imp.ReadFile(path,
        aiProcess_Triangulate | aiProcess_PreTransformVertices);

    if (!scene || !scene->mRootNode) {
        std::cout << "ERROR cargando malla de colision: " << imp.GetErrorString() << std::endl;
        return;
    }

    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            aiFace& face = mesh->mFaces[f];
            if (face.mNumIndices != 3) continue;

            Triangle tri;
            auto toV = [&](unsigned int idx) {
                aiVector3D& v = mesh->mVertices[idx];
                return glm::vec3(v.x * scale, v.y * scale, v.z * scale);
                };
            tri.a = toV(face.mIndices[0]);
            tri.b = toV(face.mIndices[1]);
            tri.c = toV(face.mIndices[2]);

            const float RADIUS = 0.0000000008f;
            tri.minX = glm::min(glm::min(tri.a.x, tri.b.x), tri.c.x) - RADIUS;
            tri.maxX = glm::max(glm::max(tri.a.x, tri.b.x), tri.c.x) + RADIUS;
            tri.minZ = glm::min(glm::min(tri.a.z, tri.b.z), tri.c.z) - RADIUS;
            tri.maxZ = glm::max(glm::max(tri.a.z, tri.b.z), tri.c.z) + RADIUS;
            tri.minY = glm::min(glm::min(tri.a.y, tri.b.y), tri.c.y);
            tri.maxY = glm::max(glm::max(tri.a.y, tri.b.y), tri.c.y) + 12.0f;

            collisionMesh.push_back(tri);
        }
    }

    // Construir la grilla espacial: cada triangulo se registra en todas
    // las celdas que su AABB toca
    collisionGrid.clear();
    for (int i = 0; i < (int)collisionMesh.size(); i++)
    {
        const Triangle& tri = collisionMesh[i];
        int cxMin = ToCell(tri.minX), cxMax = ToCell(tri.maxX);
        int czMin = ToCell(tri.minZ), czMax = ToCell(tri.maxZ);

        for (int cx = cxMin; cx <= cxMax; cx++)
            for (int cz = czMin; cz <= czMax; cz++)
                collisionGrid[CellKey(cx, cz)].push_back(i);
    }

    std::cout << "Malla de colision cargada: " << collisionMesh.size()
        << " triangulos en " << collisionGrid.size() << " celdas." << std::endl;
}

struct MazeZone { float minX, maxX, minZ, maxZ; };
const MazeZone mazeZones[] = {
    { -28.71f,  -0.33f,  -4.00f,   4.00f  },
    {  -0.37f,   7.67f, -32.07f,  41.18f  },
    {  -0.33f,  35.79f,  -4.00f,   4.04f  },
    {  35.75f,  43.76f, -32.09f,  32.11f  },
    {   7.62f,  43.74f,  32.02f,  40.03f  },
    {  -0.33f,   9.89f,  32.03f,  40.03f  },
    {  35.75f,  71.87f, -40.07f, -32.06f  },
    {  71.84f,  79.85f, -40.07f,  -3.95f  },
    {   4.05f,  20.35f,  14.84f,  27.80f  },
    { -13.04f,   3.26f, -28.88f, -15.92f  },
    {  51.90f,  64.86f, -35.69f, -19.39f  },
};
const int mazeZoneCount = (int)(sizeof(mazeZones) / sizeof(MazeZone));

bool InAnyMazeZone(glm::vec3 pos)
{
    for (int i = 0; i < mazeZoneCount; i++)
    {
        const MazeZone& z = mazeZones[i];
        if (pos.x > z.minX && pos.x < z.maxX &&
            pos.z > z.minZ && pos.z < z.maxZ)
            return true;
    }
    return false;
}

// Version optimizada: solo revisa los triangulos de la celda donde
// esta parado el jugador (y celdas vecinas), no la malla completa.
bool CollidesWithMesh(glm::vec3 pos)
{
    if (collisionMesh.empty()) return false;

    int cx = ToCell(pos.x);
    int cz = ToCell(pos.z);

    // Revisar la celda actual y las 8 vecinas por seguridad en los bordes
    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dz = -1; dz <= 1; dz++)
        {
            auto it = collisionGrid.find(CellKey(cx + dx, cz + dz));
            if (it == collisionGrid.end()) continue;

            for (int idx : it->second)
            {
                const Triangle& tri = collisionMesh[idx];
                if (pos.x > tri.minX && pos.x < tri.maxX &&
                    pos.z > tri.minZ && pos.z < tri.maxZ &&
                    pos.y > tri.minY && pos.y < tri.maxY)
                    return true;
            }
        }
    }
    return false;
}

// =====================================
// DOORS
// =====================================
bool  doorOpen = false;
float doorAngle = 0.0f;
bool  ePressed = false;

bool  door2Open = false;
float door2Angle = 0.0f;
bool  ePressed2 = false;

bool door3Open = false;
float door3Angle = -90.0f;
bool ePressed3 = false;

bool door4Open = false;
float door4Angle = -90.0f;
bool ePressed4 = false;

bool door5Open = false;
float door5Angle = -90.0f;
bool ePressed5 = false;

bool door6Open = false;
float door6Angle = 0.0f;
bool ePressed6 = false;

bool messageShown = false;

// Posiciones de puertas en espacio mundo (x2) - PARA DIBUJAR
const glm::vec3 doorPos = glm::vec3(7.5f, 12.6f, 21.9f);
const glm::vec3 door2Pos = glm::vec3(-0.3f, 12.6f, -21.9f);
const glm::vec3 door3Pos = glm::vec3(57.6f, 12.6f, -32.1f);
const glm::vec3 door4Pos = glm::vec3(21.6f, 12.6f, 68.0f);
const glm::vec3 door5Pos = glm::vec3(61.9f, 12.6f, 40.0f);
const glm::vec3 door6Pos = glm::vec3(79.7f, 12.6f, -58.1f);
// ⬇️ POSICIONES PARA INTERACCIÓN (a la altura de los ojos del jugador)
const glm::vec3 doorInteractPos = glm::vec3(7.5f, GROUND_LEVEL, 21.9f);
const glm::vec3 door2InteractPos = glm::vec3(-0.3f, GROUND_LEVEL, -21.9f);
const glm::vec3 door3InteractPos = glm::vec3(57.6f, GROUND_LEVEL, -32.1f);
const glm::vec3 door4InteractPos = glm::vec3(21.6f, GROUND_LEVEL, 68.0f);
const glm::vec3 door5InteractPos = glm::vec3(61.9f, GROUND_LEVEL, 40.0f);
const glm::vec3 door6InteractPos = glm::vec3(79.7f, GROUND_LEVEL, -58.1f);

// =====================================
// KEY
// =====================================
bool      tieneLlave = false;
bool      llaveRecogida = false;
glm::vec3 llavePosicion = glm::vec3(3.0f, MAZE_FLOOR_Y + 0.5f, 21.3f);

// =====================================
// BATERIAS
// =====================================
struct Bateria { glm::vec3 pos; bool recogida = false; };
Bateria baterias[5] = {
    { glm::vec3(40.0f, MAZE_FLOOR_Y + 0.5f, 36.0f), false }, // pasillo derecho
    { glm::vec3(-4.0f, MAZE_FLOOR_Y + 0.5f, -20.0f), false }, // habitacion 2
    { glm::vec3(80.0f, MAZE_FLOOR_Y + 0.5f, -45.0f), false }, // habitacion 4
	{ glm::vec3(30.0f, MAZE_FLOOR_Y + 0.5f, 60.0f), false }, // habitacion 5
};

// =====================================
// DOCUMENTOS
// =====================================
struct Documento { glm::vec3 pos; bool recogido = false; const char* mensaje; };
Documento documentos[3] = {
    { glm::vec3(10.0f, MAZE_FLOOR_Y + 0.5f, 20.0f), false,
      "Day 47 - The specimens are showing\nsigns of mutation. We must restore\npower immediately." },
    { glm::vec3(-5.0f, MAZE_FLOOR_Y + 0.5f, -25.0f), false,
      "Emergency protocol: activate both\npowerboxes to unlock the\nemergency exit." },
    { glm::vec3(58.0f, MAZE_FLOOR_Y + 0.5f, -25.0f), false,
      "It's too late. The creature has\nescaped containment. God help\nwhoever finds this." }
};
bool mostrandoDocumento = false;
float timerDocumento = 0.0f;
const char* textoDocumento = "";

// =====================================
// SISTEMA DE TAREAS
// =====================================
std::string notificacionTarea = "";
float timerNotificacion = 0.0f;
const float DURACION_NOTIFICACION = 4.0f;
bool tareaIniciada = false;
bool pb1Notificado = false;
bool pb2Notificado = false;

// =====================================
// FLASHLIGHT
// =====================================
bool  flashlightOn = true;
bool  fPressed = false;
float battery = 100.0f;
float flickerTimer = 0.0f;
bool  flickerState = true;

// =====================================
// POWERBOX MINIGAME
// =====================================
bool powerbox1Activado = false;
bool powerbox2Activado = false;

glm::vec3 powerbox1Pos = glm::vec3(-12.7f, MAZE_FLOOR_Y + 1.5f, -22.0f);
glm::vec3 powerbox2Pos = glm::vec3(60.0f, MAZE_FLOOR_Y + 1.5f, -19.8f);

bool minijuegoActivo = false;
bool ePressedPowerbox = false;
int  cableActual = 0;
int  cablesCargados = 0;
int  powerboxActual = 0;

bool lampara1Activa = false;
bool lampara2Activa = false;
bool lampara3Activa = false;
bool lampara4Activa = false;
bool lampara5Activa = false;  
bool lampara6Activa = false;  

const glm::vec3 lampEntrada = glm::vec3(-14.5f, MAZE_CEILING_Y - 0.2f, 0.0f);
const glm::vec3 lampHab1 = glm::vec3(12.2f, MAZE_CEILING_Y - 0.2f, 21.3f);
const glm::vec3 lampHab2 = glm::vec3(-4.9f, MAZE_CEILING_Y - 0.2f, -22.4f);
const glm::vec3 lampHab3 = glm::vec3(58.4f, MAZE_CEILING_Y - 0.2f, -27.5f);
const glm::vec3 lampHab4 = glm::vec3(80.0f, MAZE_CEILING_Y - 0.2f, -45.0f);  
const glm::vec3 lampHab5 = glm::vec3(60.0f, MAZE_CEILING_Y - 0.2f, -25.0f);  

// =====================================
// SCREEN / CAMERA
// =====================================
int SCREEN_W = 1280;
int SCREEN_H = 720;

glm::vec3 cameraPos = glm::vec3(-24.0f, GROUND_LEVEL, 0.0f);
glm::vec3 cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float yaw = 0.0f;
float pitch = 0.0f;
float lastX = 640.0f;
float lastY = 360.0f;
bool  firstMouse = true;

bool altPressed = false;
bool cursorFree = false;
bool pauseMenuOpen = false;
bool escPressed = false;

float velocityY = 0.0f;
bool  onGround = true;
float gravity = -20.0f;
float jumpForce = 6.5f;

bool playerCaught = false;

// =====================================
// CORRER / AGACHARSE
// =====================================
bool  isCrouching = false;
bool  isRunning = false;

const float WALK_SPEED = 3.5f;
const float RUN_SPEED = 6.0f;
const float CROUCH_SPEED = 1.8f;

const float STAND_HEIGHT = GROUND_LEVEL;
const float CROUCH_HEIGHT = GROUND_LEVEL - 1.2f;
const float CROUCH_LERP_SPEED = 8.0f;

float currentEyeHeight = GROUND_LEVEL;

float stamina = 100.0f;
const float STAMINA_DRAIN = 20.0f;
const float STAMINA_REGEN = 12.0f;

// =====================================
// RESET GAME
// =====================================
void ResetGame()
{
    cameraPos = glm::vec3(-24.0f, GROUND_LEVEL, 0.0f);
    cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
    yaw = 0.0f;
    pitch = 0.0f;
    velocityY = 0.0f;
    onGround = true;
    firstMouse = true;

    isCrouching = false;
    isRunning = false;
    currentEyeHeight = GROUND_LEVEL;
    stamina = 100.0f;

    doorOpen = false;
    doorAngle = 0.0f;
    ePressed = false;

    door2Open = false;
    door2Angle = 0.0f;
    ePressed2 = false;

    door3Open = false;
    door3Angle = -90.0f;
    ePressed3 = false;

    door4Open = false;
    door4Angle = -90.0f;
    ePressed4 = false;

    door5Open = false;
    door5Angle = -90.0f;
    ePressed5 = false;

    door6Open = false;
    door6Angle = 0.0f;
    ePressed6 = false;

    tieneLlave = false;
    llaveRecogida = false;

    for (auto& b : baterias) b.recogida = false;
    for (auto& d : documentos) d.recogido = false;
    mostrandoDocumento = false;
    timerDocumento = 0.0f;
    textoDocumento = "";

    notificacionTarea = "";
    timerNotificacion = 0.0f;
    tareaIniciada = false;
    pb1Notificado = false;
    pb2Notificado = false;

    flashlightOn = true;
    battery = 100.0f;
    flickerTimer = 0.0f;
    flickerState = true;
    fPressed = false;

    powerbox1Activado = false;
    powerbox2Activado = false;
    minijuegoActivo = false;
    ePressedPowerbox = false;
    cableActual = 0;
    cablesCargados = 0;
    powerboxActual = 0;
    lampara1Activa = false;
    lampara2Activa = false;
    lampara3Activa = false;
    lampara4Activa = false;
    lampara5Activa = false;  
    lampara6Activa = false;  

    cursorFree = false;
    pauseMenuOpen = false;
    escPressed = false;
    altPressed = false;
    messageShown = false;
}

// =====================================
// MOUSE LOOK
// =====================================
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (cursorFree) return;
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }

    float sens = (g_Menu ? g_Menu->sensitivity : 0.1f);
    float xoffset = ((float)xpos - lastX) * sens;
    float yoffset = (lastY - (float)ypos) * sens;
    lastX = (float)xpos; lastY = (float)ypos;

    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

// =====================================
// INPUT / MOVEMENT / COLLISIONS
// =====================================
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escPressed)
    {
        escPressed = true;
        pauseMenuOpen = !pauseMenuOpen;
        if (pauseMenuOpen) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPos(window, SCREEN_W / 2.0, SCREEN_H / 2.0);
            cursorFree = true;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cursorFree = false; firstMouse = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE) escPressed = false;

    bool altHeld = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
    if (altHeld && !altPressed) {
        altPressed = true; cursorFree = !cursorFree;
        if (cursorFree) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPos(window, SCREEN_W / 2.0, SCREEN_H / 2.0);
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }
    if (!altHeld) altPressed = false;

    if (cursorFree || pauseMenuOpen || minijuegoActivo) return;

    float speed = 3.5f * deltaTime;
    glm::vec3 previousPos = cameraPos;

    glm::vec3 forward;
    forward.x = cos(glm::radians(yaw));
    forward.y = 0.0f;
    forward.z = sin(glm::radians(yaw));
    forward = glm::normalize(forward);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    bool ctrlHeld = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

    isCrouching = ctrlHeld && onGround;

    bool shiftHeld = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    bool wantsToMove = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;

    isRunning = shiftHeld && !isCrouching && wantsToMove && stamina > 0.0f;

    if (isRunning) {
        stamina -= STAMINA_DRAIN * deltaTime;
        if (stamina < 0.0f) stamina = 0.0f;
    }
    else {
        stamina += STAMINA_REGEN * deltaTime;
        if (stamina > 100.0f) stamina = 100.0f;
    }

    if (isCrouching)
        speed = CROUCH_SPEED * deltaTime;
    else if (isRunning)
        speed = RUN_SPEED * deltaTime;
    else
        speed = WALK_SPEED * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += forward * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= forward * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += right * speed;

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fPressed) {
        fPressed = true; flashlightOn = !flashlightOn;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fPressed = false;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround && !isCrouching) {
        velocityY = jumpForce; onGround = false;
    }

    velocityY += gravity * deltaTime;

    if (!onGround) {
        cameraPos.y += velocityY * deltaTime;
    }

    if (cameraPos.y <= GROUND_LEVEL && !onGround) {
        onGround = true;
        velocityY = 0.0f;
        currentEyeHeight = GROUND_LEVEL;
    }

    if (cameraPos.y > CEILING_LIMIT) { cameraPos.y = CEILING_LIMIT; velocityY = 0.0f; }

    float targetHeight = isCrouching ? CROUCH_HEIGHT : STAND_HEIGHT;

    if (onGround) {
        currentEyeHeight += (targetHeight - currentEyeHeight) * CROUCH_LERP_SPEED * deltaTime;
        cameraPos.y = currentEyeHeight;
    }

    bool inMesh = collisionMesh.empty()
        ? InAnyMazeZone(cameraPos)
        : CollidesWithMesh(cameraPos);

    if (!inMesh)
        cameraPos = previousPos;

    // =====================================
    // COLISIONES DE OBJETOS EN ESCENA
    // =====================================
    auto hitBox = [&](float minX, float maxX, float minZ, float maxZ) -> bool {
        return cameraPos.x > minX && cameraPos.x < maxX &&
            cameraPos.z > minZ && cameraPos.z < maxZ;
        };

    // Escalera — (-27, -2) — REDUCIDO para no bloquear el pasillo
    if (hitBox(-27.8f, -26.2f, -2.8f, -1.2f)) cameraPos = previousPos;

    // Computadora entrada — (-25, 3) — REDUCIDO para no bloquear el pasillo
    if (hitBox(-26.0f, -24.5f, 2.5f, 4.2f))  cameraPos = previousPos;

    // Mesa 2 (debajo computadora) — (-25, 3) — REDUCIDO para no bloquear el pasillo
    if (hitBox(-26.5f, -24.0f, 2.2f, 4.5f)) cameraPos = previousPos;

    // Mesa habitacion 1 — (12, 18)
    if (hitBox(10.0f, 14.5f, 16.5f, 19.5f)) cameraPos = previousPos;

    // Librera habitacion 1 — (9, 26)
    if (hitBox(7.5f, 10.5f, 24.0f, 28.0f)) cameraPos = previousPos;

    // Sillas pasillo (6.5, 45.7) y (6.5, 55.7) y (6.5, 63.7)
    for (float sz : {45.7f, 55.7f, 63.7f})
        if (hitBox(5.3f, 7.7f, sz - 1.2f, sz + 1.2f)) cameraPos = previousPos;

    // Sillas en X=15,25,35 Z=38.7
    for (float sx : {15.0f, 25.0f, 35.0f})
        if (hitBox(sx - 1.2f, sx + 1.2f, 37.5f, 39.9f)) cameraPos = previousPos;

    // Sillas en X=42.5 con varias Z
    for (float sz : {-28.7f, -18.7f, -8.7f, 8.7f, 18.7f, 28.7f})
        if (hitBox(41.3f, 43.7f, sz - 1.2f, sz + 1.2f)) cameraPos = previousPos;

    // Silla en (48, 38.7) y (70, 38.7)
    for (float sx : {48.0f, 70.0f})
        if (hitBox(sx - 1.2f, sx + 1.2f, 37.5f, 39.9f)) cameraPos = previousPos;

    // Cápsula habitacion 2 — (-4, -27)
    if (hitBox(-6.0f, -2.0f, -29.0f, -25.0f)) cameraPos = previousPos;

    // Cajas habitacion 2 — (-11, -17)
    if (hitBox(-13.0f, -9.0f, -19.0f, -15.0f)) cameraPos = previousPos;

    // Especimen habitacion 3 — (62, -31)
    if (hitBox(59.0f, 65.0f, -34.0f, -28.0f)) cameraPos = previousPos;

    // Esqueleto — (62, -22)
    if (hitBox(61.0f, 63.0f, -23.5f, -20.5f)) cameraPos = previousPos;

    // Mesa lab — (17, 25)
    if (hitBox(15.0f, 19.0f, 23.0f, 27.0f)) cameraPos = previousPos;

    // Laboratorio set — (54, -21)
    if (hitBox(51.0f, 57.0f, -23.5f, -18.5f)) cameraPos = previousPos;

    // Powerbox 1 y 2
    if (hitBox(powerbox1Pos.x - 1.5f, powerbox1Pos.x + 1.5f, powerbox1Pos.z - 1.5f, powerbox1Pos.z + 1.5f))
        cameraPos = previousPos;
    if (hitBox(powerbox2Pos.x - 1.5f, powerbox2Pos.x + 1.5f, powerbox2Pos.z - 1.5f, powerbox2Pos.z + 1.5f))
        cameraPos = previousPos;

    // Lavabos — (53,-30) y (53,-33)
    if (hitBox(51.5f, 54.5f, -31.5f, -28.5f)) cameraPos = previousPos;
    if (hitBox(51.5f, 54.5f, -34.5f, -31.5f)) cameraPos = previousPos;

    // Objetos pasillo derecho — (70,-33), (73,-22), (73,-7), (73,5), (73,20)
    if (hitBox(68.5f, 71.5f, -34.5f, -31.5f)) cameraPos = previousPos;
    for (float sz : {-22.0f, -7.0f, 5.0f, 20.0f})
        if (hitBox(71.5f, 74.5f, sz - 1.5f, sz + 1.5f)) cameraPos = previousPos;

    // Objetos zona norte — (10,75), (25,75), (40,75)
    for (float sx : {10.0f, 25.0f, 40.0f})
        if (hitBox(sx - 1.5f, sx + 1.5f, 73.5f, 76.5f)) cameraPos = previousPos;

    // Gabinetes (3)
    for (float sx : {89.0f, 86.0f, 83.0f})
        if (hitBox(sx - 1.5f, sx + 1.5f, -54.5f, -51.5f)) cameraPos = previousPos;

    // Cajas amontonadas (2)
    for (float sx : {82.0f, 90.0f})
        if (hitBox(sx - 2.0f, sx + 2.0f, -66.0f, -62.0f)) cameraPos = previousPos;

    // Consolas (4)
    for (float sx : {30.0f, 25.0f, 20.0f, 15.0f})
        if (hitBox(sx - 1.2f, sx + 1.2f, 65.5f, 68.5f)) cameraPos = previousPos;

    // Oficinas (4)
    for (float sx : {30.0f, 25.0f, 20.0f, 15.0f})
        if (hitBox(sx - 1.5f, sx + 1.5f, 58.5f, 61.5f)) cameraPos = previousPos;

    // Pizarras (3)
    for (float sx : {63.0f, 59.0f, 54.0f})
        if (hitBox(sx - 1.5f, sx + 1.5f, 47.5f, 50.5f)) cameraPos = previousPos;

    // Camillas (4)
    for (float sx : {60.0f, 55.0f})
        for (float sz : {48.0f, 44.0f})
            if (hitBox(sx - 1.2f, sx + 1.2f, sz - 1.0f, sz + 1.0f)) cameraPos = previousPos;

    // Cuerpos en el suelo (3) - Tienen escala 0.01 pero colisionan como bultos
    for (glm::vec3 p : {glm::vec3(52.0f, 0.0f, 41.0f), glm::vec3(52.0f, 0.0f, 45.0f), glm::vec3(64.0f, 0.0f, 45.0f)})
        if (hitBox(p.x - 0.5f, p.x + 0.5f, p.z - 0.5f, p.z + 0.5f)) cameraPos = previousPos;

    // Lamparas de techo (4) - Colision en el suelo justo debajo de ellas
    for (glm::vec3 p : {glm::vec3(-14.5f, 0.0f, 0.0f), glm::vec3(12.2f, 0.0f, 21.3f), glm::vec3(-4.9f, 0.0f, -22.4f), glm::vec3(58.4f, 0.0f, -27.5f)})
        if (hitBox(p.x - 0.8f, p.x + 0.8f, p.z - 0.8f, p.z + 0.8f)) cameraPos = previousPos;
    // =====================================
    // COLISIONES - HABITACIÓN FINAL
    // =====================================

    // Cajas amontonadas 3, 4, 5 (fila Z = -1.8)
    for (float sx : {116.4f, 120.4f, 124.4f})
        if (hitBox(sx - 2.0f, sx + 2.0f, -3.8f, 0.2f)) cameraPos = previousPos;

    // Cajas amontonadas 6, 7, 8 (fila Z = -5.0)
    for (float sx : {116.4f, 120.4f, 124.4f})
        if (hitBox(sx - 2.0f, sx + 2.0f, -7.0f, -3.0f)) cameraPos = previousPos;

    // Cuerpos 1, 2, 3, 4 (fila Z = -10.0)
    for (float sx : {110.0f, 115.0f, 120.0f, 125.0f})
        if (hitBox(sx - 0.8f, sx + 0.8f, -10.8f, -9.2f)) cameraPos = previousPos;

    // Cuerpos 5, 6, 7 (fila Z = -0.0005)
    for (float sx : {125.0f, 120.0f, 115.0f})
        if (hitBox(sx - 0.8f, sx + 0.8f, -0.8f, 0.8f)) cameraPos = previousPos;

}

// =====================================
// MAIN
// =====================================
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    SCREEN_W = mode->width;
    SCREEN_H = mode->height;
    lastX = SCREEN_W / 2.0f;
    lastY = SCREEN_H / 2.0f;

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(SCREEN_W, SCREEN_H, "DarkZone", NULL, NULL);
    glfwGetWindowSize(window, &SCREEN_W, &SCREEN_H);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetWindowIconifyCallback(window, [](GLFWwindow* w, int iconified) {
        if (!iconified) {
            GLFWmonitor* mon = glfwGetPrimaryMonitor();
            const GLFWvidmode* m = glfwGetVideoMode(mon);
            int sw, sh; glfwGetWindowSize(w, &sw, &sh);
            glfwSetWindowPos(w, (m->width - sw) / 2, (m->height - sh) / 2);
        }
        });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h)
        { SCREEN_W = w; SCREEN_H = h; glViewport(0, 0, w, h); });

    stbi_set_flip_vertically_on_load(false);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    InitTextRenderer();

    if (ma_engine_init(NULL, &audioEngine) != MA_SUCCESS)
        std::cout << "ERROR::MINIAUDIO: No se pudo iniciar el motor de audio." << std::endl;

    ma_sound ambientSound;
    if (ma_sound_init_from_file(&audioEngine, "Sonidos/ambient.wav",
        MA_SOUND_FLAG_STREAM, NULL, NULL, &ambientSound) == MA_SUCCESS) {
        ma_sound_set_looping(&ambientSound, MA_TRUE);
        ma_sound_start(&ambientSound);
    }
    else {
        ma_engine_play_sound(&audioEngine, "Sonidos/ambient.wav", NULL);
    }

    // ──PASOS ──
    if (ma_sound_init_from_file(&audioEngine, "Sonidos/footstepLoop.wav", 0, NULL, NULL, &footstepsSound) == MA_SUCCESS) {
        ma_sound_set_looping(&footstepsSound, MA_FALSE);
        footstepsInitialized = true;
    }

    g_Menu = new Menu();
    g_Menu->Init(SCREEN_W, SCREEN_H);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    stbi_set_flip_vertically_on_load(true);
    auto loadTexture = [](const char* path) -> GLuint {
        GLuint id; glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        int w, h, ch;
        unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
        if (data) {
            GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else std::cout << "ERROR loading texture: " << path << std::endl;
        stbi_image_free(data);
        return id;
        };

    GLuint tableTexture = loadTexture("Textures/mesa/wooden_table_02_diff_2k.jpg");

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLuint skyboxVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(skyboxVertexShader, 1, &skyboxVertexShaderSource, NULL);
    glCompileShader(skyboxVertexShader);

    GLuint skyboxFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(skyboxFragmentShader, 1, &skyboxFragmentShaderSource, NULL);
    glCompileShader(skyboxFragmentShader);

    GLuint skyboxShaderProgram = glCreateProgram();
    glAttachShader(skyboxShaderProgram, skyboxVertexShader);
    glAttachShader(skyboxShaderProgram, skyboxFragmentShader);
    glLinkProgram(skyboxShaderProgram);

    {
        GLint success;
        char infoLog[512];
        glGetShaderiv(skyboxVertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(skyboxVertexShader, 512, NULL, infoLog);
            std::cout << "ERROR SKYBOX VERTEX SHADER: " << infoLog << std::endl;
        }
        glGetShaderiv(skyboxFragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(skyboxFragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR SKYBOX FRAGMENT SHADER: " << infoLog << std::endl;
        }
        glGetProgramiv(skyboxShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(skyboxShaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR SKYBOX SHADER PROGRAM: " << infoLog << std::endl;
        }
    }

	//LLAMADOS A MODELOS
    stbi_set_flip_vertically_on_load(false);
    Model laberintoModel("Textures/mapa/Laboratorio.obj");

    LoadCollisionMesh("Textures/mapa/Colisiones.obj", MAZE_SCALE);

    stbi_set_flip_vertically_on_load(true);
    Model flashlightModel("Textures/linterna/scene.obj");
    Model tableModel("Textures/mesa/wooden_table_02_2k.obj");
    Model chandelierModel("Textures/lamp/lamp.obj");
    Model powerBox("Textures/powerbox/powerbox.obj");
    Model computadora_1Model("Textures/computadora_1/Ordenador.obj");
    Model cajasModel("Textures/cajas/cajas.obj");
    Model llaveModel("Textures/llave/llave.obj");
    Model papelModel("Textures/documento/paper.obj");
    Model escaleraModel("Textures/escalera/escalera.obj");
    Model capsulaModel("Textures/capsula/scene.obj");
    Model laboratorioModel("Textures/laboratorio/laboratorio.obj");
    Model esqueletoModel("Textures/esqueleto/esqueleto.obj");
    Model libreraModel("Textures/librera/librera.obj");
    Model Mesa_labModel("Textures/mesa_lab/mesa1.obj");
    Model especimenModel("Textures/especimen/especimen.obj");
    Model bateriaModel("Textures/bateria/bateria.obj");
    Model lavaboModel("Textures/lavabo/lavabo.obj");
    Model mesa_2Model("Textures/mesa_2/mesa_2.obj");
    Model sillaModel("Textures/sillas_conjunto/sillas_conjuntos.obj");
    Model escritorioModel("Textures/escritorio_2/sillas_2.obj");
	Model gabineteModel("Textures/gabinete/gabinete.obj");
	Model cajas2Model("Textures/cajas_amontonadas_1/cajas_2.obj");
	Model consolaModel("Textures/consola/consola.obj");
	Model oficinaModel("Textures/oficina/oficina.obj");
	Model pizarraModel("Textures/pizarra/pizarra.obj");
	Model cuerpoModel("Textures/cuerpo_con_bolsa/camilla_con_bolsa.obj");
	Model camillasModel("Textures/camillas/camillas.obj");

    Model doorModel("Textures/door/door.obj");

    Enemy enemy(
        "Textures/enemy/Parasite L Starkie.glb",
        "Textures/enemy/Idle.glb",
        "Textures/enemy/Swagger Walk.glb",
        "Textures/enemy/Zombie Run.glb"
    );
    enemy.position = glm::vec3(58.0f, MAZE_FLOOR_Y, -27.5f);
    enemy.detectionRange = 20.0f;
    enemy.chaseSpeed = 3.0f;
    enemy.fleeSpeed = 7.0f;
    enemy.flashAngleDeg = 25.0f;
    enemy.modelScale = 0.018f;

    enemy.pitchCorrectionDeg = 90.0f;

    enemy.collisionCheck = [](glm::vec3 pos) -> bool {
        return InAnyMazeZone(pos);
        };

    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "flashlightOn"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "animated"), 0);

    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    GLuint skyboxTexture;
    glGenTextures(1, &skyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

    std::vector<std::string> faces = {
        "Textures/skybox/nx.jpg",
        "Textures/skybox/px.jpg",
        "Textures/skybox/py.jpg",
        "Textures/skybox/ny.jpg",
        "Textures/skybox/pz.jpg",
        "Textures/skybox/nz.jpg"
    };

    for (unsigned int i = 0; i < faces.size(); i++) {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    float cubeVertices[] = {
        -0.5f,-0.5f, 0.5f,  0,0,1,  0,0,
         0.5f,-0.5f, 0.5f,  0,0,1,  1,0,
         0.5f, 0.5f, 0.5f,  0,0,1,  1,1,
         0.5f, 0.5f, 0.5f,  0,0,1,  1,1,
        -0.5f, 0.5f, 0.5f,  0,0,1,  0,1,
        -0.5f,-0.5f, 0.5f,  0,0,1,  0,0,
        -0.5f,-0.5f,-0.5f,  0,0,-1, 0,0,
         0.5f,-0.5f,-0.5f,  0,0,-1, 1,0,
         0.5f, 0.5f,-0.5f,  0,0,-1, 1,1,
         0.5f, 0.5f,-0.5f,  0,0,-1, 1,1,
        -0.5f, 0.5f,-0.5f,  0,0,-1, 0,1,
        -0.5f,-0.5f,-0.5f,  0,0,-1, 0,0,
        -0.5f, 0.5f, 0.5f, -1,0,0,  1,0,
        -0.5f, 0.5f,-0.5f, -1,0,0,  1,1,
        -0.5f,-0.5f,-0.5f, -1,0,0,  0,1,
        -0.5f,-0.5f,-0.5f, -1,0,0,  0,1,
        -0.5f,-0.5f, 0.5f, -1,0,0,  0,0,
        -0.5f, 0.5f, 0.5f, -1,0,0,  1,0,
         0.5f, 0.5f, 0.5f,  1,0,0,  1,0,
         0.5f, 0.5f,-0.5f,  1,0,0,  1,1,
         0.5f,-0.5f,-0.5f,  1,0,0,  0,1,
         0.5f,-0.5f,-0.5f,  1,0,0,  0,1,
         0.5f,-0.5f, 0.5f,  1,0,0,  0,0,
         0.5f, 0.5f, 0.5f,  1,0,0,  1,0,
        -0.5f,-0.5f,-0.5f,  0,-1,0, 0,1,
         0.5f,-0.5f,-0.5f,  0,-1,0, 1,1,
         0.5f,-0.5f, 0.5f,  0,-1,0, 1,0,
         0.5f,-0.5f, 0.5f,  0,-1,0, 1,0,
        -0.5f,-0.5f, 0.5f,  0,-1,0, 0,0,
        -0.5f,-0.5f,-0.5f,  0,-1,0, 0,1,
        -0.5f, 0.5f,-0.5f,  0,1,0,  0,1,
         0.5f, 0.5f,-0.5f,  0,1,0,  1,1,
         0.5f, 0.5f, 0.5f,  0,1,0,  1,0,
         0.5f, 0.5f, 0.5f,  0,1,0,  1,0,
        -0.5f, 0.5f, 0.5f,  0,1,0,  0,0,
        -0.5f, 0.5f,-0.5f,  0,1,0,  0,1
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO); glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);              glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);

    while (!glfwWindowShouldClose(window))
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (g_GameState == GameState::MENU)
        {
            float currentFrame = (float)glfwGetTime();
            lastFrame = currentFrame;
            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            GameState result = g_Menu->HandleInput(window, mouseX, mouseY);
            g_Menu->Render(mouseX, mouseY);
            if (result == GameState::PLAYING) {
                ResetGame(); g_GameState = GameState::PLAYING;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                cursorFree = false; firstMouse = true;
                lastFrame = (float)glfwGetTime();
            }
            else if (result == GameState::EXIT)
                glfwSetWindowShouldClose(window, true);
            glfwSwapBuffers(window); glfwPollEvents(); continue;
        }

        // ── DELTA TIME ──
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        processInput(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS &&
            !escPressed)
        {
            escPressed = true;

            pauseMenuOpen = !pauseMenuOpen;

            if (pauseMenuOpen)
            {
                glfwSetInputMode(
                    window,
                    GLFW_CURSOR,
                    GLFW_CURSOR_NORMAL
                );
            }
            else
            {
                glfwSetInputMode(
                    window,
                    GLFW_CURSOR,
                    GLFW_CURSOR_DISABLED
                );

                firstMouse = true;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE)
        {
            escPressed = false;
        }

        // Presiona la tecla 'C' para imprimir las coordenadas en la consola
        static bool cPressed = false;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cPressed) {
            cPressed = true;
            std::cout << "Coordenadas habitacion vacia: X = " << cameraPos.x << ", Z = " << cameraPos.z << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
            cPressed = false;
        }

        // ── Sonido de pasos con ritmo controlado ──
        if (footstepsInitialized) {
            bool moviendose = (
                glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
                glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS
                ) && onGround && !pauseMenuOpen && !minijuegoActivo && !cursorFree;

            if (moviendose) {
                // Asignamos el volumen según el estado
                float volPasos = isCrouching ? 0.10f : (isRunning ? 0.65f : 0.40f);
                ma_sound_set_volume(&footstepsSound, volPasos);

                float intervaloPasos = isCrouching ? 0.8f : (isRunning ? 0.32f : 0.55f);

                // El temporizador avanza con el tiempo real del juego
                stepTimer += deltaTime;

                // Si ya pasó el tiempo suficiente, disparamos un único paso limpio
                if (stepTimer >= intervaloPasos) {
                    if (ma_sound_is_playing(&footstepsSound)) {
                        ma_sound_stop(&footstepsSound);
                    }

                    ma_sound_seek_to_pcm_frame(&footstepsSound, 0); // Regresa el audio al inicio
                    ma_sound_start(&footstepsSound);               // 
                    stepTimer = 0.0f;                               // Reinicia el reloj
                }
            }
            else {
                // Si el jugador se detiene, reiniciamos el reloj para que el siguiente paso suene al instante
                stepTimer = 0.5f;
            }
        }
        // ── CONSUMO DE BATERÍA ──
        if (flashlightOn) {
            battery -= 0.5f * deltaTime;
            if (battery < 0.0f) { battery = 0.0f; flashlightOn = false; }
        }
        if (battery < 20.0f) {
            flickerTimer += deltaTime;
            if (flickerTimer > 0.08f) { flickerTimer = 0.0f; flickerState = !flickerState; }
        }
        else flickerState = true;
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        glUniform1i(glGetUniformLocation(shaderProgram, "lampara1On"), lampara1Activa ? 1 : 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "lampara2On"), lampara2Activa ? 1 : 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "lampara3On"), lampara3Activa ? 1 : 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "lampara4On"), lampara4Activa ? 1 : 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "lampara5On"), lampara5Activa ? 1 : 0);  
        glUniform1i(glGetUniformLocation(shaderProgram, "lampara6On"), lampara6Activa ? 1 : 0);  
        glUniform3fv(glGetUniformLocation(shaderProgram, "lampPos"), 1, glm::value_ptr(lampEntrada));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lampPos2"), 1, glm::value_ptr(lampHab1));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lampPos3"), 1, glm::value_ptr(lampHab2));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lampPos4"), 1, glm::value_ptr(lampHab3));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lampPos5"), 1, glm::value_ptr(lampHab4));  
        glUniform3fv(glGetUniformLocation(shaderProgram, "lampPos6"), 1, glm::value_ptr(lampHab5));  

        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

        glm::vec3 flashlightTip = cameraPos + cameraFront * 0.6f + glm::vec3(0.25f, -0.20f, 0.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(flashlightTip));
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(cameraFront));
        bool finalFlashlightState = flashlightOn && flickerState;
        glUniform1i(glGetUniformLocation(shaderProgram, "flashlightOn"), finalFlashlightState ? 1 : 0);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(70.0f),
            (float)SCREEN_W / (float)SCREEN_H, 0.1f, 300.0f);

        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShaderProgram);

        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

        glUniformMatrix4fv(
            glGetUniformLocation(skyboxShaderProgram, "view"),
            1, GL_FALSE, glm::value_ptr(skyboxView)
        );
        glUniformMatrix4fv(
            glGetUniformLocation(skyboxShaderProgram, "projection"),
            1, GL_FALSE, glm::value_ptr(projection)
        );

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glUniform1i(glGetUniformLocation(skyboxShaderProgram, "skybox"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glUseProgram(shaderProgram);

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);

        glm::vec3 camRight = glm::normalize(glm::cross(cameraFront, cameraUp));
        glm::vec3 flashlightPos = cameraPos + cameraFront * 0.30f + camRight * 0.35f + cameraUp * (-0.25f);
        glm::mat4 flMat = glm::mat4(1.0f);
        flMat = glm::translate(flMat, flashlightPos);
        flMat = glm::rotate(flMat, glm::radians(-yaw - 90.0f), glm::vec3(0, 1, 0));
        flMat = glm::rotate(flMat, glm::radians(pitch), glm::vec3(1, 0, 0));
        flMat = glm::rotate(flMat, glm::radians(90.0f), glm::vec3(0, 0, 1));
        flMat = glm::rotate(flMat, glm::radians(-45.0f), glm::vec3(0, 1, 0));
        flMat = glm::scale(flMat, glm::vec3(0.05f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(flMat));
        flashlightModel.Draw(shaderProgram);

        {
            glm::mat4 m = glm::scale(glm::mat4(1.0f), glm::vec3(MAZE_SCALE));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(0);
            laberintoModel.Draw(shaderProgram);
            glBindVertexArray(VAO);
        }

        bool lookingPowerbox1 = false;
        if (!powerbox1Activado && glm::length(cameraPos - powerbox1Pos) < 6.0f)
            lookingPowerbox1 = isLookingAtObject(cameraPos, cameraFront, powerbox1Pos, 4.0f);

        bool lookingPowerbox2 = false;
        if (!powerbox2Activado && glm::length(cameraPos - powerbox2Pos) < 6.0f)
            lookingPowerbox2 = isLookingAtObject(cameraPos, cameraFront, powerbox2Pos, 4.0f);

        if ((lookingPowerbox1 || lookingPowerbox2) &&
            glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressedPowerbox)
        {
            ePressedPowerbox = true; minijuegoActivo = true;
            float d1 = glm::length(cameraPos - powerbox1Pos);
            float d2 = glm::length(cameraPos - powerbox2Pos);
            if (lookingPowerbox1 && lookingPowerbox2) powerboxActual = (d1 < d2) ? 1 : 2;
            else if (lookingPowerbox1)               powerboxActual = 1;
            else                                     powerboxActual = 2;
        }

        if (minijuegoActivo)
        {
            static bool rSoltado = false, gSoltado = false, bSoltado = false;
            if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) rSoltado = true;
            if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) gSoltado = true;
            if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) bSoltado = true;

            if (cableActual == 0 && glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && rSoltado) { cableActual = 1; rSoltado = false; }
            else if (cableActual == 1 && glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && gSoltado) { cableActual = 2; gSoltado = false; }
            else if (cableActual == 2 && glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && bSoltado) {
                minijuegoActivo = false; cableActual = 0; ePressedPowerbox = false;
                rSoltado = gSoltado = bSoltado = false;

                // ✅ Activar lámparas según el powerbox
                if (powerboxActual == 1) {
                    powerbox1Activado = true;
                    lampara3Activa = true;
                    lampara1Activa = true;
                    lampara5Activa = true;  
                }
                else if (powerboxActual == 2) {
                    powerbox2Activado = true;
                    lampara4Activa = true;
                    lampara2Activa = true;
                    lampara6Activa = true;  
                }
            }
            else if (cableActual > 0 &&
                ((glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && rSoltado) ||
                    (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && gSoltado) ||
                    (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && bSoltado)))
            {
                cableActual = 0; rSoltado = gSoltado = bSoltado = false;
            }

            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                minijuegoActivo = false; cableActual = 0; ePressedPowerbox = false;
                rSoltado = gSoltado = bSoltado = false;
            }
        }

        bool lookingLlave = false;
        if (!llaveRecogida)
            lookingLlave = isLookingAtObject(cameraPos, cameraFront, llavePosicion, 3.0f);

        if (lookingLlave && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed) {
            ePressed = true; llaveRecogida = true; tieneLlave = true;
            ma_engine_play_sound(&audioEngine, "Sonidos/key.wav", NULL);
        }

        // =====================================
        // BATERIAS
        // =====================================
        for (auto& bat : baterias) {
            if (!bat.recogida &&
                isLookingAtObject(cameraPos, cameraFront, bat.pos, 3.0f) &&
                glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed)
            {
                ePressed = true;
                bat.recogida = true;
                battery = 100.0f;
                flashlightOn = true;
                ma_engine_play_sound(&audioEngine, "Sonidos/key.wav", NULL);
            }
        }

        // =====================================
        // DOCUMENTOS
        // =====================================
        for (auto& doc : documentos) {
            if (!doc.recogido &&
                isLookingAtObject(cameraPos, cameraFront, doc.pos, 3.0f) &&
                glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed)
            {
                ePressed = true;
                doc.recogido = true;
                textoDocumento = doc.mensaje;
                mostrandoDocumento = true;
                timerDocumento = 6.0f;
                ma_engine_play_sound(&audioEngine, "Sonidos/key.wav", NULL);
            }
        }

        // =====================================
        // SISTEMA DE TAREAS
        // =====================================
        if (!tareaIniciada) {
            tareaIniciada = true;
            notificacionTarea = "Find and activate the powerboxes";
            timerNotificacion = DURACION_NOTIFICACION;
        }

        if (powerbox1Activado && !pb1Notificado && !powerbox2Activado) {
            pb1Notificado = true;
            notificacionTarea = "Good job! Find the second powerbox";
            timerNotificacion = DURACION_NOTIFICACION;
        }

        if (powerbox2Activado && !pb2Notificado) {
            pb2Notificado = true;
            notificacionTarea = "Both powerboxes active! Reach Room 3";
            timerNotificacion = DURACION_NOTIFICACION;
        }

        if (timerNotificacion > 0.0f) {
            timerNotificacion -= deltaTime;
            if (timerNotificacion <= 0.0f) { timerNotificacion = 0.0f; notificacionTarea = ""; }
        }

        if (timerDocumento > 0.0f) {
            timerDocumento -= deltaTime;
            if (timerDocumento <= 0.0f) { timerDocumento = 0.0f; mostrandoDocumento = false; }
        }

        // ── PUERTAS — habilitadas ──
        // ✅ USAR doorInteractPos para la detección
        bool lookingDoor = isLookingAtObject(cameraPos, cameraFront, doorInteractPos, 4.0f);
        bool lookingDoor2 = isLookingAtObject(cameraPos, cameraFront, door2InteractPos, 4.0f);
        bool lookingDoor3 = isLookingAtObject(cameraPos, cameraFront, door3InteractPos, 4.0f);
        bool lookingDoor4 = isLookingAtObject(cameraPos, cameraFront, door4InteractPos, 4.0f);
        bool lookingDoor5 = isLookingAtObject(cameraPos, cameraFront, door5InteractPos, 4.0f);
        bool lookingDoor6 = isLookingAtObject(cameraPos, cameraFront, door6InteractPos, 4.0f);

        if (lookingDoor && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed) {
            ePressed = true;
            if (tieneLlave || doorOpen) {
                // Si tiene llave puede abrir, si ya esta abierta puede cerrar
                doorOpen = !doorOpen;
                if (doorOpen) tieneLlave = false; // consume la llave al abrir
                ma_engine_play_sound(&audioEngine,
                    doorOpen ? "Sonidos/door_open.wav" : "Sonidos/door_close.wav", NULL);
            }
        }

        // ✅ ELIMINADO el check de distancia redundante que impedía abrir la puerta 2
        if (lookingDoor2 && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed2) {
            ePressed2 = true;
            door2Open = !door2Open;
            ma_engine_play_sound(&audioEngine,
                door2Open ? "Sonidos/door_open.wav" : "Sonidos/door_close.wav", NULL);
        }
        // ✅ ELIMINADO el check de distancia redundante que impedía abrir la puerta 3
        if (lookingDoor3 && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed3) {
            ePressed3 = true;
            door3Open = !door3Open;
            ma_engine_play_sound(&audioEngine,
                door3Open ? "Sonidos/door_open.wav" : "Sonidos/door_close.wav", NULL);
        }
        if (lookingDoor4 && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed4) {
            ePressed4 = true;
            door4Open = !door4Open;
            ma_engine_play_sound(&audioEngine,
                door4Open ? "Sonidos/door_open.wav" : "Sonidos/door_close.wav", NULL);
        }
        if (lookingDoor5 && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed5) {
            ePressed5 = true;
            door5Open = !door5Open;
            ma_engine_play_sound(&audioEngine,
                door5Open ? "Sonidos/door_open.wav" : "Sonidos/door_close.wav", NULL);
        }
        if (lookingDoor6 && glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ePressed6) {
            ePressed6 = true;
            door6Open = !door6Open;
            ma_engine_play_sound(&audioEngine,
                door6Open ? "Sonidos/door_open.wav" : "Sonidos/door_close.wav", NULL);
        }


        // ✅ Se limpian las banderas de las 6 puertas al soltar la tecla E
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
            ePressed = false;  ePressed2 = false; ePressed3 = false;
            ePressed4 = false; ePressed5 = false; ePressed6 = false;
        }

        // ── Animación de apertura y cierre diferenciado ──
        // Velocidad normal al abrir, pero bajan en bomba al cerrar para que se vea el portazo duro 🔥
        float velAbrir = 120.0f;
        float velCerrar = 360.0f;

        // Puertas 1, 2, 6 (ángulo inicial 0)
        if (doorOpen && doorAngle < 90.0f) doorAngle +=  velAbrir * deltaTime;
        if (!doorOpen && doorAngle > 0.0f) doorAngle -=  velCerrar * deltaTime;

        if (door2Open && door2Angle < 90.0f) door2Angle +=  velAbrir * deltaTime;
        if (!door2Open && door2Angle > 0.0f) door2Angle -=  velCerrar * deltaTime;
        // Puertas 3, 4, 5 (ángulo inicial -90)
        if (door3Open && door3Angle < 90.0f) door3Angle +=  velAbrir * deltaTime;
        if (!door3Open && door3Angle > -90.0f) door3Angle -=  velCerrar * deltaTime;

        if (door4Open && door4Angle < 90.0f) door4Angle +=  velAbrir * deltaTime;
        if (!door4Open && door4Angle > -90.0f) door4Angle -=  velCerrar * deltaTime;

        if (door5Open && door5Angle < 90.0f) door5Angle +=  velAbrir * deltaTime;
        if (!door5Open && door5Angle > -90.0f) door5Angle -=  velCerrar * deltaTime;

        if (door6Open && door6Angle < 90.0f) door6Angle +=  velAbrir * deltaTime;
        if (!door6Open && door6Angle > 0.0f) door6Angle -=  velCerrar * deltaTime;
        // ── DIBUJAR PUERTAS ──
        {
            // Puerta 1 — Marco2 (Habitacion derecha/ el que necesita llave)
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(7.5f, 12.6f, 21.9f));
            m = glm::rotate(m, glm::radians(doorAngle), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(MAZE_SCALE));
            m = glm::translate(m, glm::vec3(0.0f, -MAZE_FLOOR_Y / MAZE_SCALE, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(0);
            doorModel.Draw(shaderProgram);
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            // Puerta 2 — Marco3 (Habitacion izquierda/ donde esta el powerbox 1)
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(-0.3f, 12.6f, -21.9f));
            m = glm::rotate(m, glm::radians(door2Angle), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(MAZE_SCALE));
            m = glm::translate(m, glm::vec3(0.0f, -MAZE_FLOOR_Y / MAZE_SCALE, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(0);
            doorModel.Draw(shaderProgram);
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            // Puerta 3 — Marco4 (Habitacion medio/donde esta el esqueleto y el powerbox 2)
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(57.6f, 12.6f, -32.1f));
            m = glm::rotate(m, glm::radians(door3Angle), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(MAZE_SCALE));
            m = glm::translate(m, glm::vec3(0.0f, -MAZE_FLOOR_Y / MAZE_SCALE, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(0);
            doorModel.Draw(shaderProgram);
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            // Puerta 4 — Marco5 (Habitacion medio/donde estan las consolas y oficinas)
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(21.6f, 12.6f, 68.0f));
            m = glm::rotate(m, glm::radians(door4Angle), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(MAZE_SCALE));
            m = glm::translate(m, glm::vec3(0.0f, -MAZE_FLOOR_Y / MAZE_SCALE, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(0);
            doorModel.Draw(shaderProgram);
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            // Puerta 5 — Marco6 (Habitacion medio/donde estan las pizarras y los cadaveres)
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(61.9f, 12.6f, 40.0f));
            m = glm::rotate(m, glm::radians(door5Angle), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(MAZE_SCALE));
            m = glm::translate(m, glm::vec3(0.0f, -MAZE_FLOOR_Y / MAZE_SCALE, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(0);
            doorModel.Draw(shaderProgram);
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            // Puerta 6 — Marco7 (Habitacion medio/donde estan los gabinetes)
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, glm::vec3(79.7f, 12.6f, -58.1f));
            m = glm::rotate(m, glm::radians(door6Angle), glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(MAZE_SCALE));
            m = glm::translate(m, glm::vec3(0.0f, -MAZE_FLOOR_Y / MAZE_SCALE, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
            glBindVertexArray(0);
            doorModel.Draw(shaderProgram);
            glBindVertexArray(VAO);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tableTexture);
        glBindVertexArray(VAO);
        glUseProgram(shaderProgram);

        // =====================================
        //  DIBUJAR MODELOS EN ESCENA
        // =====================================

        // ── Escalera — Entrada_principal ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(-27.0f, MAZE_FLOOR_Y, -2.0f));
            t = glm::rotate(t, glm::radians(90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.05f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); escaleraModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }
        // ── Computadora de la entrada ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(-25.0f, MAZE_FLOOR_Y + 1.0f, 3.0f));
            t = glm::scale(t, glm::vec3(0.2f));
            t = glm::rotate(t, glm::radians(180.0f), glm::vec3(0, 1, 0));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); computadora_1Model.Draw(shaderProgram); glBindVertexArray(VAO);
        }

        // ── Mesa 2 — debajo de la computadora de la entrada ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(-25.0f, MAZE_FLOOR_Y, 3.0f));
            t = glm::rotate(t, glm::radians(180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); mesa_2Model.Draw(shaderProgram); glBindVertexArray(VAO);
        }

        // ── Mesa — Habitacion1 ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(12.0f, MAZE_FLOOR_Y, 18.0f));
            t = glm::scale(t, glm::vec3(0.02f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            tableModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        // ── Librera — Habitacion1 ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(9.0f, MAZE_FLOOR_Y + 4.5f, 26.0f));
            t = glm::rotate(t, glm::radians(90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); libreraModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }
        // ── Llave ──
        if (!llaveRecogida) {
            float fo = sin(glfwGetTime() * 2.0f) * 0.05f;
            glm::mat4 t = glm::mat4(1);
            t = glm::translate(t, glm::vec3(llavePosicion.x, llavePosicion.y + fo, llavePosicion.z));
            t = glm::rotate(t, (float)glfwGetTime() * 1.5f, glm::vec3(0, 1, 0));
            t = glm::scale(t, glm::vec3(0.05f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0); llaveModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }
        // ── Baterías flotantes ──
        for (const auto& bat : baterias) {
            if (!bat.recogida) {
                float fo = sin(glfwGetTime() * 2.0f) * 0.05f;
                glm::mat4 t = glm::mat4(1);
                t = glm::translate(t, glm::vec3(bat.pos.x, bat.pos.y + fo, bat.pos.z));
                t = glm::rotate(t, (float)glfwGetTime() * 1.5f, glm::vec3(0, 1, 0));
                t = glm::scale(t, glm::vec3(0.1f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindVertexArray(0); bateriaModel.Draw(shaderProgram); glBindVertexArray(VAO);
            }
        }
        // ── Documentos flotantes ──
        for (const auto& doc : documentos) {
            if (!doc.recogido) {
                float fo = sin(glfwGetTime() * 2.0f) * 0.05f;
                glm::mat4 t = glm::mat4(1);
                t = glm::translate(t, glm::vec3(doc.pos.x, doc.pos.y + fo, doc.pos.z));
                t = glm::rotate(t, (float)glfwGetTime() * 1.5f, glm::vec3(0, 1, 0));
                t = glm::scale(t, glm::vec3(0.5f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindVertexArray(0); papelModel.Draw(shaderProgram); glBindVertexArray(VAO);
            }
        }
        //sillas 1
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(6.5f, MAZE_FLOOR_Y, 45.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 2
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(15.0f, MAZE_FLOOR_Y, 38.7f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 3
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(25.0f, MAZE_FLOOR_Y, 38.7f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 4
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(35.0f, MAZE_FLOOR_Y, 38.7f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 5
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(42.5f, MAZE_FLOOR_Y, -28.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 6
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(42.5f, MAZE_FLOOR_Y, -18.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 7
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(42.5f, MAZE_FLOOR_Y, -8.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 8
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(42.5f, MAZE_FLOOR_Y, 8.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 9
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(42.5f, MAZE_FLOOR_Y, 18.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 10
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(42.5f, MAZE_FLOOR_Y, 28.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 11
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(6.5f, MAZE_FLOOR_Y, 55.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 12
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(6.5f, MAZE_FLOOR_Y, 63.7f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 13
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(48.0f, MAZE_FLOOR_Y, 38.7f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //sillas 14
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(70.0f, MAZE_FLOOR_Y, 38.7f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.2f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindTexture(GL_TEXTURE_2D, tableTexture);
            sillaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        // ── Cápsula — Habitacion2 ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(-4.0f, MAZE_FLOOR_Y, -27.0f));
            t = glm::rotate(t, glm::radians(90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); capsulaModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }
        // ── Cajas — Habitacion2 ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(-11.0f, MAZE_FLOOR_Y, -17.0f));
            t = glm::scale(t, glm::vec3(40.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); cajasModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }
        // ── Especimen — Habitacion3 ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(62.0f, MAZE_FLOOR_Y + 1.0f, -31.0f));
            t = glm::rotate(t, glm::radians(90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(6.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); especimenModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }
        // ── Esqueleto ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(62.0f, MAZE_FLOOR_Y, -22.0f));
            t = glm::rotate(t, glm::radians(180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.6f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); esqueletoModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }

        // ── Mesa lab ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(17.0f, MAZE_FLOOR_Y, 25.0f));
            t = glm::rotate(t, glm::radians(45.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.4f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); Mesa_labModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }

        // ── Laboratorio set — Pasillo8 ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(54.0f, MAZE_FLOOR_Y, -21.0f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.4f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            glBindVertexArray(0); laboratorioModel.Draw(shaderProgram); glBindVertexArray(VAO);
        }
        // ── Powerbox 1 ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, powerbox1Pos);
            t = glm::rotate(t, glm::radians(90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(3.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            powerBox.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        // ── Powerbox 2 ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, powerbox2Pos);
            t = glm::rotate(t, glm::radians(180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(3.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            powerBox.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }

        // ── Lavabo ──
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(53.0f, MAZE_FLOOR_Y, -30.0f));
            t = glm::rotate(t, glm::radians(0.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.4f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            lavaboModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //--sillas laboratorio
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(53.0f, MAZE_FLOOR_Y, -33.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(70.0f, MAZE_FLOOR_Y, -33.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(73.0f, MAZE_FLOOR_Y, -22.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(73.0f, MAZE_FLOOR_Y, -7.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(73.0f, MAZE_FLOOR_Y, 5.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(73.0f, MAZE_FLOOR_Y, 20.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }

        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(10.0f, MAZE_FLOOR_Y, 75.0f));
            t = glm::rotate(t, glm::radians(180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(25.0f, MAZE_FLOOR_Y, 75.0f));
            t = glm::rotate(t, glm::radians(180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(40.0f, MAZE_FLOOR_Y, 75.0f));
            t = glm::rotate(t, glm::radians(180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            escritorioModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //gabinete
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(89.0f, MAZE_FLOOR_Y, -53.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.3f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
			gabineteModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //gabinete 2
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(86.0f, MAZE_FLOOR_Y, -53.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.3f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            gabineteModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //gabinete 3
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(83.0f, MAZE_FLOOR_Y, -53.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.3f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            gabineteModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
		//cajas amontonadas
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(82.0f, MAZE_FLOOR_Y + 0.7f, -64.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cajas2Model.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cajas amontonadas 2
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(90.0f, MAZE_FLOOR_Y + 0.7f, -64.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cajas2Model.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cajas amontonadas 3
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(116.4f, MAZE_FLOOR_Y + 0.7f, -1.8f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cajas2Model.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cajas amontonadas 4
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(120.4f, MAZE_FLOOR_Y + 0.7f, -1.8f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cajas2Model.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cajas amontonadas 5
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(124.4f, MAZE_FLOOR_Y + 0.7f, -1.8f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cajas2Model.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cajas amontonadas 6
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(116.4f, MAZE_FLOOR_Y + 0.7f, -5.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cajas2Model.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cajas amontonadas 7
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(120.4f, MAZE_FLOOR_Y + 0.7f, -5.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cajas2Model.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cajas amontonadas 8
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(124.4f, MAZE_FLOOR_Y + 0.7f, -5.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(2.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cajas2Model.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 1
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(110.0f, MAZE_FLOOR_Y + 0.2f, -10.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 2
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(115.0f, MAZE_FLOOR_Y + 0.2f, -10.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 3
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(120.0f, MAZE_FLOOR_Y + 0.2f, -10.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 4
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(125.0f, MAZE_FLOOR_Y + 0.2f, -10.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 5
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(125.0f, MAZE_FLOOR_Y + 0.2f, -0.0005f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 6
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(120.0f, MAZE_FLOOR_Y + 0.2f, -0.0005f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 7
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(115.0f, MAZE_FLOOR_Y + 0.2f, -0.0005f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //consola
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(30.0f, MAZE_FLOOR_Y , 67.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.7f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
			consolaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //consola 2
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(25.0f, MAZE_FLOOR_Y, 67.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.7f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            consolaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //consola 3
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(20.0f, MAZE_FLOOR_Y, 67.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.7f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            consolaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //consola 4
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(15.0f, MAZE_FLOOR_Y, 67.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.7f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            consolaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //oficina
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(15.0f, MAZE_FLOOR_Y, 60.0f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            oficinaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
		}
        //oficina 2
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(20.0f, MAZE_FLOOR_Y, 60.0f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            oficinaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //oficina 3
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(25.0f, MAZE_FLOOR_Y, 60.0f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            oficinaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //oficina 4
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(30.0f, MAZE_FLOOR_Y, 60.0f));
            t = glm::rotate(t, glm::radians(-90.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            oficinaModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //pizarra 
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(63.0f, MAZE_FLOOR_Y + 2.0f, 49.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
			pizarraModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //pizarra 2
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(59.0f, MAZE_FLOOR_Y + 2.0f, 49.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            pizarraModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //pizarra 3
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(54.0f, MAZE_FLOOR_Y + 2.0f, 49.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            pizarraModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 8
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(52.0f, MAZE_FLOOR_Y + 0.2f, 41.0f));
            t = glm::rotate(t, glm::radians(-270.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 9
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(52.0f, MAZE_FLOOR_Y + 0.2f, 45.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //cuerpo 10
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(64.0f, MAZE_FLOOR_Y + 0.2f, 45.0f));
            t = glm::rotate(t, glm::radians(-360.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(0.01f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            cuerpoModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //camilla
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(60.0f, MAZE_FLOOR_Y + 0.9f, 48.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            camillasModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //camilla 2
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(55.0f, MAZE_FLOOR_Y + 0.9f, 48.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            camillasModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //camilla 3
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(60.0f, MAZE_FLOOR_Y + 0.9f, 44.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            camillasModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        //camilla 4
        {
            glm::mat4 t = glm::mat4(1); t = glm::translate(t, glm::vec3(55.0f, MAZE_FLOOR_Y + 0.9f, 44.0f));
            t = glm::rotate(t, glm::radians(-180.0f), glm::vec3(0, 1, 0)); t = glm::scale(t, glm::vec3(1.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            camillasModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        // ── Lámparas de techo ──
        {
            glm::mat4 t = glm::mat4(1);
            t = glm::translate(t, glm::vec3(lampEntrada.x, MAZE_CEILING_Y - 0.2f, lampEntrada.z));
            t = glm::scale(t, glm::vec3(0.6f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            chandelierModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1);
            t = glm::translate(t, glm::vec3(lampHab1.x, MAZE_CEILING_Y - 0.2f, lampHab1.z));
            t = glm::scale(t, glm::vec3(0.6f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            chandelierModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1);
            t = glm::translate(t, glm::vec3(lampHab2.x, MAZE_CEILING_Y - 0.2f, lampHab2.z));
            t = glm::scale(t, glm::vec3(0.6f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            chandelierModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1);
            t = glm::translate(t, glm::vec3(lampHab3.x, MAZE_CEILING_Y - 0.2f, lampHab3.z));
            t = glm::scale(t, glm::vec3(0.6f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            chandelierModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1);
            t = glm::translate(t, glm::vec3(lampHab4.x, MAZE_CEILING_Y - 0.2f, lampHab4.z));
            t = glm::scale(t, glm::vec3(0.6f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            chandelierModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }
        {
            glm::mat4 t = glm::mat4(1);
            t = glm::translate(t, glm::vec3(lampHab5.x, MAZE_CEILING_Y - 0.2f, lampHab5.z));
            t = glm::scale(t, glm::vec3(0.6f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(t));
            chandelierModel.Draw(shaderProgram); glBindVertexArray(VAO); glBindTexture(GL_TEXTURE_2D, 0);
        }


        // =====================================
        // ENEMY IA
        // =====================================
        {
            enemy.Update(deltaTime, cameraPos, cameraFront, flashlightTip, finalFlashlightState);

            glUniform1i(glGetUniformLocation(shaderProgram, "animated"), 1);
            enemy.Draw(shaderProgram);
            glUniform1i(glGetUniformLocation(shaderProgram, "animated"), 0);
        }

        // =====================================
        // GAME OVER — Enemy te atrapa
        // =====================================
        if (glm::distance(cameraPos, enemy.position) < 3.0f)
        {
            std::cout << "¡GAME OVER! Distancia: " << glm::distance(cameraPos, enemy.position) << std::endl;
            g_GameState = GameState::GAME_OVER;
            ma_engine_play_sound(&audioEngine, "Sonidos/jumspcare.wav", NULL);
        }

        // =====================================
        // VICTORY — Powerboxes activos + zona final
        // =====================================
        bool inFinalZone = cameraPos.x > 51.90f && cameraPos.x < 64.86f &&
            cameraPos.z > -35.69f && cameraPos.z < -19.39f;

        if (powerbox1Activado && powerbox2Activado && inFinalZone)
        {
            g_GameState = GameState::VICTORY;
            ma_engine_play_sound(&audioEngine, "Sonidos/victory.wav", NULL);
        }

        // =====================================
        // HUD
        // =====================================
        glDisable(GL_DEPTH_TEST);

        const float HUD_SCALE = 1.8f;

        int hudX = (int)(SCREEN_W * 0.015f);

        char batteryText[64];
        sprintf_s(batteryText, "Battery: %.0f%%", battery);
        RenderizarTexto(hudX, (int)(SCREEN_H * 0.027f), batteryText,
            (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));

        // Contador de documentos recogidos
        {
            int docsRecogidos = 0;
            for (const auto& d : documentos)
                if (d.recogido) docsRecogidos++;
            char docsText[32];
            sprintf_s(docsText, "Documents: %d/3", docsRecogidos);
            RenderizarTexto(hudX, (int)(SCREEN_H * 0.055f), docsText,
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
        }

        // Stamina (solo visible si no esta llena, para no saturar el HUD)
        if (stamina < 99.5f) {
            char staminaText[64];
            sprintf_s(staminaText, "Stamina: %.0f%%", stamina);
            RenderizarTexto(hudX, (int)(SCREEN_H * 0.083f), staminaText,
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
        }

        if (tieneLlave)
            RenderizarTexto(hudX, (int)(SCREEN_H * 0.145f), "Key: YES",
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));

        // ── LISTA DE TAREAS (debajo de batería, desaparecen al completarse) ──
        {
            int tareaY = (int)(SCREEN_H * 0.200f / HUD_SCALE);
            int tareaLineH = (int)(SCREEN_H * 0.055f / HUD_SCALE);
            int tareaX = hudX;
            int sw = (int)(SCREEN_W / HUD_SCALE);
            int sh = (int)(SCREEN_H / HUD_SCALE);
            int lineIdx = 0;

            // Tarea 1: activar powerbox 1 — desaparece cuando se completa
            if (!powerbox1Activado) {
                RenderizarTexto(tareaX, tareaY + lineIdx * tareaLineH,
                    "[ ] Activate powerbox 1", sw, sh);
                lineIdx++;
            }

            // Tarea 2: activar powerbox 2 — solo aparece cuando la 1 ya esta activa
            if (powerbox1Activado && !powerbox2Activado) {
                RenderizarTexto(tareaX, tareaY + lineIdx * tareaLineH,
                    "[ ] Activate powerbox 2", sw, sh);
                lineIdx++;
            }

            // Tarea 3: llegar a habitacion 3 — solo aparece cuando ambos powerbox activos
            bool inFinalZoneCheck = cameraPos.x > 51.90f && cameraPos.x < 64.86f &&
                cameraPos.z > -35.69f && cameraPos.z < -19.39f;
            if (powerbox1Activado && powerbox2Activado && !inFinalZoneCheck) {
                RenderizarTexto(tareaX, tareaY + lineIdx * tareaLineH,
                    "[ ] Reach Room 3", sw, sh);
                lineIdx++;
            }
        }

        int hudCX = (int)(SCREEN_W * 0.39f / HUD_SCALE);
        int hudCY = (int)(SCREEN_H * 0.90f / HUD_SCALE);

        if (lookingLlave && !llaveRecogida)
            RenderizarTexto(hudCX, hudCY, "[E] Pick up key",
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));

        // Mensaje pickup baterias
        for (const auto& bat : baterias) {
            if (!bat.recogida && isLookingAtObject(cameraPos, cameraFront, bat.pos, 3.0f))
                RenderizarTexto(hudCX, hudCY, "[E] Pick up battery",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
        }

        // Mensaje pickup documentos
        for (const auto& doc : documentos) {
            if (!doc.recogido && isLookingAtObject(cameraPos, cameraFront, doc.pos, 3.0f))
                RenderizarTexto(hudCX, hudCY, "[E] Read document",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
        }

        // ── Notificacion de tarea (centro inferior) ──
        if (timerNotificacion > 0.0f && !notificacionTarea.empty()) {
            int notifX = (int)(SCREEN_W * 0.25f / HUD_SCALE);
            int notifY = (int)(SCREEN_H * 0.82f / HUD_SCALE);
            RenderizarTexto(notifX, notifY, notificacionTarea.c_str(),
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
        }

        // ── Texto de documento (centro pantalla) ──
        if (mostrandoDocumento && textoDocumento) {
            int docX = (int)(SCREEN_W * 0.25f / HUD_SCALE);
            int docY = (int)(SCREEN_H * 0.38f / HUD_SCALE);
            int lineH = (int)(SCREEN_H * 0.045f / HUD_SCALE);
            std::string txt(textoDocumento);
            int lineIdx = 0;
            size_t pos = 0;
            while (pos < txt.size()) {
                size_t nl = txt.find('\n', pos);
                std::string line = (nl == std::string::npos) ? txt.substr(pos) : txt.substr(pos, nl - pos);
                RenderizarTexto(docX, docY + lineIdx * lineH, line.c_str(),
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
                lineIdx++;
                if (nl == std::string::npos) break;
                pos = nl + 1;
            }
        }

        if (lookingPowerbox1 && !powerbox1Activado && !minijuegoActivo)
            RenderizarTexto(hudCX, hudCY, "[E] Interact with powerbox",
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
        if (lookingPowerbox2 && !powerbox2Activado && !minijuegoActivo)
            RenderizarTexto(hudCX, hudCY, "[E] Interact with powerbox",
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));

        // Puerta 1 (requiere llave)
        if (lookingDoor) {
            if (!doorOpen && tieneLlave) {
                RenderizarTexto(hudCX, hudCY, "[E] Open door (key required)",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
            else if(doorOpen) {
                RenderizarTexto(hudCX, hudCY, "[E] Close door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
        }

        // Puerta 2 (sin llave)
        if (lookingDoor2) {
            if (door2Open) {
                RenderizarTexto(hudCX, hudCY, "[E] Close door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
            else {
                RenderizarTexto(hudCX, hudCY, "[E] Open door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
        }

        // Puerta 3 (sin llave)
        if (lookingDoor3) {
            if (door3Open) {
                RenderizarTexto(hudCX, hudCY, "[E] Close door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
            else {
                RenderizarTexto(hudCX, hudCY, "[E] Open door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
        }
        // Puerta 4 (sin llave)
        if (lookingDoor4) {
            if (door4Open) {
                RenderizarTexto(hudCX, hudCY, "[E] Close door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
            else {
                RenderizarTexto(hudCX, hudCY, "[E] Open door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
        }
        // Puerta 5 (sin llave)
        if (lookingDoor5) {
            if (door5Open) {
                RenderizarTexto(hudCX, hudCY, "[E] Close door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
            else {
                RenderizarTexto(hudCX, hudCY, "[E] Open door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
        }
        // Puerta 6 (sin llave)
        if (lookingDoor6) {
            if (door6Open) {
                RenderizarTexto(hudCX, hudCY, "[E] Close door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
            else {
                RenderizarTexto(hudCX, hudCY, "[E] Open door",
                    (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
            }
        }

        if (powerbox1Activado || lampara3Activa || lampara1Activa || lampara5Activa)
            RenderizarTexto(hudX, (int)(SCREEN_H * 0.140f), "Room 1 lamp: ON and Room 2 lamp: ON and Cabinet Hall lamp: ON",
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));
        if (powerbox2Activado || lampara4Activa || lampara2Activa || lampara6Activa)
            RenderizarTexto(hudX, (int)(SCREEN_H * 0.182f), "Room 3 lamp: ON and Room 4 lamp: ON and Final Room lamp: ON",
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));

        if (cursorFree && !pauseMenuOpen)
            RenderizarTexto(hudX, (int)(SCREEN_H * 0.194f), "[ALT] Cursor free - press ALT to lock",
                (int)(SCREEN_W / HUD_SCALE), (int)(SCREEN_H / HUD_SCALE));

        if (minijuegoActivo)
        {
            int mx = (int)(SCREEN_W * 0.31f / HUD_SCALE);
            int my2 = (int)(SCREEN_H * 0.27f / HUD_SCALE);
            int ms = (int)(SCREEN_H * 0.055f / HUD_SCALE);
            int sw2 = (int)(SCREEN_W / HUD_SCALE);
            int sh2 = (int)(SCREEN_H / HUD_SCALE);
            RenderizarTexto(mx, my2, "=== RECONNECT CABLES ===", sw2, sh2);
            RenderizarTexto(mx, my2 + ms, "Connect the cables in order", sw2, sh2);
            RenderizarTexto(mx, my2 + ms * 2 + 20, cableActual == 0 ? ">> [R] Red cable   - DISCONNECTED" : "   [R] Red cable   - CONNECTED", sw2, sh2);
            if (cableActual == 1)   RenderizarTexto(mx, my2 + ms * 3 + 20, ">> [G] Green cable - DISCONNECTED", sw2, sh2);
            else if (cableActual > 1)RenderizarTexto(mx, my2 + ms * 3 + 20, "   [G] Green cable - CONNECTED", sw2, sh2);
            else                  RenderizarTexto(mx, my2 + ms * 3 + 20, "   [G] Green cable - DISCONNECTED", sw2, sh2);
            RenderizarTexto(mx, my2 + ms * 4 + 20, cableActual == 2 ? ">> [B] Blue cable  - DISCONNECTED" : "   [B] Blue cable  - DISCONNECTED", sw2, sh2);
            RenderizarTexto(mx, my2 + ms * 5 + 30, "[P] Cancel", sw2, sh2);
        }

        glEnable(GL_DEPTH_TEST);

        if (pauseMenuOpen)
        {
            GameState pauseResult = g_Menu->HandlePauseInput(window, mouseX, mouseY);
            g_Menu->RenderPause(mouseX, mouseY);

            if (pauseResult == GameState::PLAYING) {
                ma_engine_play_sound(&audioEngine, "Sonidos/menu_select.wav", NULL);
                pauseMenuOpen = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                cursorFree = false; firstMouse = true;
            }
            else if (pauseResult == GameState::MENU) {
                ma_engine_play_sound(&audioEngine, "Sonidos/menu_select.wav", NULL);
                pauseMenuOpen = false;
                g_GameState = GameState::MENU;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                cursorFree = false;
            }
            // Si es PAUSED, no hace nada - el menú sigue abierto
        }

        // ── GAME OVER / VICTORY ──
        if (g_GameState == GameState::GAME_OVER || g_GameState == GameState::VICTORY)
        {
            bool isVictory = (g_GameState == GameState::VICTORY);

            glClearColor(0, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            GameState result = g_Menu->HandleEndScreenInput(window, mouseX, mouseY, isVictory);
            g_Menu->RenderEndScreen(mouseX, mouseY, isVictory);

            if (result == GameState::PLAYING) {
                ResetGame();
                g_GameState = GameState::PLAYING;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                cursorFree = false; firstMouse = true;
                lastFrame = (float)glfwGetTime();
            }
            else if (result == GameState::MENU) {
                g_GameState = GameState::MENU;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                cursorFree = false;
            }

            glfwSwapBuffers(window); glfwPollEvents(); continue;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    CleanupTextRenderer();
    delete g_Menu; g_Menu = nullptr;
    glfwTerminate();
    return 0;
}