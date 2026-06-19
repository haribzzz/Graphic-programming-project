
#include "Menu.h"
#include "stb_easy_font.h"
#include <iostream>
#include <vector>

Menu* g_Menu = nullptr;
GameState g_GameState = GameState::MENU;

// =====================================
// MENU SHADERS
// =====================================

const char* menuVertSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform vec2 resolution;
void main()
{
    vec2 ndc = (aPos / resolution) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
}
)";

const char* menuFragSrc = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main()
{
    FragColor = color;
}
)";

// =====================================
// CONSTRUCTOR / DESTRUCTOR
// =====================================

Menu::Menu()
    : screenW(1280), screenH(720),
    shaderProgram(0), VAO(0), VBO(0),
    textVAO(0), textVBO(0),
    currentSubmenu(GameState::MENU),
    pauseSubmenu(GameState::MENU)  // <-- cambiado de PAUSED a MENU
{
}

Menu::~Menu()
{
    Cleanup();
}

// =====================================
// INIT
// =====================================

void Menu::Init(int w, int h)
{
    screenW = w;
    screenH = h;

    InitShaders();
    InitGeometry();

    float cx = screenW / 2.0f;
    float bw = screenW * 0.28f;
    float bh = screenH * 0.10f;
    float start = screenH * 0.44f;
    float gap = screenH * 0.13f;

    btnPlay = { cx - bw / 2, start,           bw, bh, "NEW GAME", false };
    btnOptions = { cx - bw / 2, start + gap,     bw, bh, "OPTIONS",  false };
    btnCredits = { cx - bw / 2, start + gap * 2, bw, bh, "CREDITS",  false };
    btnExit = { cx - bw / 2, start + gap * 3, bw, bh, "EXIT",     false };

    btnBack = { cx - bw / 2, start + gap * 3, bw, bh, "BACK", false };

    // End screen button (Retry) — reutiliza cx ya declarado arriba
    float endBw = 300.0f, endBh = 55.0f;
    btnRetry = { cx - endBw / 2, 400.0f, endBw, endBh, "RETRY", false };

    // ── Botones +/- pequeños ──
    float sbw = screenW * 0.022f;
    float sbh = screenH * 0.038f;
    float optY1 = screenH * 0.535f;
    float optY2 = screenH * 0.665f;

    float barW = screenW * 0.20f;
    float barX = cx - barW / 2.0f;

    btnSensUp = { barX + barW + screenW * 0.012f, optY1, sbw, sbh, "+", false };
    btnSensDown = { barX - sbw - screenW * 0.012f,  optY1, sbw, sbh, "-", false };
    btnVolUp = { barX + barW + screenW * 0.012f, optY2, sbw, sbh, "+", false };
    btnVolDown = { barX - sbw - screenW * 0.012f,  optY2, sbw, sbh, "-", false };

    RebuildPauseButtons();
}

void Menu::RebuildPauseButtons()
{
    float cx = screenW / 2.0f;
    float bw = screenW * 0.28f;
    float bh = screenH * 0.10f;
    float start = screenH * 0.42f;
    float gap = screenH * 0.13f;

    btnResume = { cx - bw / 2, start,           bw, bh, "RESUME",    false };
    btnPauseOptions = { cx - bw / 2, start + gap,     bw, bh, "OPTIONS",   false };
    btnMainMenu = { cx - bw / 2, start + gap * 2, bw, bh, "MAIN MENU", false };
}

// =====================================
// SHADERS / GEOMETRY
// =====================================

void Menu::InitShaders()
{
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &menuVertSrc, NULL);
    glCompileShader(vert);

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &menuFragSrc, NULL);
    glCompileShader(frag);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vert);
    glAttachShader(shaderProgram, frag);
    glLinkProgram(shaderProgram);

    glDeleteShader(vert);
    glDeleteShader(frag);
}

void Menu::InitGeometry()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, 99999 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

// =====================================
// DRAW HELPERS
// =====================================

void Menu::DrawRect(float x, float y, float w, float h,
    float r, float g, float b, float a)
{
    float verts[] = {
        x,     y,
        x + w, y,
        x + w, y + h,
        x,     y,
        x + w, y + h,
        x,     y + h
    };

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glUniform2f(glGetUniformLocation(shaderProgram, "resolution"), (float)screenW, (float)screenH);
    glUniform4f(glGetUniformLocation(shaderProgram, "color"), r, g, b, a);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Menu::DrawText2D(const std::string& text,
    float x, float y, float scale,
    float r, float g, float b)
{
    static char buf[99999];
    int numQuads = stb_easy_font_print(0, 0, (char*)text.c_str(), nullptr, buf, sizeof(buf));

    std::vector<float> verts;
    float* data = (float*)buf;

    for (int i = 0; i < numQuads * 4; i++)
    {
        verts.push_back(x + data[i * 4 + 0] * scale);
        verts.push_back(y + data[i * 4 + 1] * scale);
        verts.push_back(0.0f);
        verts.push_back(0.0f);
    }

    std::vector<float> tris;
    for (int i = 0; i < numQuads; i++)
    {
        int b0 = i * 4;
        tris.push_back(verts[b0 * 4]);         tris.push_back(verts[b0 * 4 + 1]);
        tris.push_back(verts[b0 * 4 + 2]);     tris.push_back(verts[b0 * 4 + 3]);
        tris.push_back(verts[(b0 + 1) * 4]);        tris.push_back(verts[(b0 + 1) * 4 + 1]);
        tris.push_back(verts[(b0 + 1) * 4 + 2]);      tris.push_back(verts[(b0 + 1) * 4 + 3]);
        tris.push_back(verts[(b0 + 2) * 4]);        tris.push_back(verts[(b0 + 2) * 4 + 1]);
        tris.push_back(verts[(b0 + 2) * 4 + 2]);      tris.push_back(verts[(b0 + 2) * 4 + 3]);

        tris.push_back(verts[b0 * 4]);         tris.push_back(verts[b0 * 4 + 1]);
        tris.push_back(verts[b0 * 4 + 2]);     tris.push_back(verts[b0 * 4 + 3]);
        tris.push_back(verts[(b0 + 2) * 4]);        tris.push_back(verts[(b0 + 2) * 4 + 1]);
        tris.push_back(verts[(b0 + 2) * 4 + 2]);      tris.push_back(verts[(b0 + 2) * 4 + 3]);
        tris.push_back(verts[(b0 + 3) * 4]);        tris.push_back(verts[(b0 + 3) * 4 + 1]);
        tris.push_back(verts[(b0 + 3) * 4 + 2]);      tris.push_back(verts[(b0 + 3) * 4 + 3]);
    }

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, tris.size() * sizeof(float), tris.data());
    glUniform2f(glGetUniformLocation(shaderProgram, "resolution"), (float)screenW, (float)screenH);
    glUniform4f(glGetUniformLocation(shaderProgram, "color"), r, g, b, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(tris.size() / 4));
    glBindVertexArray(0);
}

void Menu::DrawBtn(MenuButton& btn)
{
    float br = btn.hovered ? 0.7f : 0.15f;

    DrawRect(btn.x - 2, btn.y - 2, btn.width + 4, btn.height + 4, 0.6f, 0.0f, 0.0f, 1.0f);
    DrawRect(btn.x, btn.y, btn.width, btn.height, br, 0.0f, 0.0f, 1.0f);

    float scale = (btn.height * 0.35f) / 9.0f;
    float charW = scale * 9.0f;
    float tx = btn.x + btn.width / 2.0f - (btn.text.size() * charW) / 2.0f;
    float ty = btn.y + btn.height / 2.0f - scale * 4.5f;
    float tr = btn.hovered ? 1.0f : 0.9f;
    DrawText2D(btn.text, tx, ty, scale, tr, 0.9f, 0.9f);
}

// =====================================
// HOVER HELPERS
// =====================================

bool Menu::IsHovered(const MenuButton& btn, double mx, double my)
{
    return mx >= btn.x && mx <= btn.x + btn.width &&
        my >= btn.y && my <= btn.y + btn.height;
}

void Menu::UpdateHovers(double mx, double my)
{
    btnPlay.hovered = IsHovered(btnPlay, mx, my);
    btnOptions.hovered = IsHovered(btnOptions, mx, my);
    btnCredits.hovered = IsHovered(btnCredits, mx, my);
    btnExit.hovered = IsHovered(btnExit, mx, my);
    btnBack.hovered = IsHovered(btnBack, mx, my);
    btnSensUp.hovered = IsHovered(btnSensUp, mx, my);
    btnSensDown.hovered = IsHovered(btnSensDown, mx, my);
    btnVolUp.hovered = IsHovered(btnVolUp, mx, my);
    btnVolDown.hovered = IsHovered(btnVolDown, mx, my);
}

void Menu::UpdatePauseHovers(double mx, double my)
{
    btnResume.hovered = IsHovered(btnResume, mx, my);
    btnPauseOptions.hovered = IsHovered(btnPauseOptions, mx, my);
    btnMainMenu.hovered = IsHovered(btnMainMenu, mx, my);
    btnBack.hovered = IsHovered(btnBack, mx, my);
    btnSensUp.hovered = IsHovered(btnSensUp, mx, my);
    btnSensDown.hovered = IsHovered(btnSensDown, mx, my);
    btnVolUp.hovered = IsHovered(btnVolUp, mx, my);
    btnVolDown.hovered = IsHovered(btnVolDown, mx, my);
}

// =====================================
// RENDER MAIN MENU
// =====================================

void Menu::Render(double mx, double my)
{
    UpdateHovers(mx, my);

    glUseProgram(shaderProgram);
    glDisable(GL_DEPTH_TEST);

    float vw = (float)screenW;
    float vh = (float)screenH;
    float cx = vw / 2.0f;

    DrawRect(0, 0, vw, vh, 0.0f, 0.0f, 0.0f, 1.0f);

    float vm = screenH * 0.167f;
    DrawRect(0, 0, vw, vm, 0, 0, 0, 0.7f);
    DrawRect(0, vh - vm, vw, vm, 0, 0, 0, 0.7f);
    DrawRect(0, 0, vm, vh, 0, 0, 0, 0.7f);
    DrawRect(vw - vm, 0, vm, vh, 0, 0, 0, 0.7f);

    if (currentSubmenu == GameState::MENU)
    {
        float titleScale = screenH * 0.015f;
        float subtitleScale = screenH * 0.004f;
        float hintScale = screenH * 0.003f;

        float titleY = screenH * 0.16f;
        float subtitleY = screenH * 0.28f;
        float lineY = screenH * 0.36f;

        DrawText2D("DarkZone",
            cx - screenW * 0.16f, titleY,
            titleScale, 0.8f, 0.0f, 0.0f);

        DrawText2D("Abandoned Laboratory",
            cx - screenW * 0.13f, subtitleY,
            subtitleScale, 0.6f, 0.6f, 0.6f);

        DrawRect(cx - screenW * 0.16f, lineY, screenW * 0.32f, 2.0f, 0.6f, 0.0f, 0.0f, 1.0f);

        DrawBtn(btnPlay);
        DrawBtn(btnOptions);
        DrawBtn(btnCredits);
        DrawBtn(btnExit);

        DrawText2D("Use the mouse to navigate",
            cx - screenW * 0.10f, vh - screenH * 0.07f,
            hintScale, 0.4f, 0.4f, 0.4f);
    }
    else if (currentSubmenu == GameState::OPTIONS)
    {
        float titleScale = screenH * 0.010f;

        DrawText2D("OPTIONS",
            cx - screenW * 0.07f, screenH * 0.16f,
            titleScale, 0.8f, 0.0f, 0.0f);

        DrawRect(cx - screenW * 0.16f, screenH * 0.27f,
            screenW * 0.32f, 2.0f, 0.6f, 0.0f, 0.0f, 1.0f);

        RenderOptionsContent();
        DrawBtn(btnBack);
    }
    else if (currentSubmenu == GameState::CREDITS)
    {
        float titleScale = screenH * 0.010f;
        float textScale = screenH * 0.004f;

        DrawText2D("CREDITS",
            cx - screenW * 0.07f, screenH * 0.16f,
            titleScale, 0.8f, 0.0f, 0.0f);

        DrawRect(cx - screenW * 0.16f, screenH * 0.27f,
            screenW * 0.32f, 2.0f, 0.6f, 0.0f, 0.0f, 1.0f);

        DrawText2D("DarkZone - Abandoned Laboratory",
            cx - screenW * 0.16f, screenH * 0.33f,
            textScale, 0.8f, 0.0f, 0.0f);
        DrawText2D("Developed by:",
            cx - screenW * 0.09f, screenH * 0.40f,
            textScale, 0.7f, 0.7f, 0.7f);
        DrawText2D("Aura Abihail Robleto Valdivia",
            cx - screenW * 0.15f, screenH * 0.46f,
            textScale, 0.9f, 0.9f, 0.9f);
        DrawText2D("Idhe Isabel Cross Ramirez",
            cx - screenW * 0.13f, screenH * 0.51f,
            textScale, 0.9f, 0.9f, 0.9f);
        DrawText2D("Harisema Milagros Ampie Amaya",
            cx - screenW * 0.15f, screenH * 0.56f,
            textScale, 0.9f, 0.9f, 0.9f);
        DrawText2D("Edith Maria Rayo Rocha",
            cx - screenW * 0.11f, screenH * 0.61f,
            textScale, 0.9f, 0.9f, 0.9f);

        DrawBtn(btnBack);
    }

    glEnable(GL_DEPTH_TEST);
}

// =====================================
// RENDER PAUSE MENU
// =====================================

void Menu::RenderPause(double mx, double my)
{
    UpdatePauseHovers(mx, my);

    glUseProgram(shaderProgram);
    glDisable(GL_DEPTH_TEST);

    float vw = (float)screenW;
    float vh = (float)screenH;
    float cx = vw / 2.0f;

    DrawRect(0, 0, vw, vh, 0.0f, 0.0f, 0.0f, 0.72f);

    float pw = screenW * 0.35f;
    float ph = screenH * 0.60f;
    float px = cx - pw / 2.0f;
    float py = screenH * 0.20f;

    DrawRect(px - 3, py - 3, pw + 6, ph + 6, 0.6f, 0.0f, 0.0f, 1.0f);
    DrawRect(px, py, pw, ph, 0.05f, 0.05f, 0.05f, 1.0f);

    if (pauseSubmenu == GameState::MENU)
    {
        float titleScale = screenH * 0.013f;
        float hintScale = screenH * 0.003f;

        DrawText2D("PAUSED",
            cx - screenW * 0.09f, py + ph * 0.08f,
            titleScale, 0.8f, 0.0f, 0.0f);

        DrawRect(px + pw * 0.05f, py + ph * 0.22f,
            pw * 0.90f, 2.0f, 0.6f, 0.0f, 0.0f, 1.0f);

        DrawBtn(btnResume);
        DrawBtn(btnPauseOptions);
        DrawBtn(btnMainMenu);

        DrawText2D("Press ESC to continue",
            cx - screenW * 0.09f, py + ph * 0.88f,
            hintScale, 0.35f, 0.35f, 0.35f);
    }
    else if (pauseSubmenu == GameState::OPTIONS)
    {
        float titleScale = screenH * 0.010f;

        DrawText2D("OPTIONS",
            cx - screenW * 0.07f, py + ph * 0.07f,
            titleScale, 0.8f, 0.0f, 0.0f);

        DrawRect(px + pw * 0.05f, py + ph * 0.20f,
            pw * 0.90f, 2.0f, 0.6f, 0.0f, 0.0f, 1.0f);

        RenderOptionsContent();
        DrawBtn(btnBack);
    }

    glEnable(GL_DEPTH_TEST);
}

// =====================================
// OPTIONS CONTENT
// FIXES: botones +/- con escala de texto correcta (proporcional al botón pequeño)
// =====================================

void Menu::RenderOptionsContent()
{
    float cx = screenW / 2.0f;
    float textScale = screenH * 0.004f;
    float barW = screenW * 0.20f;
    float barH = screenH * 0.018f;
    float barX = cx - barW / 2.0f;

    float sensLabelY = screenH * 0.50f;
    float sensBarY = screenH * 0.54f;
    float volLabelY = screenH * 0.63f;
    float volBarY = screenH * 0.67f;

    // Sensibilidad label + barra
    char sensBuf[64];
    sprintf_s(sensBuf, "Sensitivity: %.2f", sensitivity);
    DrawText2D(sensBuf, barX, sensLabelY, textScale, 0.9f, 0.9f, 0.9f);
    DrawRect(barX, sensBarY, barW, barH, 0.3f, 0.0f, 0.0f, 1.0f);
    DrawRect(barX, sensBarY, barW * sensitivity / 0.5f, barH, 0.8f, 0.0f, 0.0f, 1.0f);

    // ?? Botones +/- pequeños ??
    // Escala de texto proporcional al tamaño real del botón (sbh * 0.35 / 9)
    // que con sbh ? 27px da scale ? 1.05 — apropiado para el símbolo +/-
    auto drawSmallBtn = [&](MenuButton& btn)
        {
            // Borde + fondo
            DrawRect(btn.x - 2, btn.y - 2, btn.width + 4, btn.height + 4,
                0.6f, 0.0f, 0.0f, 1.0f);
            DrawRect(btn.x, btn.y, btn.width, btn.height,
                btn.hovered ? 0.5f : 0.15f, 0.0f, 0.0f, 1.0f);

            // Texto: escala proporcional al alto del botón pequeño
            float s = (btn.height * 0.40f) / 9.0f;   // 40% del alto, base stb=9px
            // Centrar el carácter (+/-) dentro del botón
            float charW = s * 9.0f;
            float tx = btn.x + btn.width / 2.0f - charW / 2.0f;
            float ty = btn.y + btn.height / 2.0f - s * 4.5f;
            DrawText2D(btn.text, tx, ty, s, 1.0f, 1.0f, 1.0f);
        };

    drawSmallBtn(btnSensDown);
    drawSmallBtn(btnSensUp);

    // Volumen label + barra
    char volBuf[64];
    sprintf_s(volBuf, "Volume: %.0f%%", volume * 100.0f);
    DrawText2D(volBuf, barX, volLabelY, textScale, 0.9f, 0.9f, 0.9f);
    DrawRect(barX, volBarY, barW, barH, 0.3f, 0.0f, 0.0f, 1.0f);
    DrawRect(barX, volBarY, barW * volume, barH, 0.8f, 0.0f, 0.0f, 1.0f);

    drawSmallBtn(btnVolDown);
    drawSmallBtn(btnVolUp);
}

// =====================================
// HANDLE INPUT — MAIN MENU
// =====================================

GameState Menu::HandleInput(GLFWwindow* window, double mx, double my)
{
    bool clicking = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (clicking && !mousePressed)
    {
        mousePressed = true;

        if (currentSubmenu == GameState::MENU)
        {
            if (IsHovered(btnPlay, mx, my)) return GameState::PLAYING;
            if (IsHovered(btnOptions, mx, my)) currentSubmenu = GameState::OPTIONS;
            if (IsHovered(btnCredits, mx, my)) currentSubmenu = GameState::CREDITS;
            if (IsHovered(btnExit, mx, my)) return GameState::EXIT;
        }
        else
        {
            if (IsHovered(btnBack, mx, my))
                currentSubmenu = GameState::MENU;

            if (currentSubmenu == GameState::OPTIONS)
            {
                if (IsHovered(btnSensUp, mx, my)) sensitivity = glm::min(sensitivity + 0.05f, 0.5f);
                if (IsHovered(btnSensDown, mx, my)) sensitivity = glm::max(sensitivity - 0.05f, 0.01f);
                if (IsHovered(btnVolUp, mx, my)) volume = glm::min(volume + 0.1f, 1.0f);
                if (IsHovered(btnVolDown, mx, my)) volume = glm::max(volume - 0.1f, 0.0f);
            }
        }
    }

    if (!clicking) mousePressed = false;
    return GameState::MENU;
}

// =====================================
// HANDLE INPUT — PAUSE MENU
// =====================================

GameState Menu::HandlePauseInput(GLFWwindow* window, double mx, double my)
{
    bool clicking = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (clicking && !mousePressed)
    {
        mousePressed = true;

        if (pauseSubmenu == GameState::MENU)
        {
            if (IsHovered(btnResume, mx, my)) return GameState::PLAYING;
            if (IsHovered(btnPauseOptions, mx, my)) pauseSubmenu = GameState::OPTIONS;
            if (IsHovered(btnMainMenu, mx, my))
            {
                pauseSubmenu = GameState::MENU;
                currentSubmenu = GameState::MENU;
                return GameState::MENU;
            }
        }
        else if (pauseSubmenu == GameState::OPTIONS)
        {
            if (IsHovered(btnBack, mx, my)) pauseSubmenu = GameState::MENU;
            if (IsHovered(btnSensUp, mx, my)) sensitivity = glm::min(sensitivity + 0.05f, 0.5f);
            if (IsHovered(btnSensDown, mx, my)) sensitivity = glm::max(sensitivity - 0.05f, 0.01f);
            if (IsHovered(btnVolUp, mx, my)) volume = glm::min(volume + 0.1f, 1.0f);
            if (IsHovered(btnVolDown, mx, my)) volume = glm::max(volume - 0.1f, 0.0f);
        }
    }

    if (!clicking) mousePressed = false;
    return GameState::MENU;
}

void Menu::RenderEndScreen(double mx, double my, bool isVictory)
{
    btnRetry.hovered = IsHovered(btnRetry, mx, my);
    btnMainMenu.hovered = IsHovered(btnMainMenu, mx, my);

    glUseProgram(shaderProgram);
    glDisable(GL_DEPTH_TEST);

    // Fondo negro
    DrawRect(0, 0, (float)screenW, (float)screenH, 0.0f, 0.0f, 0.0f, 1.0f);

    if (isVictory)
    {
        DrawText2D("YOU SURVIVED",
            screenW / 2.0f - 215.0f, 150.0f,
            6.0f, 0.0f, 0.8f, 0.2f); // verde

        DrawText2D("You restored power and escaped the lab",
            screenW / 2.0f - 230.0f, 250.0f,
            2.2f, 0.7f, 0.7f, 0.7f);
    }
    else
    {
        DrawText2D("GAME OVER",
            screenW / 2.0f - 170.0f, 150.0f,
            6.0f, 0.8f, 0.0f, 0.0f); // rojo

        DrawText2D("The creature caught you in the dark",
            screenW / 2.0f - 220.0f, 250.0f,
            2.2f, 0.7f, 0.7f, 0.7f);
    }

    DrawRect(screenW / 2.0f - 200.0f, 320.0f, 400.0f, 2.0f, 0.6f, 0.0f, 0.0f, 1.0f);

    // Botones
    auto drawBtn = [&](MenuButton& btn)
        {
            DrawRect(btn.x - 2, btn.y - 2, btn.width + 4, btn.height + 4,
                0.6f, 0.0f, 0.0f, 1.0f);

            DrawRect(btn.x, btn.y, btn.width, btn.height,
                btn.hovered ? 0.7f : 0.15f, 0.0f, 0.0f, 1.0f);

            float tx = btn.x + btn.width / 2.0f - btn.text.size() * 4.5f;
            float ty = btn.y + btn.height / 2.0f - 8.0f;

            DrawText2D(btn.text, tx, ty, 2.0f, 0.9f, 0.9f, 0.9f);
        };

    drawBtn(btnRetry);
    drawBtn(btnMainMenu);

    glEnable(GL_DEPTH_TEST);
}

GameState Menu::HandleEndScreenInput(GLFWwindow* window, double mx, double my, bool isVictory)
{
    bool clicking = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (clicking && !mousePressed)
    {
        mousePressed = true;

        if (IsHovered(btnRetry, mx, my))
            return GameState::PLAYING;

        if (IsHovered(btnMainMenu, mx, my))
            return GameState::MENU;
    }

    if (!clicking) mousePressed = false;

    return (isVictory ? GameState::VICTORY : GameState::GAME_OVER);
}

// =====================================
// CLEANUP
// =====================================

void Menu::Cleanup()
{
    if (VAO)           glDeleteVertexArrays(1, &VAO);
    if (VBO)           glDeleteBuffers(1, &VBO);
    if (textVAO)       glDeleteVertexArrays(1, &textVAO);
    if (textVBO)       glDeleteBuffers(1, &textVBO);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}