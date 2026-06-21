#include "TextRenderer.h"

#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

GLuint textVAO = 0;
GLuint textVBO = 0;
GLuint textShader = 0;

const char* textVertexShader = R"(
#version 330 core

layout(location = 0) in vec2 aPos;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* textFragmentShader = R"(
#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

bool InitTextRenderer()
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &textVertexShader, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &textFragmentShader, nullptr);
    glCompileShader(fs);

    textShader = glCreateProgram();
    glAttachShader(textShader, vs);
    glAttachShader(textShader, fs);
    glLinkProgram(textShader);

    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    glBindVertexArray(textVAO);

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        1000000,
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(float) * 2,
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    return true;
}

// scale: 1.0 = tamaño original stb (~9px de alto).
// Para HUD en 1080p usa scale ~2.0–2.5; para 720p usa ~1.5–2.0.
void RenderizarTexto(
    float x,
    float y,
    const char* text,
    int screenWidth,
    int screenHeight,
    float scale        // NUEVO: factor de escala del texto
)
{
    char stbBuffer[99999];

    int quads =
        stb_easy_font_print(
            0, 0,           // generamos en origen y desplazamos con scale
            (char*)text,
            nullptr,
            stbBuffer,
            sizeof(stbBuffer)
        );

    struct Vertex2D { float x; float y; };

    std::vector<Vertex2D> triangles;

    float* v = (float*)stbBuffer;

    for (int i = 0; i < quads; i++)
    {
        // stb_easy_font produce quads de 4 vértices, stride 16 bytes (4 floats)
        Vertex2D p0 = { x + v[i * 16 + 0] * scale,  y + v[i * 16 + 1] * scale };
        Vertex2D p1 = { x + v[i * 16 + 4] * scale,  y + v[i * 16 + 5] * scale };
        Vertex2D p2 = { x + v[i * 16 + 8] * scale,  y + v[i * 16 + 9] * scale };
        Vertex2D p3 = { x + v[i * 16 + 12] * scale, y + v[i * 16 + 13] * scale };

        triangles.push_back(p0);
        triangles.push_back(p1);
        triangles.push_back(p2);

        triangles.push_back(p0);
        triangles.push_back(p2);
        triangles.push_back(p3);
    }

    glm::mat4 projection(1.0f);

    projection[0][0] = 2.0f / screenWidth;
    projection[1][1] = -2.0f / screenHeight;
    projection[3][0] = -1.0f;
    projection[3][1] = 1.0f;

    glUseProgram(textShader);

    glUniformMatrix4fv(
        glGetUniformLocation(textShader, "projection"),
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    glBindVertexArray(textVAO);

    glBindBuffer(GL_ARRAY_BUFFER, textVBO);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        triangles.size() * sizeof(Vertex2D),
        triangles.data()
    );

    glDisable(GL_DEPTH_TEST);

    glDrawArrays(
        GL_TRIANGLES,
        0,
        (GLsizei)triangles.size()
    );

    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(0);
}

void CleanupTextRenderer()
{
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(textShader);
}