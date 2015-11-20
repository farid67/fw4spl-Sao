/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __VISUOGREADAPTOR_SSHADERPARAMETER_HPP__
#define __VISUOGREADAPTOR_SSHADERPARAMETER_HPP__



#include <fwRenderOgre/IAdaptor.hpp>

#include "visuOgreAdaptor/config.hpp"

namespace visuOgreAdaptor
{

/**
 * @brief   Send a FW4SPL data as a shader parameter
 * @class   ShaderParameter
 */
class VISUOGREADAPTOR_CLASS_API SShaderParameter : public ::fwRenderOgre::IAdaptor
{

public:

    fwCoreServiceClassDefinitionsMacro ( (SShaderParameter)(::fwRenderOgre::IAdaptor) );

    /// Enum containing the different values for the supported shader types.
    typedef enum ShaderEnum
    {
        VERTEX,
        FRAGMENT,
        GEOMETRY
    } ShaderEnumType;

    /// Constructor.
    VISUOGREADAPTOR_API SShaderParameter() throw();

    /// Destructor. Does nothing
    VISUOGREADAPTOR_API virtual ~SShaderParameter() throw();

    /// Sets the shaderType by passing the value of the ShaderEnumType of this adaptor.
    VISUOGREADAPTOR_API void setShaderType(ShaderEnumType shaderType);

    /// Sets the shaderType by passing the name of the ShaderEnumType of this adaptor (vp or fp).
    VISUOGREADAPTOR_API void setShaderType(std::string shaderType);

    /// Sets the value of the member m_materialName.
    VISUOGREADAPTOR_API void setMaterialName(std::string matName);

    /// Sets the name of the parameter m_paramName.
    VISUOGREADAPTOR_API void setParamName(std::string paramName);

protected:

    /**
     * @brief Configure the ShaderParameter adaptor
     *
     * Actually can just send parameters to vertex and fragment shaders
     *
     * @verbatim
        <service uid="ShaderParameterInstance"
                 impl="::visuOgreAdaptor::SShaderParameter" type="::fwRenderOgre::IAdaptor">
             <parameter>param</parameter>
        </service>
       @endverbatim
     * - \b Parameter : parameter description.
     */
    VISUOGREADAPTOR_API virtual void doConfigure()  throw ( ::fwTools::Failed );
    /// Does Nothing
    VISUOGREADAPTOR_API virtual void doStart()  throw ( ::fwTools::Failed );
    /// Does Nothing
    VISUOGREADAPTOR_API virtual void doStop()  throw ( ::fwTools::Failed );
    /// Does Nothing
    VISUOGREADAPTOR_API virtual void doSwap() throw ( ::fwTools::Failed );
    /// Updates the shaderparameter values via the private method updateValue(), and requests a render of the scene.
    VISUOGREADAPTOR_API virtual void doUpdate() throw ( ::fwTools::Failed );

private:

    /**
     * @brief updateValue(), updates parameters function of the attached fwData::Object
     * Updates some ::Ogre::GpuProgramParametersSharedPtr
     * and directly sends them to the programmable pipeline of the GPU.
     *
     * - \b m_paramvalues is an array filled with the corresponding data
     * - \b m_paramType is the type (from the enum Types) of the data which values are stored in m_paramValues.
     */
    void updateValue();

    /// Contains the different parameters for the shader
    ::Ogre::GpuProgramParametersSharedPtr m_params;

    /// Pointer containing the value(s) of the shader parameter
    float* m_paramValues;
    /// Number of values contained by the shader parameter
    int m_paramNbElem;
    /// Indicates by what multiple are grouped the values of the shader parameter
    int m_paramElemMultiple;

    /// Material's name
    std::string m_materialName;
    /// Parameter's name
    std::string m_paramName;
    /// Stores the value of the enum representing the shader's type.
    ShaderEnumType m_shaderType;

};

} // visuOgreAdaptor

#endif // __VISUOGREADAPTOR_SSHADERPARAMETER_HPP__
