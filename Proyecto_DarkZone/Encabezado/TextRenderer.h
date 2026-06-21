#pragma once

#include <glad/glad.h>

bool InitTextRenderer();

// scale: factor de tamaño. 1.0 = tamaño base stb (~9px).
// Usa 2.0–2.5 para textos HUD normales, 1.5 para textos pequeños.
void RenderizarTexto(
    float x,
    float y,
    const char* text,
    int screenWidth,
    int screenHeight,
    float scale = 2.0f   // valor por defecto: doble del original
);

void CleanupTextRenderer();