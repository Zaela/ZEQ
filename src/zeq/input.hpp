
#ifndef _ZEQ_INPUT_HPP_
#define _ZEQ_INPUT_HPP_

#include "define.hpp"
#include "vec3.hpp"
#include "vec2.hpp"
#include "mat4.hpp"
#include "camera.hpp"
#include <algorithm>

class Input
{
private:
    Camera m_camera;

    int8_t m_move;
    int8_t m_turn;

    bool m_mouseLook;
    bool m_mouseLookMoved;

    static const int8_t MOVE_FORWARD    = 1;
    static const int8_t MOVE_BACKWARD   = -1;
    static const int8_t TURN_LEFT       = -1;
    static const int8_t TURN_RIGHT      = 1;

    Vec2 m_mouseRel;
    Vec2 m_mousePos;

    bool m_close;

private:
    void handleKeyPress(sf::Event::KeyEvent& key);
    void handleKeyRelease(sf::Event::KeyEvent& key);
    void handleMouseMove(sf::Event::MouseMoveEvent& mouse);
    void handleMousePress(sf::Event::MouseButtonEvent& mouse);
    void handleMouseRelease(sf::Event::MouseButtonEvent& mouse);
    
public:
    Input();

    void update(double delta);
    bool handleEvent(sf::Event& ev);

    Camera& getCamera() { return m_camera; }
};

#endif//_ZEQ_INPUT_HPP_
