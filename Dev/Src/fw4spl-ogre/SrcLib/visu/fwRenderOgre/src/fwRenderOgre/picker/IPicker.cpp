/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "fwRenderOgre/picker/IPicker.hpp"

#include <fwRenderOgre/Utils.hpp>
#include <fwRenderOgre/collisionTools/CollisionTools.hpp>

#include <fwServices/Base.hpp>
#include <fwServices/helper/Config.hpp>
#include <fwServices/macros.hpp>

#include <cmath>

#include <OGRE/OgreRay.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreMovableObject.h>

#include <OGRE/OgreManualObject.h>

namespace fwRenderOgre
{

namespace picker
{

// ----------------------------------------------------------------------------

IPicker::IPicker() :
    m_sceneManager(nullptr),
    m_selectedObject(nullptr),
    m_hasSceneManager(false)
{
}

// ----------------------------------------------------------------------------

IPicker::~IPicker()
{
}

bool IPicker::executeRaySceneQuery(int x, int y, int width, int height)
{
    ::Ogre::Ray r = m_sceneManager->getCamera("PlayerCam")->getCameraToViewportRay(
        static_cast< ::Ogre::Real>(x) / static_cast< ::Ogre::Real>(width),
        static_cast< ::Ogre::Real>(y) / static_cast< ::Ogre::Real>(height));

    float distance;

#ifdef SHOW_BOUNDS
    if (m_selectedObject)
    {
        m_selectedObject->getParentSceneNode()->showBoundingBox(false);
    }
#endif

    ::fwRenderOgre::CollisionTools tool = ::fwRenderOgre::CollisionTools(m_sceneManager);
    bool entityFound = tool.raycast(r, m_rayIntersect, m_selectedObject, distance,
                                    ::Ogre::SceneManager::ENTITY_TYPE_MASK);


    if (entityFound)
    {

#ifdef SHOW_BOUNDS
        m_selectedObject->getParentSceneNode()->showBoundingBox(true);
#endif

        OSLM_DEBUG("Entity find and intersect at " << getIntersectionInWorldSpace() << "(WS)");

        OSLM_DEBUG("Entity find and intersect at " << getIntersectionInViewSpace() << "(VS)");

        OSLM_DEBUG("Entity find and intersect at " << getIntersectionInPixel() << "(Px)");

    }
    else
    {
        m_selectedObject = nullptr;
    }

    return entityFound;
}

//-----------------------------------------------------------------------------

::Ogre::SceneNode* IPicker::getCameraSceneNode() const
{
    SLM_ASSERT("The associated SceneManager is not instanciated", m_sceneManager);
    return m_sceneManager->getCamera("PlayerCam")->getParentSceneNode();
}

//-----------------------------------------------------------------------------

::Ogre::Vector3 IPicker::getIntersectionInWorldSpace() const
{
    return m_rayIntersect;
}

//-----------------------------------------------------------------------------

::Ogre::Vector2 IPicker::getIntersectionInViewSpace() const
{
    ::Ogre::Camera* cam        = m_sceneManager->getCamera("PlayerCam");
    ::Ogre::Matrix4 viewMatrix = cam->getViewMatrix();
    ::Ogre::Matrix4 projMatrix = cam->getProjectionMatrixWithRSDepth();

    ::Ogre::Vector3 point = projMatrix * (viewMatrix * m_rayIntersect);

    ::Ogre::Vector2 screenSpacePoint = ::Ogre::Vector2::ZERO;
    screenSpacePoint.x               = (point.x / 2.f) + 0.5f;
    screenSpacePoint.y               = (point.y / 2.f) + 0.5f;

    return screenSpacePoint;
}

//-----------------------------------------------------------------------------

::Ogre::Vector2 IPicker::getIntersectionInPixel() const
{
    ::Ogre::Vector2 screenSpacePoint = getIntersectionInViewSpace();

    ::Ogre::Camera* cam        = m_sceneManager->getCamera("PlayerCam");
    ::Ogre::Viewport* viewport = cam->getViewport();

    /// We need to round the result to get the right pixel
    screenSpacePoint.x = std::round(screenSpacePoint.x * static_cast< ::Ogre::Real>(viewport->getActualWidth()));
    screenSpacePoint.y = std::round(screenSpacePoint.y * static_cast< ::Ogre::Real>(viewport->getActualHeight()));

    return screenSpacePoint;
}

//-----------------------------------------------------------------------------

void IPicker::setSceneManager(::Ogre::SceneManager* sceneMgr)
{
    m_sceneManager    = sceneMgr;
    m_hasSceneManager = true;
}

//-----------------------------------------------------------------------------

bool IPicker::hasSceneManager()
{
    return m_hasSceneManager;
}


// ----------------------------------------------------------------------------

} // namespace interactor
} // namespace fwRenderOgre
