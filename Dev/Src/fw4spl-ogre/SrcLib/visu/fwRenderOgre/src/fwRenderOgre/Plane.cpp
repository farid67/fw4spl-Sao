/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "fwRenderOgre/Plane.hpp"

#include <fwRenderOgre/Utils.hpp>

#include <OGRE/OgreEntity.h>
#include <OGRE/OgreMeshManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreMaterialManager.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreMovablePlane.h>

#include <regex>

namespace fwRenderOgre
{

//-----------------------------------------------------------------------------

static bool isNegatoPass(const std::string _name, bool& _peelPass)
{
    const std::regex regexPeel(".*_peel.*");
    const std::regex regexWeight(".*_weight_blend.*");
    const std::regex regexDualPeelInit("Dual.*_peel_init.*");

    _peelPass = std::regex_match(_name, regexPeel);
    const bool weightPass   = std::regex_match(_name, regexWeight);
    const bool peelInitPass = std::regex_match(_name, regexDualPeelInit);

    return _name == "" || (_peelPass && !peelInitPass) || weightPass;
}

//-----------------------------------------------------------------------------

Plane::Plane( const ::fwTools::fwID::IDType& _negatoId, ::Ogre::SceneNode* _parentSceneNode,
              ::Ogre::SceneManager* _sceneManager, OrientationMode _orientation, bool _is3D,
              ::Ogre::TexturePtr _tex, float _entityOpacity) :
    m_is3D( _is3D ),
    m_threshold(false),
    m_orientation( _orientation ),
    m_originPosition(0.f, 0.f, 0.f),
    m_texture( _tex ),
    m_sceneManager ( _sceneManager ),
    m_parentSceneNode( _parentSceneNode ),
    m_width(0.f),
    m_height(0.f),
    m_depth(0.f),
    m_relativePosition(0.8f),
    m_entityOpacity( _entityOpacity )
{
    // names definition
    switch(m_orientation)
    {
        case OrientationMode::X_AXIS:
            m_slicePlaneName = _negatoId + "_Sagittal_Mesh";
            m_entityName     = _negatoId + "_Sagittal_Entity";
            m_sceneNodeName  = _negatoId + "_Sagittal_SceneNode";
            break;
        case OrientationMode::Y_AXIS:
            m_slicePlaneName = _negatoId + "_Frontal_Mesh";
            m_entityName     = _negatoId + "_Frontal_Entity";
            m_sceneNodeName  = _negatoId + "_Frontal_SceneNode";
            break;
        case OrientationMode::Z_AXIS:
            m_slicePlaneName = _negatoId + "_Axial_Mesh";
            m_entityName     = _negatoId + "_Axial_Entity";
            m_sceneNodeName  = _negatoId + "_Axial_SceneNode";
            break;
    }

    // Creates the parent's child scene node positionned at (0; 0; 0)
    m_planeSceneNode = m_parentSceneNode->createChildSceneNode( m_sceneNodeName );
    this->initializeMaterial();
}

//-----------------------------------------------------------------------------

Plane::~Plane()
{
}

//-----------------------------------------------------------------------------

void Plane::initializeMaterial()
{
    ::Ogre::MaterialPtr defaultMat = m_is3D ? ::Ogre::MaterialManager::getSingleton().getByName("Negato3D")
                                     : ::Ogre::MaterialManager::getSingleton().getByName("Negato2D");

    m_texMaterial = ::Ogre::MaterialManager::getSingleton().create(
        "NegatoMat",
        ::Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    defaultMat->copyDetailsTo(m_texMaterial);
    m_texMaterial->setCullingMode(::Ogre::CULL_NONE);
    m_texMaterial->setTextureFiltering(::Ogre::TFO_NONE);

    ::Ogre::ColourValue diffuse(1.f, 1.f, 1.f, m_entityOpacity);
    m_texMaterial->setDiffuse(diffuse);

    int orientationIndex;

    switch (m_orientation)
    {
        case OrientationMode::X_AXIS:
            orientationIndex = 0;
            break;
        case OrientationMode::Y_AXIS:
            orientationIndex = 1;
            break;
        case OrientationMode::Z_AXIS:
            orientationIndex = 2;
            break;
    }

    // This is necessary to load and compile the material, otherwise the following technique iterator
    // is null when we call this method on the first time (from doStart() for instance)
    m_texMaterial->touch();

    ::Ogre::Material::TechniqueIterator tech_iter = m_texMaterial->getSupportedTechniqueIterator();

    while( tech_iter.hasMoreElements())
    {
        ::Ogre::Technique* tech = tech_iter.getNext();

        bool peelPass = false;
        if(isNegatoPass(tech->getName(), peelPass))
        {
            ::Ogre::Pass* pass = tech->getPass(0);

            ::Ogre::TextureUnitState* texState = pass->createTextureUnitState();
            texState->setTexture(m_texture);
            texState->setTextureFiltering(::Ogre::TFO_NONE);
            texState->setTextureAddressingMode(::Ogre::TextureUnitState::TAM_CLAMP);

            pass->getVertexProgramParameters()->setNamedConstant("u_orientation", orientationIndex);
            pass->getFragmentProgramParameters()->setNamedConstant("u_orientation", orientationIndex);
        }
    }
}

//-----------------------------------------------------------------------------

void Plane::setDepthSpacing(std::vector<double> _spacing)
{
    m_spacing.clear();

    for (auto spacingValue : _spacing)
    {
        m_spacing.push_back(static_cast< ::Ogre::Real >(spacingValue));
    }
}

//-----------------------------------------------------------------------------

void Plane::initialize3DPlane()
{
    ::Ogre::MeshManager* meshManager = ::Ogre::MeshManager::getSingletonPtr();

    // First delete mesh if it already exists
    if(meshManager->resourceExists(m_slicePlaneName))
    {
        meshManager->remove(m_slicePlaneName);
    }

    if( m_sceneManager->hasEntity(m_entityName))
    {
        m_sceneManager->getEntity( m_entityName)->detachFromParent();
        m_sceneManager->destroyEntity(m_entityName);
    }

    ::Ogre::MovablePlane* plane = setDimensions();

    // Mesh plane instanciation:
    // Y is the default upVector,
    // so if we want a plane which normal is the Y unit vector we have to create it differently
    if(m_orientation == ::fwComEd::helper::MedicalImageAdaptor::Orientation::Y_AXIS)
    {
        m_slicePlane = ::Ogre::MeshManager::getSingletonPtr()->createPlane(
            m_slicePlaneName,
            ::Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            *plane,
            m_width, m_height,
            1, 1,
            true,
            1, 1.0, 1.0,
            ::Ogre::Vector3::UNIT_Z);
    }
    else
    {
        m_slicePlane = ::Ogre::MeshManager::getSingletonPtr()->createPlane(
            m_slicePlaneName,
            ::Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            *plane,
            m_width, m_height);
    }

    // Entity creation
    ::Ogre::Entity *planeEntity = m_sceneManager->createEntity(m_entityName, m_slicePlane);
    planeEntity->setMaterial(m_texMaterial);
    m_planeSceneNode->attachObject(planeEntity);

    this->initializePosition();
}

//-----------------------------------------------------------------------------

void Plane::initialize2DPlane()
{
    ::Ogre::MeshManager* meshManager = ::Ogre::MeshManager::getSingletonPtr();

    // First delete mesh if it already exists
    if(meshManager->resourceExists(m_slicePlaneName))
    {
        meshManager->remove(m_slicePlaneName);
    }

    if( m_sceneManager->hasEntity(m_entityName))
    {
        m_sceneManager->getEntity( m_entityName)->detachFromParent();
        m_sceneManager->destroyEntity(m_entityName);
    }

    ::Ogre::MovablePlane* plane = setDimensions();

    // Mesh plane instanciation
    m_slicePlane = ::Ogre::MeshManager::getSingletonPtr()->createPlane(
        m_slicePlaneName,
        ::Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        *plane,
        m_width, m_height);

    // Entity creation
    ::Ogre::Entity *planeEntity = m_sceneManager->createEntity(m_entityName, m_slicePlane);
    planeEntity->setMaterial(m_texMaterial);
    m_planeSceneNode->attachObject(planeEntity);
}

//-----------------------------------------------------------------------------

void Plane::initializePosition()
{
    this->moveToOriginPosition();

    if(m_is3D)
    {
        switch(m_orientation)
        {
            case OrientationMode::X_AXIS:
                m_planeSceneNode->translate(0, m_height / 2, m_width / 2);
                break;
            case OrientationMode::Y_AXIS:
                m_planeSceneNode->translate(m_width / 2, 0, m_height / 2);
                break;
            case OrientationMode::Z_AXIS:
                m_planeSceneNode->translate(m_width / 2, m_height / 2, 0);
                break;
        }
    }
}

//-----------------------------------------------------------------------------

void Plane::moveAlongAxis()
{
    SLM_ASSERT("2D Plane, cannot move along its normal.", m_is3D);

    this->initializePosition();
    ::Ogre::Real distance = m_relativePosition * m_depth;


    switch(m_orientation)
    {
        case OrientationMode::X_AXIS:
            m_planeSceneNode->translate(distance, 0, 0);
            break;
        case OrientationMode::Y_AXIS:
            m_planeSceneNode->translate(0, distance, 0);
            break;
        case OrientationMode::Z_AXIS:
            m_planeSceneNode->translate(0, 0, distance);
            break;
    }
}

//-----------------------------------------------------------------------------

void Plane::setRelativePosition(float _relativePosition)
{
    if( _relativePosition <= 0 )
    {
        m_relativePosition = 0;
    }
    else if ( _relativePosition >= 1)
    {
        // as close as possible from 1, but smaller.
        m_relativePosition = 0.999999999999999f;
    }
    else
    {
        m_relativePosition = _relativePosition;
    }
}

//-----------------------------------------------------------------------------

void Plane::setWindowing(float _minValue, float _maxValue)
{
    ::Ogre::Material::TechniqueIterator tech_iter = m_texMaterial->getSupportedTechniqueIterator();

    while( tech_iter.hasMoreElements())
    {
        ::Ogre::Technique* tech = tech_iter.getNext();

        bool peelPass = false;
        if(isNegatoPass(tech->getName(), peelPass))
        {
            ::Ogre::Pass* pass = tech->getPass(0);
            SLM_ASSERT("Can't find Ogre pass", pass);

            pass->getFragmentProgramParameters()->setNamedConstant("u_minValue", _minValue);
            pass->getFragmentProgramParameters()->setNamedConstant("u_maxValue", _maxValue);
        }
    }
}

//-----------------------------------------------------------------------------

void Plane::switchThresholding(bool _threshold)
{
    ::Ogre::Material::TechniqueIterator tech_iter = m_texMaterial->getSupportedTechniqueIterator();

    while( tech_iter.hasMoreElements())
    {
        ::Ogre::Technique* tech = tech_iter.getNext();

        bool peelPass = false;
        if(isNegatoPass(tech->getName(), peelPass))
        {
            ::Ogre::Pass* pass = tech->getPass(0);

            SLM_ASSERT("Can't find Ogre pass", pass);

            m_threshold = _threshold;
            pass->getFragmentProgramParameters()->setNamedConstant("u_threshold", static_cast<int>(m_threshold));
        }
    }
}

//-----------------------------------------------------------------------------

void Plane::moveToOriginPosition()
{
    m_planeSceneNode->setPosition(m_originPosition);
}

//------------------------------------------------------------------------------

double Plane::getSliceWorldPosition() const
{
    ::Ogre::Real position;

    switch(m_orientation)
    {
        case OrientationMode::X_AXIS:
            position = m_planeSceneNode->getPosition().x;
            break;
        case OrientationMode::Y_AXIS:
            position = m_planeSceneNode->getPosition().y;
            break;
        case OrientationMode::Z_AXIS:
            position = m_planeSceneNode->getPosition().z;
            break;
    }

    return static_cast<double>(position);
}

//------------------------------------------------------------------------------

void Plane::setOrientationMode(OrientationMode _newMode)
{
    SLM_ASSERT("3D negato Plane, cannot change orientation", !m_is3D);

    m_orientation = _newMode;
    this->initializeMaterial();
    this->initialize2DPlane();
}

//------------------------------------------------------------------------------

void Plane::setEntityOpacity(float _f)
{
    SLM_ASSERT("2D negato Plane, cannot change opacity", m_is3D);

    m_entityOpacity = _f;

    ::Ogre::ColourValue diffuse(1.f, 1.f, 1.f, m_entityOpacity);
    m_texMaterial->setDiffuse(diffuse);

    ::Ogre::Material::TechniqueIterator tech_iter = m_texMaterial->getSupportedTechniqueIterator();

    while( tech_iter.hasMoreElements())
    {
        ::Ogre::Technique* tech = tech_iter.getNext();

        bool peelPass = false;
        if(isNegatoPass(tech->getName(), peelPass) && !peelPass)
        {
            ::Ogre::Pass* pass = tech->getPass(0);

            // We don't want a depth check if we have non-OIT transparency
            bool needDepthCheck = (m_entityOpacity == 1.f);
            pass->setDepthCheckEnabled(needDepthCheck);
        }
    }
}

//------------------------------------------------------------------------------

void Plane::changeSlice(float sliceIndex)
{
    ::Ogre::Material::TechniqueIterator tech_iter = m_texMaterial->getSupportedTechniqueIterator();

    while( tech_iter.hasMoreElements())
    {
        ::Ogre::Technique* tech = tech_iter.getNext();

        bool peelPass = false;
        if(isNegatoPass(tech->getName(), peelPass))
        {
            ::Ogre::Pass* pass = tech->getPass(0);

            SLM_ASSERT("Can't find Ogre pass", pass);

            pass->getFragmentProgramParameters()->setNamedConstant("u_slice", sliceIndex );
        }
    }

    if (m_is3D)
    {
        this->setRelativePosition( sliceIndex );
        this->moveAlongAxis();
    }
}

//-----------------------------------------------------------------------------

::Ogre::MovablePlane* Plane::setDimensions()
{
    ::Ogre::Real tex_width  = static_cast< ::Ogre::Real >( m_texture->getWidth() );
    ::Ogre::Real tex_height = static_cast< ::Ogre::Real >( m_texture->getHeight() );
    ::Ogre::Real tex_depth  = static_cast< ::Ogre::Real >( m_texture->getDepth() );

    ::Ogre::MovablePlane* plane;
    switch(m_orientation)
    {
        case OrientationMode::X_AXIS:
            m_width  = tex_depth * m_spacing[2];
            m_height = tex_height * m_spacing[1];
            m_depth  = tex_width * m_spacing[0];
            break;
        case OrientationMode::Y_AXIS:
            m_width  = tex_width * m_spacing[0];
            m_height = tex_depth * m_spacing[2];
            m_depth  = tex_height * m_spacing[1];
            break;
        case OrientationMode::Z_AXIS:
            m_width  = tex_width * m_spacing[0];
            m_height = tex_height * m_spacing[1];
            m_depth  = tex_depth * m_spacing[2];
            break;
    }

    if(m_is3D)
    {
        switch(m_orientation)
        {
            case OrientationMode::X_AXIS:
                plane = new ::Ogre::MovablePlane(::Ogre::Vector3::UNIT_X, 0);
                break;
            case OrientationMode::Y_AXIS:
                plane = new ::Ogre::MovablePlane(::Ogre::Vector3::UNIT_Y, 0);
                break;
            case OrientationMode::Z_AXIS:
                plane = new ::Ogre::MovablePlane(::Ogre::Vector3::UNIT_Z, 0);
                break;
        }
    }
    else
    {
        plane = new ::Ogre::MovablePlane(::Ogre::Vector3::UNIT_Z, 0);
        // It's more convenient to display the Negato2D's sagittal plane horizontally.
        if (m_orientation == OrientationMode::X_AXIS)
        {
            m_height = (tex_depth -1) * m_spacing[2];
            m_width  = (tex_height -1) * m_spacing[1];
            m_depth  = (tex_width -1) * m_spacing[0];
        }
    }

    return plane;
}

//-----------------------------------------------------------------------------

void Plane::removeAndDestroyPlane()
{
    m_parentSceneNode->removeAndDestroyChild(m_sceneNodeName);
}

//-----------------------------------------------------------------------------
} // namespace fwRenderOgre
