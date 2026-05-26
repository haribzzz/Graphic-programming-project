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
    vec3 objectColor = vec3(0.7);

    vec3 ambient = vec3(0.03);

    vec3 norm = normalize(Normal);

    vec3 lightDirection =
        normalize(lightPos - FragPos);

    float theta =
        dot(
            lightDirection,
            normalize(-lightDir)
        );

    float intensity =
        smoothstep(0.90, 0.95, theta);

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

glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 0.0f);

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
// COLISIONES
// =====================================

float roomLimit = 4.5f;

float floorHeight = 0.0f;

float ceilingHeight = 4.0f;

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
    float speed = 3.0f * deltaTime;

    glm::vec3 previousPos = cameraPos;

    // =====================================
    // MOVIMIENTO FPS
    // =====================================

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

    // =====================================
    // GRAVEDAD
    // =====================================

    velocityY += gravity * deltaTime;

    cameraPos.y += velocityY * deltaTime;

    // =====================================
    // COLISION PISO
    // =====================================

    float playerHeight = 1.0f;

    if (cameraPos.y <= floorHeight + playerHeight)
    {
        cameraPos.y = floorHeight + playerHeight;

        velocityY = 0.0f;

        onGround = true;
    }

    // =====================================
    // COLISION TECHO
    // =====================================

    if (cameraPos.y > ceilingHeight)
    {
        cameraPos.y = ceilingHeight;

        velocityY = 0.0f;
    }

    // =====================================
    // COLISIONES PAREDES
    // =====================================
    cameraPos.x =
        glm::clamp(
            cameraPos.x,
            -roomLimit,
            roomLimit
        );

    cameraPos.z =
        glm::clamp(
            cameraPos.z,
            -roomLimit,
            roomLimit
        );

    // =====================================
    // ESC
    // =====================================

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

    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "Suspense Laboratory",
        NULL,
        NULL
    );

    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(
        window,
        GLFW_CURSOR,
        GLFW_CURSOR_DISABLED
    );

    gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress
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
    // VERTICES
    // =====================================

    float vertices[] =
    {
        // posiciones          // normales

        -0.5f,-0.5f,-0.5f, 0,0,-1,
         0.5f,-0.5f,-0.5f, 0,0,-1,
         0.5f, 0.5f,-0.5f, 0,0,-1,

         0.5f, 0.5f,-0.5f, 0,0,-1,
        -0.5f, 0.5f,-0.5f, 0,0,-1,
        -0.5f,-0.5f,-0.5f, 0,0,-1,

        -0.5f,-0.5f, 0.5f, 0,0,1,
         0.5f,-0.5f, 0.5f, 0,0,1,
         0.5f, 0.5f, 0.5f, 0,0,1,

         0.5f, 0.5f, 0.5f, 0,0,1,
        -0.5f, 0.5f, 0.5f, 0,0,1,
        -0.5f,-0.5f, 0.5f, 0,0,1,

        -0.5f, 0.5f, 0.5f,-1,0,0,
        -0.5f, 0.5f,-0.5f,-1,0,0,
        -0.5f,-0.5f,-0.5f,-1,0,0,

        -0.5f,-0.5f,-0.5f,-1,0,0,
        -0.5f,-0.5f, 0.5f,-1,0,0,
        -0.5f, 0.5f, 0.5f,-1,0,0,

         0.5f, 0.5f, 0.5f,1,0,0,
         0.5f, 0.5f,-0.5f,1,0,0,
         0.5f,-0.5f,-0.5f,1,0,0,

         0.5f,-0.5f,-0.5f,1,0,0,
         0.5f,-0.5f, 0.5f,1,0,0,
         0.5f, 0.5f, 0.5f,1,0,0,

        -0.5f,-0.5f,-0.5f,0,-1,0,
         0.5f,-0.5f,-0.5f,0,-1,0,
         0.5f,-0.5f, 0.5f,0,-1,0,

         0.5f,-0.5f, 0.5f,0,-1,0,
        -0.5f,-0.5f, 0.5f,0,-1,0,
        -0.5f,-0.5f,-0.5f,0,-1,0,

        -0.5f, 0.5f,-0.5f,0,1,0,
         0.5f, 0.5f,-0.5f,0,1,0,
         0.5f, 0.5f, 0.5f,0,1,0,

         0.5f, 0.5f, 0.5f,0,1,0,
        -0.5f, 0.5f, 0.5f,0,1,0,
        -0.5f, 0.5f,-0.5f,0,1,0
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

        glClearColor(0.01f, 0.01f, 0.01f, 1.0f);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(shaderProgram);

        // =====================================
        // LINTERNA
        // =====================================

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

        // =====================================
        // VIEW MATRIX
        // =====================================

        glm::vec3 lookDirection;

        lookDirection.x =
            cos(glm::radians(yaw)) *
            cos(glm::radians(pitch));

        lookDirection.y =
            sin(glm::radians(pitch));

        lookDirection.z =
            sin(glm::radians(yaw)) *
            cos(glm::radians(pitch));

        lookDirection =
            glm::normalize(lookDirection);

        glm::mat4 view =
            glm::lookAt(
                cameraPos,
                cameraPos + lookDirection,
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

        // =====================================
        // PISO
        // =====================================

        glm::mat4 floorModel = glm::mat4(1.0f);

        floorModel =
            glm::translate(
                floorModel,
                glm::vec3(0.0f, -1.0f, 0.0f)
            );

        floorModel =
            glm::scale(
                floorModel,
                glm::vec3(10.0f, 0.1f, 10.0f)
            );

        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            glm::value_ptr(floorModel)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // =====================================
        // PARED TRASERA
        // =====================================

        glm::mat4 backWall = glm::mat4(1.0f);

        backWall =
            glm::translate(
                backWall,
                glm::vec3(0.0f, 2.0f, -5.0f)
            );

        backWall =
            glm::scale(
                backWall,
                glm::vec3(10.0f, 5.0f, 0.1f)
            );

        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            glm::value_ptr(backWall)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // =====================================
        // PARED IZQUIERDA
        // =====================================

        glm::mat4 leftWall = glm::mat4(1.0f);

        leftWall =
            glm::translate(
                leftWall,
                glm::vec3(-5.0f, 2.0f, 0.0f)
            );

        leftWall =
            glm::scale(
                leftWall,
                glm::vec3(0.1f, 5.0f, 10.0f)
            );

        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            glm::value_ptr(leftWall)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // =====================================
        // PARED DERECHA
        // =====================================

        glm::mat4 rightWall = glm::mat4(1.0f);

        rightWall =
            glm::translate(
                rightWall,
                glm::vec3(5.0f, 2.0f, 0.0f)
            );

        rightWall =
            glm::scale(
                rightWall,
                glm::vec3(0.1f, 5.0f, 10.0f)
            );

        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            glm::value_ptr(rightWall)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // =====================================
        // TECHO
        // =====================================

        glm::mat4 ceiling = glm::mat4(1.0f);

        ceiling =
            glm::translate(
                ceiling,
                glm::vec3(0.0f, 4.5f, 0.0f)
            );

        ceiling =
            glm::scale(
                ceiling,
                glm::vec3(10.0f, 0.1f, 10.0f)
            );

        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            glm::value_ptr(ceiling)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}