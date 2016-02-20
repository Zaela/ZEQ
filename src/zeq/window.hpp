
#ifndef _ZEQ_WINDOW_HPP_
#define _ZEQ_WINDOW_HPP_

#include "define.hpp"
#include "opengl.hpp"
#include "input.hpp"
#include "model_resources.hpp"
#include "zone_model.hpp"
#include "entity_list.hpp"
#include "animated_model.hpp"
#include "config.hpp"
#include "log.hpp"
#include <vector>
#include <string>

class Window : public sf::RenderWindow
{
private:
    Input m_input;

    bool m_running;

    double      m_prevTime;
    PerfTimer   m_deltaTimer;

    ZoneModel* m_zoneModel;

    EntityList m_entityList;

private:
    static void clear();
    
public:
    Window();
    virtual ~Window();

    bool mainLoop();
    void pollInput(double delta);
    void drawAll();

    void loadZoneModel(const std::string& shortname);

    Camera& getCamera() { return m_input.getCamera(); }
};

#endif//_ZEQ_WINDOW_HPP_
