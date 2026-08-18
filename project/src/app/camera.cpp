#include "camera.h"



void AppCamera::Update(float deltaTime)
{
    // Movement
    constexpr float MOVE_SPEED { 300.0f };

    if (IsKeyDown(KEY_UP)) m_Position.y -= MOVE_SPEED * deltaTime / m_Scale;
    if (IsKeyDown(KEY_DOWN)) m_Position.y += MOVE_SPEED * deltaTime / m_Scale;
    if (IsKeyDown(KEY_LEFT)) m_Position.x -= MOVE_SPEED * deltaTime / m_Scale;
    if (IsKeyDown(KEY_RIGHT)) m_Position.x += MOVE_SPEED * deltaTime / m_Scale;

    const bool plusPressed { IsKeyPressed(KEY_EQUAL) };
    const bool minusPressed { IsKeyPressed(KEY_MINUS) };

    float scroll { 0.0f };

    if (plusPressed && !minusPressed)
        scroll = 1.0f;
    else if (!plusPressed && minusPressed)
        scroll = -1.0f;

    // Zoom
    constexpr float ZOOM_INCREMENT { 0.2f };
    constexpr float ZOOM_MIN { 0.1f };
    constexpr float ZOOM_MAX { 10.0f };

    if (scroll != 0.0f)
    {
        m_Scale *= (1.0f + scroll * ZOOM_INCREMENT);

        if (m_Scale < ZOOM_MIN) m_Scale = ZOOM_MIN;
        if (m_Scale > ZOOM_MAX) m_Scale = ZOOM_MAX;
    }
}

Vector2 AppCamera::GetPosition() const
{
    return m_Position;
}

float AppCamera::GetScale() const
{
    return m_Scale;
}