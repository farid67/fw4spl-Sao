/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __VISUOGREADAPTOR_STRANSFORM_HPP__
#define __VISUOGREADAPTOR_STRANSFORM_HPP__

#include <fwRenderOgre/IAdaptor.hpp>
#include <fwRenderOgre/ITransformable.hpp>
#include <fwRenderOgre/SRender.hpp>

#include <OGRE/OgreMovableObject.h>
#include <OGRE/OgreMatrix4.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>

#include <boost/shared_ptr.hpp>

#include <vector>

#include "visuOgreAdaptor/config.hpp"

namespace visuOgreAdaptor
{

class VISUOGREADAPTOR_CLASS_API STransform : public ::fwRenderOgre::IAdaptor,
                                             public ::fwRenderOgre::ITransformable
{

public:
    fwCoreServiceClassDefinitionsMacro((STransform)(::fwRenderOgre::IAdaptor));

    /// Constructor,
    VISUOGREADAPTOR_API STransform() throw();
    /// Destructor, does nothing
    VISUOGREADAPTOR_API virtual ~STransform() throw();

    /**
     * @brief Get Ogre transform matrix
     */
    VISUOGREADAPTOR_API const ::Ogre::Matrix4& getTransform() const;
    /// Sets the Transformation Matrix to the ::Ogre::Matrix t, then updates it in F4S by copy from Ogre
    VISUOGREADAPTOR_API void setTransform(const ::Ogre::Matrix4& t);

    /**
     * @brief Ogre transform sceneNode.
     */
    VISUOGREADAPTOR_API ::Ogre::SceneNode* getSceneNode();
    /// Copies the transformation matrix from Ogre to F4S
    VISUOGREADAPTOR_API void updateFromOgre();

    /// Returns proposals to connect service slots to associated object signals
    ::fwServices::IService::KeyConnectionsType getObjSrvConnections() const;

protected:
    /// Creates the ::Ogre::SceneNode corresonding to the associated transform matrix.
    VISUOGREADAPTOR_API void doStart() throw(fwTools::Failed);
    /// Unregisters the service
    VISUOGREADAPTOR_API void doStop() throw(fwTools::Failed);
    /// Takes the attribute "parent" from m_config, and then puts it in m_parentTransformUID
    VISUOGREADAPTOR_API void doConfigure() throw(fwTools::Failed);
    /// Calls doUpdate
    VISUOGREADAPTOR_API void doSwap() throw(fwTools::Failed);
    /// Updates m_transform and m_ogreTransformNode from ::fwData::TransformationMatrix3D
    VISUOGREADAPTOR_API void doUpdate() throw(fwTools::Failed);
    /// returns the adress of the parentSceneNode from its transformId
    VISUOGREADAPTOR_API ::Ogre::SceneNode* getNodeById(::fwRenderOgre::SRender::OgreObjectIdType transformId);
    /**
     * @brief Ogre transform node.
     */
    ::Ogre::SceneNode* m_ogreTransformNode;
    /// Ogre parent Transform sceneNode
    ::Ogre::SceneNode* m_parentTransformNode;

};

} //namespace visuOgreAdaptor

#endif // __VISUOGREADAPTOR_STRANSFORM_HPP__
