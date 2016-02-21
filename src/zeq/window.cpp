
#include "window.hpp"

extern Log gLog;
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
    
    gLog.printf("OpenGL %u.%u\n", settings.majorVersion, settings.minorVersion);
    
    OpenGL::loadExtensions();
    
    m_input.getCamera().applyView();
    m_prevTime = m_deltaTimer.seconds();
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
    m_entityList.animateModels(delta, getCamera().getPosition());
    
    drawAll();
    
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

void Window::drawAll()
{
    clear();
    
    // Set draw states
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Draw world models
    Camera& camera = getCamera();
    camera.recalculate();
    camera.applyView();
    
    glMatrixMode(GL_MODELVIEW);
    
    if (m_zoneModel)
        m_zoneModel->draw(camera);
    
    m_entityList.drawModels(camera);
    
    // Draw GUI
    
    // Swap frame buffers
    display();
    
    // Clear draw states
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}

void Window::loadZoneModel(const std::string& shortname)
{
    ZoneModel* prevZone = m_zoneModel;
    
    m_zoneModel = gModelResources.loadZoneModel(shortname);
    
    if (prevZone)
        prevZone->drop();
    
    setTitle("ZEQ :: " + shortname);
    
    AnimatedModelPrototype* model = gModelResources.getMobModel(75, 2);
    
    Skeleton* skele = nullptr;
    if (model)
    {
        skele = model->createSkeletonInstance();
        skele->setAnimation(5);
        
        m_entityList.add(skele, Vec3(-20, 0, 0));
    }
    
    model = gModelResources.getMobModel(2, 2);
    
    if (model)
    {
        Skeleton* skele2 = model->createSkeletonInstance();
        skele2->setAnimation(5);
        
        m_entityList.add(skele2, Vec3(-20, 0, 0));
        
        if (skele)
            skele->attach(AttachPoint::Slot::RightHand, skele2);
    }
}
