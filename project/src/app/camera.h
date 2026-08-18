#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>



class AppCamera final
{
    public:

    AppCamera() = default;
    ~AppCamera() = default;

    void Update(float deltaTime);

    Vector2 GetPosition() const;
    float GetScale() const;

    private:

    Vector2 m_Position { };
    float m_Scale { 1.0f };
};

#endif // !CAMERA_H