
#include "window.hpp"

WindowSet gWindowSet;
extern Config gConfig;
extern ModelResources gModelResources;

Window::Window()
    : m_running(true),
      m_zoneModel(nullptr)
{
    sf::ContextSettings settings;
    settings.depthBits          = 24;
    settings.stencilBits        = 8;
    settings.antialiasingLevel  = gConfig.getInt(Config::AntiAliasLevel, 0);
    settings.majorVersion       = 3;
    settings.minorVersion       = 0;
    
    int style = 0;
    
    if (!gConfig.getBool(Config::HideTitleBar, false))
    {
        style = sf::Style::Titlebar | sf::Style::Close;
    }
    
    if (gConfig.getBool(Config::Fullscreen, false))
    {
        style |= sf::Style::Fullscreen;
    }
    
    sf::VideoMode videoMode;
    
    if (gConfig.getBool(Config::UseNativeAspectRatio, false))
    {
        videoMode = sf::VideoMode::getDesktopMode();
    }
    else
    {
        videoMode.width     = gConfig.getInt(Config::ScreenWidth, 800);
        videoMode.height    = gConfig.getInt(Config::ScreenHeight, 600);
    }
    
    create(videoMode, "ZEQ", style, settings);
    
    setKeyRepeatEnabled(false);
    setVerticalSyncEnabled(gConfig.getBool(Config::Vsync, false));
    
    settings = getSettings();
    
    printf("OpenGL %u.%u\n", settings.majorVersion, settings.minorVersion);
    
    OpenGL::loadExtensions();
    
    m_input.getCamera().applyView();
    m_prevTime = m_deltaTimer.seconds();
    
    
    
    
    
    m_animModel = nullptr;
    m_skele = nullptr;
}

Window::~Window()
{
    close();
    
    if (m_zoneModel)
        m_zoneModel->drop();
}

bool Window::mainLoop()
{
    double time     = m_deltaTimer.seconds();
    double delta    = time - m_prevTime;
    m_prevTime      = time;
    
    pollInput(delta);
    
    clear();

    Camera* cam = &getCamera();
    cam->recalculate();
    cam->applyView();
    
    if (m_zoneModel)
        m_zoneModel->draw(&getCamera());
    
    //if (m_animModel)
    //    m_animModel->draw();
    if (m_skele)
        m_skele->draw();
    
    display();
    
    return m_running;
}

void Window::pollInput(double delta)
{
    sf::Event ev;
    
    while (pollEvent(ev))
    {
        if (!m_input.handleEvent(ev))
            m_running = false;
    }
    
    m_input.update(delta);
}

void Window::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::loadZoneModel(const std::string& shortname)
{
    ZoneModel* prevZone = m_zoneModel;
    
    m_zoneModel = gModelResources.loadZoneModel(shortname);
    
    if (prevZone)
        prevZone->drop();
    
    setTitle("ZEQ :: " + shortname);
    
    AnimatedModelPrototype* model = gModelResources.getMobModel(1, 2);
    m_animModel = model;
    
    m_skele = model->createSkeletonInstance();
    m_skele->setAnimation(1);
    m_skele->animate(0.066);
}
