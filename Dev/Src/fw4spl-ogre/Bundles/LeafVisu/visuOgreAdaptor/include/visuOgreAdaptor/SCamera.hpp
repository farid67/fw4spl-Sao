/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __VISUOGREADAPTOR_SCAMERA_HPP__
#define __VISUOGREADAPTOR_SCAMERA_HPP__

#include <vector>

#include <fwData/TransformationMatrix3D.hpp>

#include <fwCom/Slot.hpp>
#include <fwCom/Slots.hpp>

#include <fwServices/helper/SigSlotConnection.hpp>

#include <fwRenderOgre/IAdaptor.hpp>

#include <OgreMovableObject.h>

#include "visuOgreAdaptor/config.hpp"

fwCorePredeclare( (arData)(Camera) )

namespace visuOgreAdaptor
{

/**
 * @brief   Adaptor from fw4 Camera to Ogre Camera
 * @class   Camera
 */
class VISUOGREADAPTOR_CLASS_API SCamera : public ::fwRenderOgre::IAdaptor
{

public:
    fwCoreServiceClassDefinitionsMacro((SCamera)(::fwRenderOgre::IAdaptor));

    /// Constructor.
    VISUOGREADAPTOR_API SCamera() throw();

    /// Destructor. Does nothing
    VISUOGREADAPTOR_API virtual ~SCamera() throw();

    VISUOGREADAPTOR_API static const ::fwCom::Slots::SlotKeyType s_CALIBRATE_SLOT;

    /// Get camera
    ::Ogre::Camera* getCurrentCamera();

    /// Updates Transformation Matrix
    VISUOGREADAPTOR_API void updateTF3D();

    /// Returns proposals to connect service slots to associated object signals
    ::fwServices::IService::KeyConnectionsType getObjSrvConnections() const;

protected:
    /// Starting method. Do nothing
    VISUOGREADAPTOR_API void doStart() throw(fwTools::Failed);

    /// Stopping method
    VISUOGREADAPTOR_API void doStop() throw(fwTools::Failed);

    /**
     * @brief Configuring method.
     *
     * @verbatim
         <adaptor id="cameraAdaptor" class="::visuOgreAdaptor::Camera" objectId="cameraTF" />
       @endverbatim
     */
    VISUOGREADAPTOR_API void doConfigure() throw(fwTools::Failed);

    /// Swaping method, only asks for a doUpdate
    VISUOGREADAPTOR_API void doSwap() throw(fwTools::Failed);

    /// Update the Camera position and orientation
    VISUOGREADAPTOR_API void doUpdate() throw(fwTools::Failed);

private:

    /// Calibrate the camera parameters according to an arData::Camera
    void calibrate();

    /// uid of the camera
    std::string m_cameraUID;

    /// Transformation Matrix
    ::fwData::TransformationMatrix3D::sptr m_transMat;

    /// camera used to calibrate ogre camera
    SPTR(::arData::Camera) m_camera;
};

} //namespace visuOgreAdaptor

#endif // __VISUOGREADAPTOR_SCAMERA_HPP__
