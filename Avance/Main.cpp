#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

// =====================================
// SHADERS
// =====================================

const char* vertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));

    Normal =
        mat3(transpose(inverse(model))) * aNormal;

    gl_Position =
        projection *
        view *
        vec4(FragPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 lightDir;

void main()
{
    vec3 objectColor = vec3(0.75);

    vec3 ambient = vec3(0.02);

    vec3 norm = normalize(Normal);

    vec3 lightDirection =
        normalize(lightPos - FragPos);

    float theta =
        dot(
            lightDirection,
            normalize(-lightDir)
        );

    float intensity =
        smoothstep(0.90, 0.97, theta);

    float diff =
        max(dot(norm, lightDirection), 0.0);

    vec3 diffuse =
        diff *
        intensity *
        vec3(1.0);

    vec3 result =
        (ambient + diffuse) * objectColor;

    FragColor = vec4(result, 1.0);
}
)";

// =====================================
// CAMARA
// =====================================

glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 2.0f);

glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// =====================================
// MOUSE
// =====================================

float yaw = -90.0f;
float pitch = 0.0f;

float lastX = 640.0f;
float lastY = 360.0f;

bool firstMouse = true;

// =====================================
// SALTO
// =====================================

float velocityY = 0.0f;

bool onGround = true;

float gravity = -9.8f;

float jumpForce = 5.0f;

// =====================================
// MOUSE LOOK
// =====================================

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;

        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;

    lastX = (float)xpos;
    lastY = (float)ypos;

    float sensitivity = 0.1f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;

    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;

    direction.x =
        cos(glm::radians(yaw)) *
        cos(glm::radians(pitch));

    direction.y =
        sin(glm::radians(pitch));

    direction.z =
        sin(glm::radians(yaw)) *
        cos(glm::radians(pitch));

    cameraFront =
        glm::normalize(direction);
}

// =====================================
// INPUT
// =====================================

void processInput(GLFWwindow* window)
{
    float speed = 3.5f * deltaTime;

    glm::vec3 previousPos = cameraPos;

    glm::vec3 forward;

    forward.x =
        cos(glm::radians(yaw));

    forward.y = 0.0f;

    forward.z =
        sin(glm::radians(yaw));

    forward =
        glm::normalize(forward);

    glm::vec3 right =
        glm::normalize(
            glm::cross(
                forward,
                glm::vec3(0.0f, 1.0f, 0.0f)
            )
        );

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += forward * speed;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= forward * speed;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= right * speed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += right * speed;

    // =====================================
    // SALTO
    // =====================================

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround)
    {
        velocityY = jumpForce;

        onGround = false;
    }

    velocityY += gravity * deltaTime;

    cameraPos.y += velocityY * deltaTime;

    if (cameraPos.y <= 1.0f)
    {
        cameraPos.y = 1.0f;

        velocityY = 0.0f;

        onGround = true;
    }

    if (cameraPos.y > 4.0f)
    {
        cameraPos.y = 4.0f;

        velocityY = 0.0f;
    }

    // =====================================
    // COLISIONES
    // =====================================

    bool inRoom1 =
        (
            cameraPos.x > -5.0f &&
            cameraPos.x < 5.0f &&
            cameraPos.z < 5.0f &&
            cameraPos.z > -5.0f
            );

    bool inHallway =
        (
            cameraPos.x > -1.5f &&
            cameraPos.x < 1.5f &&
            cameraPos.z < -5.0f &&
            cameraPos.z > -13.0f
            );

    bool inRoom2 =
        (
            cameraPos.x > -5.0f &&
            cameraPos.x < 5.0f &&
            cameraPos.z < -13.0f &&
            cameraPos.z > -23.0f
            );

    if (!inRoom1 && !inHallway && !inRoom2)
    {
        cameraPos = previousPos;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// =====================================
// MAIN
// =====================================

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    GLFWwindow* window =
        glfwCreateWindow(
            1280,
            720,
            "DarkZone",
            NULL,
            NULL
        );

    glfwMakeContextCurrent(window);

    gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress
    );

    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(
        window,
        GLFW_CURSOR,
        GLFW_CURSOR_DISABLED
    );

    glEnable(GL_DEPTH_TEST);

    // =====================================
    // SHADERS
    // =====================================

    GLuint vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        NULL
    );

    glCompileShader(vertexShader);

    GLuint fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        NULL
    );

    glCompileShader(fragmentShader);

    GLuint shaderProgram =
        glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);

    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    // =====================================
    // VERTICES CUBO
    // =====================================

    float vertices[] =
    {
        -0.5f,-0.5f,-0.5f,0,0,-1,
         0.5f,-0.5f,-0.5f,0,0,-1,
         0.5f, 0.5f,-0.5f,0,0,-1,
         0.5f, 0.5f,-0.5f,0,0,-1,
        -0.5f, 0.5f,-0.5f,0,0,-1,
        -0.5f,-0.5f,-0.5f,0,0,-1,

        -0.5f,-0.5f,0.5f,0,0,1,
         0.5f,-0.5f,0.5f,0,0,1,
         0.5f,0.5f,0.5f,0,0,1,
         0.5f,0.5f,0.5f,0,0,1,
        -0.5f,0.5f,0.5f,0,0,1,
        -0.5f,-0.5f,0.5f,0,0,1,

        -0.5f,0.5f,0.5f,-1,0,0,
        -0.5f,0.5f,-0.5f,-1,0,0,
        -0.5f,-0.5f,-0.5f,-1,0,0,
        -0.5f,-0.5f,-0.5f,-1,0,0,
        -0.5f,-0.5f,0.5f,-1,0,0,
        -0.5f,0.5f,0.5f,-1,0,0,

         0.5f,0.5f,0.5f,1,0,0,
         0.5f,0.5f,-0.5f,1,0,0,
         0.5f,-0.5f,-0.5f,1,0,0,
         0.5f,-0.5f,-0.5f,1,0,0,
         0.5f,-0.5f,0.5f,1,0,0,
         0.5f,0.5f,0.5f,1,0,0,

        -0.5f,-0.5f,-0.5f,0,-1,0,
         0.5f,-0.5f,-0.5f,0,-1,0,
         0.5f,-0.5f,0.5f,0,-1,0,
         0.5f,-0.5f,0.5f,0,-1,0,
        -0.5f,-0.5f,0.5f,0,-1,0,
        -0.5f,-0.5f,-0.5f,0,-1,0,

        -0.5f,0.5f,-0.5f,0,1,0,
         0.5f,0.5f,-0.5f,0,1,0,
         0.5f,0.5f,0.5f,0,1,0,
         0.5f,0.5f,0.5f,0,1,0,
        -0.5f,0.5f,0.5f,0,1,0,
        -0.5f,0.5f,-0.5f,0,1,0
    };

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);

    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    // =====================================
    // LOOP
    // =====================================

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();

        deltaTime = currentFrame - lastFrame;

        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(shaderProgram);

        glUniform3f(
            glGetUniformLocation(shaderProgram, "lightPos"),
            cameraPos.x,
            cameraPos.y,
            cameraPos.z
        );

        glUniform3f(
            glGetUniformLocation(shaderProgram, "lightDir"),
            cameraFront.x,
            cameraFront.y,
            cameraFront.z
        );

        glm::mat4 view =
            glm::lookAt(
                cameraPos,
                cameraPos + cameraFront,
                cameraUp
            );

        glm::mat4 projection =
            glm::perspective(
                glm::radians(70.0f),
                1280.0f / 720.0f,
                0.1f,
                100.0f
            );

        GLuint modelLoc =
            glGetUniformLocation(shaderProgram, "model");

        GLuint viewLoc =
            glGetUniformLocation(shaderProgram, "view");

        GLuint projectionLoc =
            glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(
            viewLoc,
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );

        glUniformMatrix4fv(
            projectionLoc,
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );

        glBindVertexArray(VAO);

        auto drawCube = [&](glm::vec3 pos, glm::vec3 scale)
            {
                glm::mat4 model = glm::mat4(1.0f);

                model =
                    glm::translate(model, pos);

                model =
                    glm::scale(model, scale);

                glUniformMatrix4fv(
                    modelLoc,
                    1,
                    GL_FALSE,
                    glm::value_ptr(model)
                );

                glDrawArrays(GL_TRIANGLES, 0, 36);
            };

        // =====================================
        // HABITACION 1
        // =====================================

        // PISO
        drawCube(
            glm::vec3(0, -1, 0),
            glm::vec3(10, 0.1, 10)
        );

        // TECHO
        drawCube(
            glm::vec3(0, 4.05, 0),
            glm::vec3(10, 0.3, 10)
        );

        // PARED IZQUIERDA
        drawCube(
            glm::vec3(-5, 1.5, 0),
            glm::vec3(0.1, 5.2, 10)
        );

        // PARED DERECHA
        drawCube(
            glm::vec3(5, 1.5, 0),
            glm::vec3(0.1, 5.2, 10)
        );

        // =====================================
        // PARED DELANTERA
        // =====================================

        drawCube(
            glm::vec3(0, 1.5, 5),
            glm::vec3(10, 5.2, 0.1)
        );

        // =====================================
        // PARED TRASERA CON AGUJERO
        // =====================================

        // IZQUIERDA
        drawCube(
            glm::vec3(-3.25f, 1.5f, -5),
            glm::vec3(3.5f, 5.2f, 0.1f)
        );

        // DERECHA
        drawCube(
            glm::vec3(3.25f, 1.5f, -5),
            glm::vec3(3.5f, 5.2f, 0.1f)
        );

        // SUPERIOR
        drawCube(
            glm::vec3(0, 4, -5),
            glm::vec3(3, 1, 0.1f)
        );

        // =====================================
        // PASILLO
        // =====================================

        // PISO
        drawCube(
            glm::vec3(0, -1, -9),
            glm::vec3(3, 0.1, 8)
        );

        // TECHO
        drawCube(
            glm::vec3(0, 4.05, -9),
            glm::vec3(3, 0.3, 8)
        );

        // PARED IZQUIERDA
        drawCube(
            glm::vec3(-1.5, 1.5, -9),
            glm::vec3(0.1, 5.2, 8)
        );

        // PARED DERECHA
        drawCube(
            glm::vec3(1.5, 1.5, -9),
            glm::vec3(0.1, 5.2, 8)
        );

        // =====================================
        // ENTRADA HABITACION 2
        // =====================================

        // IZQUIERDA
        drawCube(
            glm::vec3(-3.25f, 1.5f, -13),
            glm::vec3(3.5f, 5.2f, 0.1f)
        );

        // DERECHA
        drawCube(
            glm::vec3(3.25f, 1.5f, -13),
            glm::vec3(3.5f, 5.2f, 0.1f)
        );

        // SUPERIOR
        drawCube(
            glm::vec3(0, 4, -13),
            glm::vec3(3, 1, 0.1f)
        );

        // =====================================
        // HABITACION 2
        // =====================================

        // PISO
        drawCube(
            glm::vec3(0, -1, -18),
            glm::vec3(10, 0.1, 10)
        );

        // TECHO
        drawCube(
            glm::vec3(0, 4.05, -18),
            glm::vec3(10, 0.3, 10)
        );

        // PARED IZQUIERDA
        drawCube(
            glm::vec3(-5, 1.5, -18),
            glm::vec3(0.1, 5.2, 10)
        );

        // PARED DERECHA
        drawCube(
            glm::vec3(5, 1.5, -18),
            glm::vec3(0.1, 5.2, 10)
        );

        // PARED FINAL
        drawCube(
            glm::vec3(0, 1.5, -23),
            glm::vec3(10, 5.2, 0.1)
        );

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}