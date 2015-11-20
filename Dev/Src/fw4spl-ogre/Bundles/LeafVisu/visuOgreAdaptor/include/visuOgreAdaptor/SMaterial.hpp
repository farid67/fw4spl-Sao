/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __VISUOGREADAPTOR_SMATERIAL_HPP__
#define __VISUOGREADAPTOR_SMATERIAL_HPP__

#include "visuOgreAdaptor/config.hpp"

#include <fwCom/Slot.hpp>
#include <fwCom/Slots.hpp>

#include <fwData/Image.hpp>
#include <fwData/Material.hpp>

#include <fwRenderOgre/IAdaptor.hpp>

#include <boost/shared_ptr.hpp>

#include <OGRE/OgreGpuProgramParams.h>
#include <OGRE/OgreMaterial.h>

#include "visuOgreAdaptor/STexture.hpp"

fwCorePredeclare( (fwData)(Material) )

namespace visuOgreAdaptor
{


class VISUOGREADAPTOR_CLASS_API SMaterial : public ::fwRenderOgre::IAdaptor
{

public:

    fwCoreServiceClassDefinitionsMacro ( (SMaterial)(::fwRenderOgre::IAdaptor) );

    /**
     * @name Slots API
     * @{
     */
    VISUOGREADAPTOR_API static const ::fwCom::Slots::SlotKeyType s_UPDATE_FIELD_SLOT;
    VISUOGREADAPTOR_API static const ::fwCom::Slots::SlotKeyType s_REMOVE_FIELD_SLOT;

    VISUOGREADAPTOR_API static const ::fwCom::Slots::SlotKeyType s_SWAP_TEXTURE_SLOT;
    VISUOGREADAPTOR_API static const ::fwCom::Slots::SlotKeyType s_ADD_TEXTURE_SLOT;
    VISUOGREADAPTOR_API static const ::fwCom::Slots::SlotKeyType s_REMOVE_TEXTURE_SLOT;
    /** @} */

    /// Name of the default used as a base for the instance created by this adaptor
    VISUOGREADAPTOR_API static const std::string DEFAULT_MATERIAL_TEMPLATE_NAME;

    /// Constructor
    VISUOGREADAPTOR_API SMaterial() throw();

    /// Destructor
    VISUOGREADAPTOR_API virtual ~SMaterial() throw();

    /// Get Ogre associated material
    VISUOGREADAPTOR_API ::Ogre::MaterialPtr getMaterial();

    /// Get material name
    VISUOGREADAPTOR_API std::string getMaterialName() const;

    /// Set material name
    VISUOGREADAPTOR_API void setMaterialName(const std::string &materialName);

    /// Set material template name
    VISUOGREADAPTOR_API void setMaterialTemplateName(const std::string &materialName);

    /// Retrieves the associated texture adaptor
    VISUOGREADAPTOR_API void setTextureAdaptor(const std::string& textureAdaptorId);

    VISUOGREADAPTOR_API bool getHasMeshNormal() const;

    VISUOGREADAPTOR_API void setHasMeshNormal(bool hasMeshNormal);

    /// Update fwData material parameters from Ogre material parameters
    VISUOGREADAPTOR_API void updateFromOgre();

    /// Returns proposals to connect service slots to associated object signals
    ::fwServices::IService::KeyConnectionsType getObjSrvConnections() const;

protected:

    /**
     * @brief Configures the adaptor
     * @verbatim
       <adaptor id="materialAdaptor" class="::visuOgreAdaptor::SMaterial" objectId="materialKey">
        <config materialTemplate="materialTemplateName" materialName="meshMaterial" textureAdaptor="texAdaptorUID" />
       </adaptor>
       @endverbatim
     * With :
     *  - \b materialTemplate (mandatory) : name of the base Ogre material
     *  - \b materialName (optional) : name of the managed Ogre material
     *  - \b textureAdaptor (optional) : the texture adaptor listened by the material
     */
    VISUOGREADAPTOR_API void doConfigure() throw(fwTools::Failed);

    /// Starting method under fixed function pipeline
    VISUOGREADAPTOR_API void doStart() throw(fwTools::Failed);

    /// Stopping method
    VISUOGREADAPTOR_API void doStop() throw(fwTools::Failed);

    /// Swapping method, doUpdate
    VISUOGREADAPTOR_API void doSwap() throw(fwTools::Failed);

    /// Updating method, updates fixed function pipeline parameters
    VISUOGREADAPTOR_API void doUpdate() throw(fwTools::Failed);

private:

    /// Slot called when the material's field changed
    void updateField( ::fwData::Object::FieldsContainerType fields);

    /// Slot called when the texture is swapped in the texture adaptor
    void swapTexture();

    /// Creates a new object from loaded shader
    ::fwData::Object::sptr createObjectFromShaderParameter(::Ogre::GpuConstantType type,
                                                           std::string paramName);

    /// Loads material parameters from ressources
    void loadMaterialParameters();

    /// Updates material parameters from a specific template
    void loadShaderParameters(::Ogre::GpuProgramParametersSharedPtr params, std::string shaderType);

    /// Updates material polygon mode (surface/point/wireframe) in fixed function pipeline
    void setPolygonMode( int polygonMode );

    /// Manages service associated to a shader parameter
    void setServiceOnShaderParameter(::fwRenderOgre::IAdaptor::sptr& srv,
                                     std::shared_ptr<fwData::Object> object,
                                     std::string paramName,
                                     std::string shaderType);

    /// Update material shading mode (gourand/phong/flat) in fixed function pipeline
    void setShadingMode( int shadingMode );

    /// Update material color in fixed function pipeline
    void updateRGBAMode( ::fwData::Material::sptr fw_material );

    /// Slot called to create a texture adaptor when a texture is added to the material.
    /// This method is also called from the doStart in order to create the texture adaptor if the material has a
    /// default texture.
    void createTextureAdaptor();

    /// Slot called to remove the texture adaptor when the texture is removed from the material
    void removeTextureAdaptor(::fwData::Image::sptr texture);

    /// Updates material color in fixed function pipeline
    void updateTransparency( ::fwData::Material::sptr fw_material );

    /// Checks support of technique's schemes
    void updateSchemeSupport();

    /// Associated Ogre material
    ::Ogre::MaterialPtr m_material;
    /// Material name. It is auto generated.
    std::string m_materialName;

    /// Default template name, given by xml configuration.
    /// It must refer an existing Ogre material which will be used in order to instanciate m_material
    std::string m_materialTemplateName;

    /// The texture adaptor the material adaptor is listening to
    ::visuOgreAdaptor::STexture::sptr m_texAdaptor;
    std::string m_texAdaptorUID;

    /// Defines if the associated mesh has a normal layer
    bool m_hasMeshNormal;

    std::vector< Ogre::String > m_schemesSupported;

    /// Signal/Slot connections with texture adaptor
    ::fwServices::helper::SigSlotConnection::sptr m_textureConnection;
};

} //namespace visuOgreAdaptor

#endif // __VISUOGREADAPTOR_SMATERIAL_HPP__
