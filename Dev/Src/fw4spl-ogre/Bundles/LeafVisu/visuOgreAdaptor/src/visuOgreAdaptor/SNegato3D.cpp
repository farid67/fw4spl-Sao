/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "visuOgreAdaptor/SNegato3D.hpp"

#include <fwComEd/fieldHelper/MedicalImageHelpers.hpp>

#include <fwCom/Signal.hxx>
#include <fwCom/Slot.hxx>
#include <fwCom/Slots.hxx>

#include <fwComEd/Dictionary.hpp>

#include <fwData/Boolean.hpp>
#include <fwData/Image.hpp>
#include <fwData/Integer.hpp>

#include <fwRenderOgre/Utils.hpp>
#include <fwRenderOgre/Plane.hpp>

#include <fwServices/Base.hpp>
#include <fwServices/macros.hpp>

#include <algorithm>

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreTextureManager.h>

namespace visuOgreAdaptor
{

fwServicesRegisterMacro( ::fwRenderOgre::IAdaptor, ::visuOgreAdaptor::SNegato3D, ::fwData::Image);

const ::fwCom::Slots::SlotKeyType SNegato3D::s_NEWIMAGE_SLOT          = "newImage";
const ::fwCom::Slots::SlotKeyType SNegato3D::s_SLICETYPE_SLOT         = "sliceType";
const ::fwCom::Slots::SlotKeyType SNegato3D::s_SLICEINDEX_SLOT        = "sliceIndex";
const ::fwCom::Slots::SlotKeyType SNegato3D::s_UPDATE_OPACITY_SLOT    = "updateOpacity";
const ::fwCom::Slots::SlotKeyType SNegato3D::s_UPDATE_VISIBILITY_SLOT = "updateVisibility";

//------------------------------------------------------------------------------

SNegato3D::SNegato3D() throw() :
    m_autoResetCamera(true),
    m_activePlane(nullptr),
    m_negatoSceneNode(nullptr),
    m_interpolation(true)
{
    this->installTFSlots(this);

    newSlot(s_NEWIMAGE_SLOT, &SNegato3D::newImage, this);
    newSlot(s_SLICETYPE_SLOT, &SNegato3D::changeSliceType, this);
    newSlot(s_SLICEINDEX_SLOT, &SNegato3D::changeSliceIndex, this);
    newSlot(s_UPDATE_OPACITY_SLOT, &SNegato3D::setPlanesOpacity, this);
    newSlot(s_UPDATE_VISIBILITY_SLOT, &SNegato3D::setPlanesOpacity, this);
}

//------------------------------------------------------------------------------

SNegato3D::~SNegato3D() throw()
{
}

//------------------------------------------------------------------------------

void SNegato3D::doStart() throw(::fwTools::Failed)
{
    this->updateImageInfos(this->getObject< ::fwData::Image >());
    this->updateTransferFunction(this->getImage());

    // Texture instantiation
    m_3DOgreTexture = ::Ogre::TextureManager::getSingletonPtr()->create(
        this->getID() + "_Texture",
        ::Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        true);

    // Scene node's instantiation
    m_negatoSceneNode = this->getSceneManager()->getRootSceneNode()->createChildSceneNode();

    // Instanciation of the planes
    m_planes[0] = new ::fwRenderOgre::Plane(this->getID(), m_negatoSceneNode,
                                            getSceneManager(),
                                            OrientationMode::X_AXIS, true, m_3DOgreTexture);
    m_planes[1] = new ::fwRenderOgre::Plane(this->getID(), m_negatoSceneNode,
                                            getSceneManager(),
                                            OrientationMode::Y_AXIS, true, m_3DOgreTexture);
    m_planes[2] = new ::fwRenderOgre::Plane(this->getID(), m_negatoSceneNode,
                                            getSceneManager(),
                                            OrientationMode::Z_AXIS, true, m_3DOgreTexture);

    this->setPlanesOpacity();

    // Default active plane = sagittal plane.
    m_activePlane = m_planes[0];

    if (m_autoResetCamera)
    {
        this->getRenderService()->resetCameraCoordinates(m_layerID);
    }

    this->installTFConnections();

    bool isValid = ::fwComEd::fieldHelper::MedicalImageHelpers::checkImageValidity(this->getImage());
    if (isValid)
    {
        this->newImage();
    }
}

//------------------------------------------------------------------------------

void SNegato3D::doStop() throw(::fwTools::Failed)
{
    this->removeTFConnections();

    m_planes[0]->removeAndDestroyPlane();
    m_planes[1]->removeAndDestroyPlane();
    m_planes[2]->removeAndDestroyPlane();

    delete m_planes[0];
    delete m_planes[1];
    delete m_planes[2];

    this->requestRender();
}

//------------------------------------------------------------------------------

void SNegato3D::doConfigure() throw(::fwTools::Failed)
{
    SLM_ASSERT("No config tag", m_configuration->getName() == "config");

    // Axis orientation mode by default
    m_orientation = OrientationMode::Z_AXIS;

    if(m_configuration->hasAttribute("sliceIndex"))
    {
        std::string orientation = m_configuration->getAttributeValue("sliceIndex");

        if(orientation == "axial")
        {
            m_orientation = Z_AXIS;
        }
        else if(orientation == "frontal")
        {
            m_orientation = Y_AXIS;
        }
        else if(orientation == "sagittal")
        {
            m_orientation = X_AXIS;
        }
    }
    if(m_configuration->hasAttribute("autoresetcamera"))
    {
        std::string autoResetCamera = m_configuration->getAttributeValue("autoresetcamera");
        m_autoResetCamera = (autoResetCamera == "yes");
    }
    if(m_configuration->hasAttribute("transform"))
    {
        this->setTransformUID(m_configuration->getAttributeValue("tranform"));
    }
    if(m_configuration->hasAttribute("interpolation"))
    {
        // Whatever you specify, yes by default, needs explicit "no" to remove interpolation.
        this->setInterpolation(!(m_configuration->getAttributeValue("interpolation") == "no"));
    }

    this->parseTFConfig(m_configuration);
}

//------------------------------------------------------------------------------

void SNegato3D::doUpdate() throw(::fwTools::Failed)
{

    this->requestRender();
}

//------------------------------------------------------------------------------

void SNegato3D::doSwap() throw(::fwTools::Failed)
{
    this->doStop();
    this->doStart();
    this->doUpdate();
}

//------------------------------------------------------------------------------

void SNegato3D::createPlanes(const ::fwData::Image::SpacingType& _spacing, const ::fwData::Image::OriginType& _origin)
{
    ::Ogre::Vector3 origin(static_cast< ::Ogre::Real >(_origin[0]),
                           static_cast< ::Ogre::Real >(_origin[1]),
                           static_cast< ::Ogre::Real >(_origin[2]));

    // Fits the planes to the new texture
    for(int i(0); i < 3; ++i)
    {
        m_planes[i]->setDepthSpacing(_spacing);
        m_planes[i]->setOriginPosition(origin);
        m_planes[i]->initialize3DPlane();
    }
}

//------------------------------------------------------------------------------

void SNegato3D::newImage()
{
    this->getRenderService()->makeCurrent();

    ::fwData::Image::sptr image = this->getImage();

    // Retrieves or creates the slice index fields
    this->updateImageInfos(image);

    ::fwRenderOgre::Utils::convertImageForNegato(m_3DOgreTexture.get(), image);

    createPlanes(image->getSpacing(), image->getOrigin());

    // Update Slice
    this->changeSliceIndex(m_axialIndex->value(), m_frontalIndex->value(), m_sagittalIndex->value());

    // Update TF
    this->updatingTFWindowing(this->getTransferFunction()->getWindow(), this->getTransferFunction()->getLevel());

    // Update threshold if necessary
    this->updatingTFPoints();

    if (m_autoResetCamera)
    {
        this->getRenderService()->resetCameraCoordinates(m_layerID);
    }
    this->requestRender();
}

//------------------------------------------------------------------------------

void SNegato3D::changeSliceType(int _from, int _to)
{
    // We have to update the active plane
    m_activePlane = m_planes[_to];

    this->getRenderService()->makeCurrent();

    // Update TF
    this->updatingTFWindowing(this->getTransferFunction()->getWindow(), this->getTransferFunction()->getLevel());

    // Update threshold if necessary
    this->updatingTFPoints();

    this->requestRender();
}

//------------------------------------------------------------------------------

void SNegato3D::changeSliceIndex(int _axialIndex, int _frontalIndex, int _sagittalIndex)
{
    ::fwData::Image::sptr image = this->getImage();

    float sliceIndex[3] = {
        static_cast<float>(_sagittalIndex ) / (static_cast<float>(image->getSize()[0] - 1)),
        static_cast<float>(_frontalIndex  ) / (static_cast<float>(image->getSize()[1] - 1)),
        static_cast<float>(_axialIndex    ) / (static_cast<float>(image->getSize()[2] - 1))
    };

    for (int i = 0; i < 3; ++i)
    {
        m_planes[i]->changeSlice( sliceIndex[i] );
    }

    this->requestRender();
}

//-----------------------------------------------------------------------------

::fwServices::IService::KeyConnectionsType SNegato3D::getObjSrvConnections() const
{
    ::fwServices::IService::KeyConnectionsType connections;
    connections.push_back( std::make_pair( ::fwData::Image::s_MODIFIED_SIG, s_NEWIMAGE_SLOT ) );
    connections.push_back( std::make_pair( ::fwData::Image::s_BUFFER_MODIFIED_SIG, s_NEWIMAGE_SLOT ) );
    connections.push_back( std::make_pair( ::fwData::Image::s_SLICE_TYPE_MODIFIED_SIG, s_SLICETYPE_SLOT ) );
    connections.push_back( std::make_pair( ::fwData::Image::s_SLICE_INDEX_MODIFIED_SIG, s_SLICEINDEX_SLOT ) );
    connections.push_back( std::make_pair( ::fwData::Image::s_TRANSPARENCY_MODIFIED_SIG, s_UPDATE_OPACITY_SLOT ) );
    connections.push_back( std::make_pair( ::fwData::Image::s_VISIBILITY_MODIFIED_SIG, s_UPDATE_VISIBILITY_SLOT ) );
    return connections;
}

//-----------------------------------------------------------------------------

void SNegato3D::updatingTFPoints()
{
    m_planes[0]->switchThresholding(this->getTransferFunction()->getIsClamped());
    m_planes[1]->switchThresholding(this->getTransferFunction()->getIsClamped());
    m_planes[2]->switchThresholding(this->getTransferFunction()->getIsClamped());

    this->requestRender();
}

//-----------------------------------------------------------------------------

void SNegato3D::updatingTFWindowing(double window, double level)
{
    float minVal = static_cast<float>(level) - static_cast<float>(window) / 2.f;
    float maxVal = static_cast<float>(level) + static_cast<float>(window) / 2.f;
    m_planes[0]->setWindowing(minVal, maxVal);
    m_planes[1]->setWindowing(minVal, maxVal);
    m_planes[2]->setWindowing(minVal, maxVal);

    this->requestRender();
}

//-----------------------------------------------------------------------------

void SNegato3D::setPlanesOpacity()
{
    ::fwData::Image::sptr image = this->getImage();

    const std::string TRANSPARENCY_FIELD = "TRANSPARENCY";
    const std::string VISIBILITY_FIELD   = "VISIBILITY";

    ::fwData::Integer::sptr transparency = image->setDefaultField(TRANSPARENCY_FIELD, ::fwData::Integer::New(0));
    ::fwData::Boolean::sptr isVisible    = image->setDefaultField(VISIBILITY_FIELD, ::fwData::Boolean::New(true));

    float opacity = isVisible->getValue() ? (100.f - transparency->getValue())/100.f : 0.f;

    if(m_planes[0] && m_planes[1] && m_planes[2])
    {
        m_planes[0]->setEntityOpacity(opacity);
        m_planes[1]->setEntityOpacity(opacity);
        m_planes[2]->setEntityOpacity(opacity);
    }

    this->requestRender();
}

//------------------------------------------------------------------------------

} // namespace visuOgreAdaptor
