/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "visuOgreAdaptor/SVideo.hpp"

#include <arData/Camera.hpp>

#include <fwCom/Signal.hxx>

#include <fwData/mt/ObjectReadLock.hpp>
#include <fwData/Point.hpp>

#include <fwServices/Base.hpp>

#include <fwRenderOgre/Utils.hpp>

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreMaterialManager.h>
#include <OGRE/OgreMeshManager.h>
#include <OGRE/OgreMovablePlane.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgreTextureManager.h>

namespace visuOgreAdaptor
{

fwServicesRegisterMacro( ::fwRenderOgre::IAdaptor, ::visuOgreAdaptor::SVideo, ::fwData::Image);

static const char* VIDEO_MATERIAL_NAME = "Video";

//------------------------------------------------------------------------------
SVideo::SVideo() throw() :
    m_imageData(nullptr),
    m_isTextureInit(false),
    m_previousWidth(0),
    m_previousHeight(0),
    m_reverse(false)
{
}

//------------------------------------------------------------------------------

SVideo::~SVideo() throw()
{
}

//------------------------------------------------------------------------------

void SVideo::doStart() throw(::fwTools::Failed)
{
    m_texture = ::Ogre::TextureManager::getSingletonPtr()->create(
        this->getID() + "_VideoTexture",
        ::Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        true);

    // Duplicate default material to create Video material
    ::Ogre::MaterialPtr defaultMat = ::Ogre::MaterialManager::getSingleton().getByName(VIDEO_MATERIAL_NAME);

    ::Ogre::MaterialPtr texMat = ::Ogre::MaterialManager::getSingleton().create( this->getID() + "_VideoMaterial",
                                                                                 ::Ogre::ResourceGroupManager::
                                                                                 DEFAULT_RESOURCE_GROUP_NAME);

    defaultMat->copyDetailsTo(texMat);

    // Set the texture to the main material pass
    ::Ogre::Pass* ogrePass = texMat->getTechnique(0)->getPass(0);
    ogrePass->createTextureUnitState(this->getID() + "_VideoTexture");
}

//------------------------------------------------------------------------------

void SVideo::doStop() throw(::fwTools::Failed)
{
    this->unregisterServices();
}

//------------------------------------------------------------------------------

void SVideo::doSwap() throw(::fwTools::Failed)
{
    this->doUpdate();
}

//------------------------------------------------------------------------------

void SVideo::doConfigure() throw(::fwTools::Failed)
{
    SLM_ASSERT("Not a \"config\" configuration", m_configuration->getName() == "config");

    m_cameraUID = m_configuration->getAttributeValue("cameraUID");

    std::string reverse = m_configuration->getAttributeValue("reverse");
    if (!reverse.empty() )
    {
        if( reverse=="true" )
        {
            m_reverse = true;
        }
    }
}

//------------------------------------------------------------------------------

void SVideo::doUpdate() throw(::fwTools::Failed)
{
    this->getRenderService()->makeCurrent();

    // Getting FW4SPL Image
    ::fwData::Image::sptr imageFw4s = ::fwData::Image::dynamicCast(this->getObject());
    SLM_ASSERT("Problem getting the image",imageFw4s);

    ::fwData::Image::SizeType size       = imageFw4s->getSize();
    ::fwData::Image::SpacingType spacing = imageFw4s->getSpacing();

    ::fwData::mt::ObjectReadLock lock(imageFw4s);
    ::Ogre::Image imageOgre = ::fwRenderOgre::Utils::convertFwDataImageToOgreImage( imageFw4s );

    if (!m_isTextureInit || imageOgre.getWidth() != m_previousWidth || imageOgre.getHeight() != m_previousHeight )
    {
        std::string thisID        = this->getID();
        std::string videoMeshName = thisID + "_VideoMesh";
        std::string entityName    = thisID + "_VideoEntity";
        std::string nodeName      = thisID + "_VideoSceneNode";

        m_texture->freeInternalResources();

        m_texture->setWidth(imageOgre.getWidth());
        m_texture->setHeight(imageOgre.getHeight());
        m_texture->setTextureType(::Ogre::TEX_TYPE_2D);
        m_texture->setFormat(::Ogre::PF_X8R8G8B8);
        m_texture->setDepth(1);
        m_texture->setNumMipmaps(0);
        m_texture->setUsage(::Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

        m_texture->createInternalResources();

        // Create Ogre Plane
        ::Ogre::MovablePlane plane( ::Ogre::Vector3::UNIT_Z, 0 );

        ::Ogre::SceneManager* sceneManager = this->getSceneManager();
        ::Ogre::MeshManager& meshManager   = ::Ogre::MeshManager::getSingleton();
        /// Delete deprecated Ogre resources if necessary
        if (meshManager.resourceExists(videoMeshName))
        {
            ::Ogre::ResourcePtr mesh = meshManager.getResourceByName(videoMeshName);
            meshManager.remove(mesh);
            sceneManager->destroyEntity(entityName);
            sceneManager->getRootSceneNode()->removeAndDestroyChild(nodeName);
        }

        meshManager.createPlane(videoMeshName,
                                ::Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                plane,
                                static_cast< ::Ogre::Real >(size[0]) * static_cast< ::Ogre::Real >(spacing[0]),
                                static_cast< ::Ogre::Real >(size[1]) * static_cast< ::Ogre::Real >(spacing[1]));

        // Create Ogre Entity
        ::Ogre::Entity* ent = sceneManager->createEntity(entityName,videoMeshName);

        ent->setMaterialName(thisID + "_VideoMaterial");

        // Add the entity to the scene
        ::Ogre::SceneNode* sn = sceneManager->getRootSceneNode()->createChildSceneNode(nodeName);
        sn->attachObject(ent);
        sn->setPosition(0,0,0);

        ::Ogre::Camera* cam = sceneManager->getCamera("PlayerCam");
        cam->setProjectionType(::Ogre::PT_ORTHOGRAPHIC);
        cam->setOrthoWindowHeight(static_cast< ::Ogre::Real >(size[1]) * static_cast< ::Ogre::Real >(spacing[1]));

        m_isTextureInit = true;

        if(!m_cameraUID.empty())
        {
            ::fwTools::Object::sptr obj   = ::fwTools::fwID::getObject(m_cameraUID);
            ::arData::Camera::sptr camera = ::arData::Camera::dynamicCast(obj);
            SLM_ASSERT("Missing camera", camera);

            float shiftX = static_cast<float>(size[0] ) / 2.f - static_cast<float>(camera->getCx());
            float shiftY = static_cast<float>(size[1] ) / 2.f - static_cast<float>(camera->getCy());

            if (m_reverse)
            {
                cam->setPosition(shiftX, -shiftY, 0);
            }
            else
            {
                cam->setPosition(-shiftX, shiftY, 0);
            }
        }
    }


    m_texture->getBuffer(0,0)->blitFromMemory(imageOgre.getPixelBox(0,0));

    lock.unlock();

    m_previousWidth  = imageOgre.getWidth();
    m_previousHeight = imageOgre.getHeight();

    this->requestRender();
}

//-----------------------------------------------------------------------------

::fwServices::IService::KeyConnectionsType SVideo::getObjSrvConnections() const
{
    ::fwServices::IService::KeyConnectionsType connections;
    connections.push_back( std::make_pair( ::fwData::Image::s_BUFFER_MODIFIED_SIG, s_UPDATE_SLOT ) );
    connections.push_back( std::make_pair( ::fwData::Image::s_MODIFIED_SIG, s_UPDATE_SLOT ) );
    return connections;
}


//------------------------------------------------------------------------------

} // namespace visuOgreAdaptor
