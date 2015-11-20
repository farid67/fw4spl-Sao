/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "visuOgreQt/Window.hpp"

#define FW_PROFILING_DISABLED
#include <fwCore/Profiling.hpp>

#include <fwRenderOgre/Utils.hpp>
#include <fwRenderOgre/WindowManager.hpp>

#include <OgreOverlay.h>
#include <OgreOverlayManager.h>

#define ZOOM_SPEED 0.2

namespace visuOgreQt
{

int Window::m_counter = 0;
::Ogre::OverlaySystem* Window::m_ogreOverlaySystem = 0;

// ----------------------------------------------------------------------------

Window::Window(QWindow *parent) :
    QWindow(parent),
    m_id(Window::m_counter++),
    m_ogreRoot(nullptr),
    m_ogreRenderWindow(nullptr),
    //    m_trayMgr(nullptr),
    m_update_pending(false),
    m_animating(false),
    m_isInitialised(false),
    m_showOverlay(false),
    m_lastPosLeftClick(nullptr),
    m_lastPosMiddleClick(nullptr)
{
    setAnimating(false);
    installEventFilter(this);
}

// ----------------------------------------------------------------------------

Window::~Window()
{
    destroy();
}

// ----------------------------------------------------------------------------

void Window::showOverlay(bool show)
{
    m_showOverlay = show;
}
// ----------------------------------------------------------------------------

void Window::render(QPainter *painter)
{
    Q_UNUSED(painter);
}

// ----------------------------------------------------------------------------

void Window::initialise()
{
    m_ogreRoot = ::fwRenderOgre::Utils::getOgreRoot();

    ::Ogre::RenderSystem* rs = m_ogreRoot->getRenderSystem();
    SLM_ASSERT("OpenGL RenderSystem not found", rs->getName().find("GL") != std::string::npos);

    Ogre::NameValuePairList parameters;

    ::fwRenderOgre::WindowManager::sptr mgr = ::fwRenderOgre::WindowManager::get();

    // We share the OpenGL context on all windows. The first Ogre window will create the context, the other ones will
    // reuse the current context
    if(!mgr->hasWindow())
    {
        parameters["currentGLContext"] = Ogre::String("false");
    }
    else
    {
        parameters["currentGLContext"] = Ogre::String("true");
    }

    /*
       We need to supply the low level OS window handle to this QWindow so that Ogre3D knows where to draw
       the scene. Below is a cross-platform method on how to do this.
       If you set both options (externalWindowHandle and parentWindowHandle) this code will work with OpenGL
       and DirectX.
     */
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
    parameters["externalWindowHandle"] = Ogre::StringConverter::toString((size_t)(this->winId()));
    parameters["parentWindowHandle"]   = Ogre::StringConverter::toString((size_t)(this->winId()));
#else
    parameters["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned long)(this->winId()));
//    parameters["parentWindowHandle"]   = Ogre::StringConverter::toString((unsigned long)(this->winId()));
#endif

#if defined(Q_OS_MAC)
    parameters["macAPI"]               = "cocoa";
    parameters["macAPICocoaUseNSView"] = "true";
#endif

    /*
       Note below that we supply the creation function for the Ogre3D window the width and height
       from the current QWindow object using the "this" pointer.
     */
    m_ogreRenderWindow = m_ogreRoot->createRenderWindow("Widget-RenderWindow_" + std::to_string(m_id),
                                                        static_cast<unsigned int>(this->width()),
                                                        static_cast<unsigned int>(this->height()),
                                                        false,
                                                        &parameters);

    m_ogreRenderWindow->setVisible(true);
    m_ogreRenderWindow->setAutoUpdated(false);
    m_ogreRenderWindow->addListener(this);

    mgr->registerWindow(m_ogreRenderWindow);

    Q_EMIT renderWindowCreated();

    ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo info;
    info.interactionType = ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo::RESIZE;
    info.x               = this->width();
    info.y               = this->height();
    info.dx              = 0;
    info.dy              = 0;
    Q_EMIT interacted(info);
}

// ----------------------------------------------------------------------------

void Window::requestRender()
{
    Q_EMIT cameraClippingComputation();
    this->renderLater();
}

//------------------------------------------------------------------------------

void Window::makeCurrent()
{
    if(m_ogreRenderWindow)
    {
        ::Ogre::RenderSystem* renderSystem = m_ogreRoot->getRenderSystem();

        if(renderSystem)
        {
            // This allows to set the current OpengGL context in Ogre internal state
            renderSystem->_setRenderTarget(m_ogreRenderWindow);

            // Use this trick to apply the current OpenGL context
            //
            // Actually this method does the following :
            // void GLRenderSystem::postExtraThreadsStarted()
            // {
            //   OGRE_LOCK_MUTEX(mThreadInitMutex);
            //   if(mCurrentContext)
            //     mCurrentContext->setCurrent();
            // }
            //
            // This is actually want we want to do, even if this is not the initial purpose of this method
            //
            renderSystem->postExtraThreadsStarted();
        }
    }
}

// ----------------------------------------------------------------------------

void Window::destroyWindow()
{
    Window::m_counter--;

    if(m_ogreRenderWindow)
    {
        ::fwRenderOgre::WindowManager::sptr mgr = ::fwRenderOgre::WindowManager::get();
        mgr->unregisterWindow(m_ogreRenderWindow);
    }

    if (m_lastPosLeftClick)
    {
        delete m_lastPosLeftClick;
    }
    if (m_lastPosMiddleClick)
    {
        delete m_lastPosMiddleClick;
    }
}

// ----------------------------------------------------------------------------

void Window::render()
{
    /*
       How we tied in the render function for OGre3D with QWindow's render function. This is what gets call
       repeatedly. Note that we don't call this function directly; rather we use the renderNow() function
       to call this method as we don't want to render the Ogre3D scene unless everything is set up first.
       That is what renderNow() does.

       Theoretically you can have one function that does this check but from my experience it seems better
       to keep things separate and keep the render function as simple as possible.
     */
    Ogre::WindowEventUtilities::messagePump();

    FW_PROFILE_FRAME_AVG("Ogre", 3);
    FW_PROFILE_AVG("Ogre", 3);
    this->makeCurrent();

    m_ogreRoot->_fireFrameStarted();
    m_ogreRenderWindow->update();
    m_ogreRoot->_fireFrameRenderingQueued();
//    Ogre::FrameEvent evt;
//    m_trayMgr->frameRenderingQueued(evt);
    m_ogreRoot->_fireFrameEnded();

#ifdef FW_PROFILING_DISABLED
    static unsigned int i = 0;
    if( !(++i % 100 ) )
    {
        Ogre::RenderTarget::FrameStats stats = m_ogreRenderWindow->getStatistics();
        OSLM_DEBUG("Render target last FPS: " << stats.lastFPS);
    }
#endif
}

// ----------------------------------------------------------------------------

void Window::renderLater()
{
    /*
       This function forces QWindow to keep rendering. Omitting this causes the renderNow() function to
       only get called when the window is resized, moved, etc. as opposed to all of the time; which is
       generally what we need.
     */
    if (!m_update_pending)
    {
        m_update_pending = true;
        QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

// ----------------------------------------------------------------------------

bool Window::event(QEvent *event)
{
    /*
       QWindow's "message pump". The base method that handles all QWindow events. As you will see there
       are other methods that actually process the keyboard/other events of Qt and the underlying OS.

       Note that we call the renderNow() function which checks to see if everything is initialized, etc.
       before calling the render() function.
     */

    switch (event->type())
    {
        case QEvent::UpdateRequest:
            m_update_pending = false;
            renderNow();
            return true;

        default:
            return QWindow::event(event);
    }
}

// ----------------------------------------------------------------------------

void Window::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
    {
        this->renderNow();
    }
}

// ----------------------------------------------------------------------------

void Window::renderNow()
{
    if (!isExposed())
    {
        return;
    }

    if(!m_isInitialised)
    {
        this->initialise();
        m_isInitialised = true;
    }

    this->render();

    if (m_animating)
    {
        this->renderLater();
    }
}

// ----------------------------------------------------------------------------

bool Window::eventFilter(QObject *target, QEvent *event)
{
    if (target == this)
    {
        if (event->type() == QEvent::Resize)
        {
            if (m_ogreRenderWindow != nullptr)
            {
                this->makeCurrent();

                m_ogreRenderWindow->reposition(x(), y());
#if defined(linux) || defined(__linux)
                m_ogreRenderWindow->resize(static_cast< unsigned int >(this->width()),
                                           static_cast< unsigned int >(this->height()));
#endif
                m_ogreRenderWindow->windowMovedOrResized();

                const auto numViewports = m_ogreRenderWindow->getNumViewports();

                ::Ogre::Viewport* viewport = nullptr;
                for (unsigned short i = 0; i < numViewports; i++)
                {
                    viewport = m_ogreRenderWindow->getViewport(i);
                    viewport->getCamera()->setAspectRatio(Ogre::Real(this->width()) / ::Ogre::Real(this->height()));
                }

                if (viewport && ::Ogre::CompositorManager::getSingleton().hasCompositorChain(viewport))
                {
                    ::Ogre::CompositorChain* chain = ::Ogre::CompositorManager::getSingleton().getCompositorChain(
                        viewport);
                    size_t length = chain->getNumCompositors();
                    for(size_t i = 0; i < length; i++)
                    {
                        if( chain->getCompositor(i)->getEnabled() )
                        {
                            chain->setCompositorEnabled(i, false);
                            chain->setCompositorEnabled(i, true);
                        }
                    }
                }

                ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo info;
                info.interactionType = ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo::RESIZE;
                info.x               = this->width();
                info.y               = this->height();
                info.dx              = 0;
                info.dy              = 0;
                Q_EMIT interacted(info);

                if (isExposed())
                {
                    this->requestRender();
                }
            }
        }
    }

    return false;
}

// ----------------------------------------------------------------------------

void Window::keyPressEvent(QKeyEvent * e)
{
    ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo info;
    info.interactionType = ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo::KEYPRESS;
    info.key             = e->key();
    Q_EMIT interacted(info);
}

// ----------------------------------------------------------------------------

void Window::mouseMoveEvent( QMouseEvent* e )
{
    if (e->buttons() & ::Qt::LeftButton && m_lastPosLeftClick)
    {
        int x  = m_lastPosLeftClick->x();
        int y  = m_lastPosLeftClick->y();
        int dx = x - e->x();
        int dy = y - e->y();
        ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo info;
        info.interactionType = ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo::MOUSEMOVE;
        info.x               = x;
        info.y               = y;
        info.dx              = dx;
        info.dy              = dy;
        Q_EMIT interacted(info);
        m_lastPosLeftClick->setX(e->x());
        m_lastPosLeftClick->setY(e->y());
        this->requestRender();

        m_mousedMoved = true;
    }
    else if (e->buttons() & ::Qt::MiddleButton && m_lastPosMiddleClick )
    {
        int x  = m_lastPosMiddleClick->x();
        int y  = m_lastPosMiddleClick->y();
        int dx = x - e->x();
        int dy = y - e->y();

        ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo info;
        info.interactionType = ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo::HORIZONTALMOVE;
        info.x               = x;
        info.dx              = dx;
        Q_EMIT interacted(info);

        ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo info2;
        info2.interactionType = ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo::VERTICALMOVE;
        info2.y               = y;
        info2.dy              = dy;
        Q_EMIT interacted(info2);

        m_lastPosMiddleClick->setX(e->x());
        m_lastPosMiddleClick->setY(e->y());
        this->requestRender();
    }
}

// ----------------------------------------------------------------------------

void Window::wheelEvent(QWheelEvent *e)
{
    ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo info;
    info.interactionType = ::fwRenderOgre::IRenderWindowInteractorManager::InteractionInfo::WHEELMOVE;
    info.delta           = static_cast<int>(e->delta()*ZOOM_SPEED);
    info.x               = e->x();
    info.y               = e->y();
    info.dx              = 0;
    info.dy              = 0;

    Q_EMIT interacted(info);
    Q_EMIT cameraClippingComputation();

    this->requestRender();
}

// ----------------------------------------------------------------------------

void Window::mousePressEvent( QMouseEvent* e )
{
    if(e->button() == Qt::LeftButton)
    {
        m_lastPosLeftClick = new QPoint(e->x(), e->y());

        m_mousedMoved = false;
    }
    else if(e->button() == Qt::MiddleButton)
    {
        m_lastPosMiddleClick = new QPoint(e->x(), e->y());
    }

    this->requestRender();
}

// ----------------------------------------------------------------------------

void Window::mouseReleaseEvent( QMouseEvent* e )
{
    if(e->button() == Qt::LeftButton && m_lastPosLeftClick)
    {
        if( !m_mousedMoved )
        {
            Q_EMIT rayCastRequested(e->x(),
                                    e->y(),
                                    static_cast<int>(m_ogreRenderWindow->getWidth()),
                                    static_cast<int>(m_ogreRenderWindow->getHeight()));
        }

        delete m_lastPosLeftClick;
        m_lastPosLeftClick = nullptr;

    }
    else if(e->button() == Qt::MiddleButton && m_lastPosMiddleClick)
    {
        delete m_lastPosMiddleClick;
        m_lastPosMiddleClick = nullptr;
    }
}

// ----------------------------------------------------------------------------

void Window::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
    {
        renderLater();
    }
}

// ----------------------------------------------------------------------------

Ogre::RenderWindow* Window::getOgreRenderWindow()
{
    return m_ogreRenderWindow;
}

// ----------------------------------------------------------------------------

Ogre::OverlaySystem* Window::getOgreOverlaySystem()
{
    return m_ogreOverlaySystem;
}

// ----------------------------------------------------------------------------

int Window::getId()
{
    return m_id;
}

//-----------------------------------------------------------------------------

void Window::preViewportUpdate(const ::Ogre::RenderTargetViewportEvent& evt)
{
    ::Ogre::Overlay *overlay = ::Ogre::OverlayManager::getSingletonPtr()->getByName("LogoOverlay");

    if(!m_showOverlay)
    {
        overlay->hide();
    }
    else
    {
        overlay->show();
    }

    ::Ogre::RenderTargetListener::preViewportUpdate(evt);
}

} // namespace visuOgreQt
