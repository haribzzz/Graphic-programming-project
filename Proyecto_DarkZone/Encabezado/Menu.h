#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
enum class GameState
{
    MENU,
    PLAYING,
    OPTIONS,
    CREDITS,
    GAME_OVER,
    VICTORY,
    EXIT
};
struct MenuButton
{
    float x, y, width, height;
    std::string text;
    bool hovered;
};
class Menu
{
public:
    Menu();
    ~Menu();
    void Init(int screenWidth, int screenHeight);
    void Render(double mouseX, double mouseY);
    void RenderPause(double mouseX, double mouseY);
    GameState HandleInput(GLFWwindow* window,
        double mouseX, double mouseY);
    GameState HandlePauseInput(GLFWwindow* window,
        double mouseX, double mouseY);

    // End screens (game over / victory)
    GameState HandleEndScreenInput(GLFWwindow* window, double mouseX, double mouseY, bool isVictory);
    void RenderEndScreen(double mouseX, double mouseY, bool isVictory);

    void Cleanup();
    float sensitivity = 0.1f;
    float volume = 1.0f;
private:
    int screenW, screenH;
    GLuint shaderProgram;
    GLuint VAO, VBO;
    GLuint textVAO, textVBO;
    // Main menu buttons
    MenuButton btnPlay;
    MenuButton btnOptions;
    MenuButton btnCredits;
    MenuButton btnExit;
    // Shared button
    MenuButton btnBack;
    // Options buttons
    MenuButton btnSensUp;
    MenuButton btnSensDown;
    MenuButton btnVolUp;
    MenuButton btnVolDown;
    // Pause menu buttons
    MenuButton btnResume;
    MenuButton btnPauseOptions;
    MenuButton btnMainMenu;
    // End screen buttons
    MenuButton btnRetry;

    GameState currentSubmenu;
    GameState pauseSubmenu;
    void InitShaders();
    void InitGeometry();
    void RebuildPauseButtons();
    void RenderOptionsContent();
    void DrawRect(float x, float y, float w, float h,
        float r, float g, float b, float a);
    void DrawText2D(const std::string& text,
        float x, float y, float scale,
        float r, float g, float b);
    bool IsHovered(const MenuButton& btn,
        double mouseX, double mouseY);
    void UpdateHovers(double mx, double my);
    void UpdatePauseHovers(double mx, double my);
    void DrawBtn(MenuButton& btn);
    bool mousePressed = false;
};
extern Menu* g_Menu;
extern GameState g_GameState;