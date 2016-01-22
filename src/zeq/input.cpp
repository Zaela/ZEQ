
#include "input.hpp"

Input::Input()
    : m_move(0),
      m_turn(0),
      m_mouseLook(false),
      m_mouseLookMoved(false),
      m_close(false)
{
    
}

void Input::update(double delta)
{
    if (!m_move && !m_turn && (!m_mouseLook || !m_mouseLookMoved))
        return;
    
    Vec3& pos   = m_camera.getPosition();
    Vec3 target = m_camera.getTarget() - pos;
    
    Vec2 relativeRotation = target.horizontalAngle();
    
    relativeRotation.y += m_mouseRel.x;
    relativeRotation.x -= m_mouseRel.y;
    
    m_mouseRel.set(0.0f, 0.0f);
    m_mouseLookMoved = false;
    
    if (!m_mouseLook && m_turn)
        relativeRotation.y += delta * 100.0 * m_turn;
    
    target.set(0.0f, 0.0f, std::max(1.0f, pos.length()));
    
    // Avoid gimbal lock
    if (relativeRotation.x > 180.0f)
    {
        if (relativeRotation.x < 271.0f)
            relativeRotation.x = 271.0f;
    }
    else
    {
        if (relativeRotation.x > 89.0f)
            relativeRotation.x = 89.0f;
    }
    
    Mat4 mat;
    mat.identity();
    mat.setRotation(relativeRotation.x, relativeRotation.y, 0.0f);
    mat.transformVector(target);
    
    if (m_move)
    {
        Vec3 moveDir = target;
        moveDir.normalize();
        moveDir *= delta * 100.0f * m_move;
        pos -= moveDir;
    }
    
    if (m_mouseLook && m_turn)
    {
        Vec3 strafe = target.crossProduct(m_camera.getUpVector());
        strafe.normalize();
        strafe *= delta * 100.0f * -m_turn;
        pos -= strafe;
    }
    
    target += pos;
    
    m_camera.setTarget(target);
    m_camera.recalculateView();
    m_camera.applyView();
}

bool Input::handleEvent(sf::Event& ev)
{
    switch (ev.type)
    {
    case sf::Event::Closed:
        return false;
    case sf::Event::KeyPressed:
        handleKeyPress(ev.key);
        break;
    case sf::Event::KeyReleased:
        handleKeyRelease(ev.key);
        break;
    case sf::Event::MouseMoved:
        handleMouseMove(ev.mouseMove);
        break;
    case sf::Event::MouseButtonPressed:
        handleMousePress(ev.mouseButton);
        break;
    case sf::Event::MouseButtonReleased:
        handleMouseRelease(ev.mouseButton);
        break;
    default:
        break;
    }
    
    return !m_close;
}

void Input::handleKeyPress(sf::Event::KeyEvent& key)
{
    switch (key.code)
    {
    case sf::Keyboard::Up:
        m_move = MOVE_FORWARD;
        break;
    case sf::Keyboard::Down:
        m_move = MOVE_BACKWARD;
        break;
    case sf::Keyboard::Left:
        m_turn = TURN_LEFT;
        break;
    case sf::Keyboard::Right:
        m_turn = TURN_RIGHT;
        break;
    default:
        break;
    }
}

void Input::handleKeyRelease(sf::Event::KeyEvent& key)
{
    switch (key.code)
    {
    case sf::Keyboard::Up:
        if (m_move == MOVE_FORWARD)
            m_move = 0;
        break;
    case sf::Keyboard::Down:
        if (m_move == MOVE_BACKWARD)
            m_move = 0;
        break;
    case sf::Keyboard::Left:
        if (m_turn == TURN_LEFT)
            m_turn = 0;
        break;
    case sf::Keyboard::Right:
        if (m_turn == TURN_RIGHT)
            m_turn = 0;
        break;
    case sf::Keyboard::Escape:
        m_close = true;
        break;
    default:
        break;
    }
}

void Input::handleMouseMove(sf::Event::MouseMoveEvent& mouse)
{
    if (m_mouseLook)
    {
        m_mouseRel.x += mouse.x - m_mousePos.x;
        m_mouseRel.y += mouse.y - m_mousePos.y;
        m_mouseLookMoved = true;
    }
    
    m_mousePos.set(mouse.x, mouse.y);
}

void Input::handleMousePress(sf::Event::MouseButtonEvent& mouse)
{
    switch (mouse.button)
    {
    case sf::Mouse::Right:
        m_mouseLook = true;
        break;
    default:
        break;
    }
}

void Input::handleMouseRelease(sf::Event::MouseButtonEvent& mouse)
{
    switch (mouse.button)
    {
    case sf::Mouse::Right:
        m_mouseLook = false;
        break;
    default:
        break;
    }
}
