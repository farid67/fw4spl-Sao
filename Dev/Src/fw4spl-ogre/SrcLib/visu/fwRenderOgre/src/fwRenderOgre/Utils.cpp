/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "fwRenderOgre/Utils.hpp"

#include <fwCore/spyLog.hpp>

#include <fwComEd/helper/Image.hpp>
#include <fwComEd/fieldHelper/MedicalImageHelpers.hpp>

#include <OgreConfigFile.h>
#include <OgreException.h>
#include <OgreLog.h>
#include <OgreResourceGroupManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreTextureManager.h>

#include <cstdint>

#ifdef __MACOSX__
#define PLUGIN_PATH "./share/fwRenderOgre_0-1/plugins_osx.cfg"
#elif _DEBUG
#define PLUGIN_PATH "./share/fwRenderOgre_0-1/plugins_d.cfg"
#else
#define PLUGIN_PATH "./share/fwRenderOgre_0-1/plugins.cfg"
#endif

#define RESOURCES_PATH "./share/fwRenderOgre_0-1/resources.cfg"

namespace fwRenderOgre
{

static std::set<std::string> s_resourcesPath;

::Ogre::OverlaySystem* Utils::s_overlaySystem = nullptr;

//------------------------------------------------------------------------------

void loadResources()
{
    ::Ogre::ConfigFile cf;
    ::Ogre::String resourceGroupName, typeName, archName;

    for(const auto& path : s_resourcesPath)
    {
        try
        {
            cf.load(path);

            ::Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

            while (seci.hasMoreElements())
            {
                resourceGroupName                              = seci.peekNextKey();
                ::Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
                ::Ogre::ConfigFile::SettingsMultiMap::iterator i;
                for (i = settings->begin(); i != settings->end(); ++i)
                {
                    typeName = i->first;
                    archName = i->second;
                    ::Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName,
                                                                                     resourceGroupName);
                }
            }
        }
        catch ( ::Ogre::FileNotFoundException& )
        {
            SLM_WARN("Unable to find Ogre resources path : " + path);
        }
        catch (...)
        {
            SLM_WARN("Unable to load resource from " + path);
        }
    }
}

//------------------------------------------------------------------------------

void Utils::addResourcesPath(const std::string& path)
{
    s_resourcesPath.insert(path);
}

//------------------------------------------------------------------------------

::Ogre::OverlaySystem* Utils::getOverlaySystem()
{
    return s_overlaySystem;
}

//------------------------------------------------------------------------------

::Ogre::Root* Utils::getOgreRoot()
{
    ::Ogre::Root* root = ::Ogre::Singleton< ::Ogre::Root >::getSingletonPtr();

    if(root == nullptr)
    {
        root            = new ::Ogre::Root(PLUGIN_PATH);
        s_overlaySystem = new ::Ogre::OverlaySystem();

        const Ogre::RenderSystemList& rsList = root->getAvailableRenderers();
        Ogre::RenderSystem* rs               = rsList[0];

        /*
           This list setup the search order for used render system.
         */
        Ogre::StringVector renderOrder;

        renderOrder.push_back("OpenGL");
        //renderOrder.push_back("OpenGL 3+");
        for (Ogre::StringVector::iterator iter = renderOrder.begin(); iter != renderOrder.end(); iter++)
        {
            for (Ogre::RenderSystemList::const_iterator it = rsList.begin(); it != rsList.end(); it++)
            {
                if ((*it)->getName().find(*iter) != Ogre::String::npos)
                {
                    rs = *it;
                    break;
                }
            }
            if (rs != nullptr)
            {
                break;
            }
        }
        if (rs == nullptr)
        {
            if (!root->restoreConfig())
            {
                if (!root->showConfigDialog())
                {
                    OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS,
                                "Abort render system configuration",
                                "Window::initialize");
                }
            }
        }

        /*
           Setting size and VSync on windows will solve a lot of problems
         */
//        std::string dimensions = this->width() + " x " + this->height();
//        rs->setConfigOption("Video Mode", dimensions.toStdString());
        rs->setConfigOption("Full Screen", "No");
#ifdef __MACOSX__
        rs->setConfigOption("vsync", "No");
#else
        rs->setConfigOption("VSync", "No");
        rs->setConfigOption("Display Frequency", "60");
#endif


        root->setRenderSystem(rs);

        root->initialise(false);

        ::fwRenderOgre::Utils::addResourcesPath(RESOURCES_PATH);
        loadResources();

        // TODO : Check utility of TextureManager in a shader-based programming model (RenderSystemGL3+)
        if(::Ogre::Root::getSingleton().getRenderSystem()->getName() != "OpenGL 3+ Rendering Subsystem (ALPHA)")
        {
            Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
        }
    }

    return root;
}

//------------------------------------------------------------------------------

void Utils::destroyOgreRoot()
{
    ::Ogre::Root* root = ::fwRenderOgre::Utils::getOgreRoot();
    ::Ogre::ResourceGroupManager::getSingleton().shutdownAll();

    delete s_overlaySystem;

    root->shutdown();
    delete root;
}

//------------------------------------------------------------------------------

::Ogre::Image Utils::convertFwDataImageToOgreImage( const ::fwData::Image::sptr imageFw)
{
    SLM_ASSERT("Image is null", imageFw);

    ::Ogre::Image imageOgre;
    uint32_t width = 1, height = 1, depth = 1;

    // If image is flipped, try to switch image
    ::fwData::Image::SizeType imageSize = imageFw->getSize();

    width = static_cast<uint32_t>(imageSize[0]);

    if(imageSize.size() >= 2)
    {
        height = static_cast<uint32_t>(imageSize[1]);

        if(imageSize.size() == 3)
        {
            depth = static_cast<uint32_t>(imageSize[2]);
        }
    }

    ::Ogre::PixelFormat pixelFormat = getPixelFormatOgre( imageFw );

    ::fwComEd::helper::Image imageHelper(imageFw);

    imageOgre.loadDynamicImage(static_cast<uint8_t*>(imageHelper.getBuffer()), width, height, depth, pixelFormat);

    return imageOgre;
}

//------------------------------------------------------------------------------

void Utils::convertImageForNegato( ::Ogre::Texture* _texture, const ::fwData::Image::sptr& _image )
{
    auto srcType = _image->getType();

    if(srcType == ::fwTools::Type::s_INT16)
    {
        if( _texture->getWidth()  != _image->getSize()[0] ||
            _texture->getHeight() != _image->getSize()[1] ||
            _texture->getDepth()  != _image->getSize()[2]    )
        {
            _texture->freeInternalResources();

            _texture->setWidth(static_cast< ::Ogre::uint32>(_image->getSize()[0]));
            _texture->setHeight(static_cast< ::Ogre::uint32>(_image->getSize()[1]));
            _texture->setDepth(static_cast< ::Ogre::uint32>(_image->getSize()[2]));
            _texture->setTextureType(::Ogre::TEX_TYPE_3D);
            _texture->setNumMipmaps(0);
            _texture->setFormat(::Ogre::PF_L16);
            _texture->setUsage(::Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

            _texture->createInternalResources();
        }

        // Get the pixel buffer
        ::Ogre::HardwarePixelBufferSharedPtr pixelBuffer = _texture->getBuffer();

        // Lock the pixel buffer and copy it
        {
            ::fwComEd::helper::Image srcImageHelper(_image);

            const std::int16_t* __restrict srcBuffer = static_cast< const std::int16_t* >(srcImageHelper.getBuffer());
            const ::Ogre::uint32 size                = _texture->getWidth() * _texture->getHeight() *
                                                       _texture->getDepth();

            pixelBuffer->lock(::Ogre::HardwareBuffer::HBL_DISCARD);

            const ::Ogre::PixelBox& pixelBox = pixelBuffer->getCurrentLock();

            std::uint16_t* __restrict pDest = static_cast<std::uint16_t*>(pixelBox.data);

            const std::int16_t lowBound = std::numeric_limits< std::int16_t >::min();

            for(::Ogre::uint32 i = 0; i < size; ++i)
            {
                *pDest++ = static_cast<std::uint16_t>(*srcBuffer++ - lowBound);
            }

            // Unlock the pixel buffer
            pixelBuffer->unlock();
        }
    }
    else
    {
        SLM_FATAL("Image format not supported.");
    }
}

//------------------------------------------------------------------------------

// Only handles RGB for now, since fwData::Image only does so.
::Ogre::PixelFormat Utils::getPixelFormatOgre(::fwData::Image::sptr imageFw)
{
    std::string pixelType    = ::fwTools::getString( imageFw->getPixelType() );
    size_t numberOfComponent = imageFw->getNumberOfComponents();

    if(numberOfComponent == 1)
    {
        if(pixelType == "unsigned char")
        {
            // uint8
            return ::Ogre::PF_L8;
        }
        else if(pixelType == "signed short")
        {
            // int16
            return ::Ogre::PF_L16;
        }
    }

    if(numberOfComponent == 2)
    {
        if(pixelType == "unsigned char")
        {
            // uint8
            return ::Ogre::PF_RG8;
        }
        else if(pixelType == "signed short")
        {
            // int16
            return ::Ogre::PF_R8G8_SNORM;
        }
    }

    // PixelFormat in little endian
    if(pixelType == "unsigned char")
    {
        // uint8
        return numberOfComponent == 3 ? ::Ogre::PF_BYTE_RGB : ::Ogre::PF_BYTE_RGBA;
    }
    else if (pixelType == "unsigned short")
    {
        // uint16
        return numberOfComponent == 3 ? ::Ogre::PF_R16G16B16_UINT : ::Ogre::PF_R16G16B16A16_UINT;
    }
    else if (pixelType == "unsigned int")
    {
        // uint32
        return numberOfComponent == 3 ? ::Ogre::PF_R32G32B32_UINT : ::Ogre::PF_R32G32B32A32_UINT;
    }
    else if (pixelType =="signed char" )
    {
        // int8
        return numberOfComponent == 3 ? ::Ogre::PF_R8G8B8_SINT : ::Ogre::PF_R8G8B8A8_SINT;
    }
    else if (pixelType == "signed short")
    {
        // int16
        return numberOfComponent == 3 ? ::Ogre::PF_R16G16B16_SINT : ::Ogre::PF_R16G16B16A16_SINT;
    }
    else if (pixelType == "signed int")
    {
        // int32
        return numberOfComponent == 3 ? ::Ogre::PF_R32G32B32_SINT : ::Ogre::PF_R32G32B32A32_SINT;
    }
    else if (pixelType =="unsigned long" )
    {
        // int64
        return numberOfComponent == 3 ? ::Ogre::PF_SHORT_RGB : ::Ogre::PF_SHORT_RGBA;
    }
    else if (pixelType == "signed long")
    {
        // uint64
        return numberOfComponent == 3 ? ::Ogre::PF_R16G16B16_UINT : ::Ogre::PF_R16G16B16A16_UINT;
    }
    else if (pixelType == "float")
    {
        return numberOfComponent == 3 ? ::Ogre::PF_FLOAT16_RGB : ::Ogre::PF_FLOAT16_RGBA;
    }
    else if (pixelType == "double")
    {
        OSLM_FATAL("Pixel format not handled.");
    }
    SLM_WARN("Pixel format not found, trying with the default 8-bits RGBA.");
    return ::Ogre::PF_BYTE_RGBA;
}

//------------------------------------------------------------------------------

void Utils::loadOgreTexture(::fwData::Image::sptr image, ::Ogre::TexturePtr texture,
                            ::Ogre::TextureType texType, ::Ogre::PixelFormat pxFormat)
{
    bool imageIsValid = ::fwComEd::fieldHelper::MedicalImageHelpers::checkImageValidity(image);

    if(imageIsValid)
    {
        // Conversion from fwData::Image to ::Ogre::Image
        ::Ogre::Image ogreImage = ::fwRenderOgre::Utils::convertFwDataImageToOgreImage(image);
        texture->freeInternalResources();

        texture->setWidth(ogreImage.getWidth());
        texture->setHeight(ogreImage.getHeight());
        texture->setTextureType(texType);
        texture->setDepth(ogreImage.getDepth());
        texture->setNumMipmaps(0);
        texture->setFormat(pxFormat);
        texture->setUsage(::Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

        texture->createInternalResources();

        // Copy image's pixel box into texture buffer
        texture->getBuffer(0,0)->blitFromMemory(ogreImage.getPixelBox(0,0));
    }
}

//------------------------------------------------------------------------------

} // namespace fwRenderOgre
