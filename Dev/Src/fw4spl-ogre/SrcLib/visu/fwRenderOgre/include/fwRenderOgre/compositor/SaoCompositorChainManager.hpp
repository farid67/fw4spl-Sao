#ifndef __FWRENDEROGE_COMPOSITOR_SAOCOMPOSITORCHAINMANAGER_HPP
#define __FWRENDEROGE_COMPOSITOR_SAOCOMPOSITORCHAINMANAGER_HPP


// used to view the content of the Ogre Compositor Chain
#include <OGRE/OgreCompositorChain.h>
#include <OGRE/OgreCompositorInstance.h>
#include <OGRE/OgreCompositor.h>
#include <OGRE/OgrePrerequisites.h>
#include <OGRE/OgreCompositorManager.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreTexture.h>
#include <OGRE/OgreTextureManager.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgrePass.h>


#include <fwCore/BaseObject.hpp>
#include <fwRenderOgre/config.hpp>
#include <vector>

namespace Ogre
{
class CompositorManager;
class Viewport;
}

namespace fwRenderOgre // on s'insère ici
{
    class FWRENDEROGRE_CLASS_API SaoCompositorChainManager : public ::fwCore::BaseObject
    {
    public:
        fwCoreClassDefinitionsWithFactoryMacro ((SaoCompositorChainManager)(::fwRenderOgre::SaoCompositorChainManager),(()),
                                                new SaoCompositorChainManager);
        fwCoreAllowSharedFromThis(); // permet de générer un pointeur vers la classe souhaitée en la castant correctement

        typedef std::string SaoCompositorIdType; // un compositor ne sera ici qu'une chaine de caractère correspondant à son nom (pas de booléen car toujours activé lorsque la chaine est choisie)
        typedef std::vector<SaoCompositorIdType> SaoCompositorChainType; // liste des

        FWRENDEROGRE_API SaoCompositorChainManager();
        FWRENDEROGRE_API SaoCompositorChainManager(::Ogre::Viewport* ogreViewport);// Chaine de compositor attachée au viewport Ogre

        FWRENDEROGRE_API void setOgreViewport(::Ogre::Viewport* viewport);

        // pas sur -> bouton = enable?
        FWRENDEROGRE_API void setSaoState(bool state);

        // inline function of the end

        FWRENDEROGRE_API SaoCompositorChainType getSaoCompositorChain();

        // Here we add the class derivated from the Ogre Listener
        class FWRENDEROGRE_CLASS_API SaoListener : public ::Ogre::CompositorInstance::Listener
        {
        public:
            SaoListener():m_VP(nullptr),m_MipMap(nullptr){}
            SaoListener(::Ogre::Viewport* vp):m_VP(vp)
            {

                // creation de la texture MipMap texture, pour le moment ici
                m_MipMap = ::Ogre::TextureManager::getSingletonPtr()->createManual("MipMap_tex", // texture name
                                                                                   "General", // texture group name
                                                                                ::Ogre::TextureType::TEX_TYPE_2D, // texture type -> 2D array to contain the mip map levels
                                                                                256,256,1, // size + MipMap levels
                                                                                ::Ogre::PixelFormat::PF_FLOAT32_R, // pixel format
                                                                                ::Ogre::TextureUsage::TU_DYNAMIC).get(); // usage
            }
            ~SaoListener(){}
            void notifyMaterialRender(::Ogre::uint32 pass_id, ::Ogre::MaterialPtr& mat)
                // method called before a render_target operation involving a material to set dynamically the material parameters
            {
                //§ When entering in this method, the Mip Map compositor has finished its work

                std::cout << "No? " << std::endl;
                std::cout << mat.get()->getName() << std::endl;


                // ---------------------------------------------------
                //  Try to change the content of the Second Compositor
                // ---------------------------------------------------


                // get a pointer on the precedent compositor


                ::Ogre::CompositorChain* compChain =
                    ::Ogre::CompositorManager::getSingletonPtr()->getCompositorChain(m_VP);
                ::Ogre::CompositorChain::InstanceIterator compIter = compChain->getCompositors();


//                ::Ogre::Compositor *MipMap,*Test;
                ::Ogre::TexturePtr mip0,mip1,mip2;//,rt0;
                while( compIter.hasMoreElements())
                {
                    ::Ogre::CompositorInstance* targetComp = compIter.getNext();
                    if (targetComp->getCompositor()->getName() == "MipMap")
                    {
                        mip0 = targetComp->getTextureInstance("mip0",0);
                        mip1 = targetComp->getTextureInstance("mip1",0);
                        mip2 = targetComp->getTextureInstance("mip2",0);
                    }
//                    if (targetComp->getCompositor()->getName() == "Test")
//                    {
//                        rt0 = targetComp->getTextureInstance("rt0",0);
//                    }
                }


                // ---------------------------------------------------
                //  Copy the content of mip0,mip1... in a texture
                // ---------------------------------------------------




                // get a pointer on the MipMap texture created
//                ::Ogre::TexturePtr MipMap_tex = ::Ogre::TextureManager::getSingletonPtr()->getByName("MipMap_tex");
//                MipMap_tex.get()->getGroup() = mip0.get()->getGroup();

                m_MipMap->freeInternalResources();

                m_MipMap->changeGroupOwnership(mip0.get()->getGroup());
                m_MipMap->setWidth(mip0.get()->getWidth());
                m_MipMap->setHeight(mip0.get()->getHeight());
                m_MipMap->setNumMipmaps(1);
                m_MipMap->setFormat(::Ogre::PixelFormat::PF_FLOAT32_R);
                m_MipMap->setUsage(::Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
                m_MipMap->setTextureType(::Ogre::TextureType::TEX_TYPE_2D);

                m_MipMap->createInternalResources();

                // first Mip Map level -> get the Pixel Buffer
                if (!(m_MipMap == nullptr))
                    std::cout << "Mip Map texture correctly created" << std::endl;
                else
                    std::cout << "Can not load the created texture " << std::endl;
/*
                ::Ogre::HardwarePixelBufferSharedPtr Mip_0_level = m_MipMap->getBuffer(0,0);
                if (!Mip_0_level.isNull())
                    std::cout << "Level 0 correctly recovered" << std::endl;

                ::Ogre::HardwarePixelBufferSharedPtr mip0_buff = mip0.get()->getBuffer();
//                mip0_buff.get()->lock(0,mip0.get()->getSize(),::Ogre::HardwareBuffer::HBL_READ_ONLY);
                mip0_buff.get()->lock(::Ogre::HardwareBuffer::HBL_READ_ONLY);
//                Mip_0_level.get()->lock(0,mip0.get()->getHeight()*mip0.get()->getWidth(),::Ogre::HardwareBuffer::HBL_DISCARD);
                Mip_0_level.get()->lock(::Ogre::HardwareBuffer::HBL_DISCARD);


                ::Ogre::PixelBox pDest, pSend;
                pDest = Mip_0_level.get()->getCurrentLock();
                pSend = mip0_buff.get()->getCurrentLock();

                float *dest = static_cast<float*> (pDest.data);
                float *send = static_cast<float*> (pSend.data);

                // write values

                for (size_t i = 0; i < pSend.getHeight() * pSend.getWidth(); i++)
                {
                    dest[i] = send[i];
                }

                // unlock
                mip0_buff.get()->unlock();
                Mip_0_level.get()->unlock();
*/

                // The following line should work but don't give the good result
                m_MipMap->getBuffer(0,0)->blit(mip0.get()->getBuffer());
//                m_MipMap->getBuffer(0,0)->blit(mip0.get()->getBuffer());
                // same here
//                Mip_0_level.get()->blitFromMemory(pSend);


/*

                // test Second mip level write directly
                ::Ogre::HardwarePixelBufferSharedPtr Mip_1_level = m_MipMap->getBuffer(0,1);
                if (!Mip_1_level.isNull())
                    std::cout << "Level 1 correctly recovered" << std::endl;
//                Mip_1_level.get()->blit(mip1.get()->getBuffer());



                // test
                std::cout << "mip0 size : " << mip0.get()->getSize() << std::endl;
                std::cout << "mip0 : " << mip0.get()->getWidth() << "*" << mip0.get()->getHeight() << std::endl;
                std::cout << "mip1 size : " << mip1.get()->getSize() << std::endl;
                std::cout << "PDest size " << pDest.getWidth() << "*" << pDest.getHeight() << std::endl;
                std::cout << "PSend size " << pSend.getWidth() << "*" << pSend.getHeight() << std::endl;


                ::Ogre::HardwarePixelBufferSharedPtr mip1_buff = mip1.get()->getBuffer();
//                mip1_buff.get()->lock(0,mip1.get()->getSize(),::Ogre::HardwareBuffer::HBL_READ_ONLY);
                mip1_buff.get()->lock(::Ogre::HardwareBuffer::HBL_READ_ONLY);



//                Mip_1_level.get()->lock(0,mip1.get()->getSize(),::Ogre::HardwareBuffer::HBL_DISCARD);
                Mip_1_level.get()->lock(::Ogre::HardwareBuffer::HBL_DISCARD);


                pDest = Mip_1_level.get()->getCurrentLock();
                pSend = mip1_buff.get()->getCurrentLock();
                std::cout << "PDest2 size " << pDest.getWidth() << "*" << pDest.getHeight() << std::endl;
                dest = static_cast<float*> (pDest.data);
                send = static_cast<float*> (pSend.data);

                for (size_t i = 0; i < pSend.getHeight() * pSend.getWidth() ; i++)
                {
                    dest[i] = send[i];
                }

                // unlock
                Mip_1_level.get()->unlock();
                mip1_buff.get()->unlock();

*/

//                // second Mip Map level
//                ::Ogre::HardwarePixelBufferSharedPtr Mip_1_level = MipMap_tex.get()->getBuffer(0,1);
//                Mip_1_level.get()->copyData(mip1.get()->getBuffer().dynamicCast);
//                // third
//                ::Ogre::HardwarePixelBufferSharedPtr Mip_2_level = MipMap_tex.get()->getBuffer(0,2);
//                Mip_2_level.get()->copyData(mip2.get()->getBuffer().dynamicCast);


                // try to change the rt0 parameter of the Test Compositor

                if (!mip0.isNull() && !mip1.isNull() && !mip2.isNull() && !(m_MipMap == nullptr))
                {/*
                    if (!rt0.isNull())
                    {*/
                        std::cout << "try to change the input of the next material" << std::endl;
//                        std::cout << "try to copy the texture mip0 in rt0" << std::endl;
//                        m_MipMap->copyToTexture(rt0);
                        if (mat.get()->getTechnique(0)->getPass(0)->createTextureUnitState("MipMap_tex") != nullptr)
                            std::cout << "texture correctly sent to the material" << std::endl;
//                        mip0.get()->copyToTexture(rt0);
//                    }
                }



            }

        private:
            ::Ogre::Viewport* m_VP;
            ::Ogre::Texture* m_MipMap;
        };

    private :
        /// Getter for the Ogre CompositorManager
        ::Ogre::CompositorManager* getCompositorManager();

        /// The parent layer's viewport.
        /// The ogre's compositor manager needs it in order to access the right compositor chain.
        ::Ogre::Viewport* m_ogreViewport;

        SaoCompositorChainType m_saoChain;

//        ::Ogre::TexturePtr m_MipMap_tex;

    };

    //-----------------------------------------------------------------------------
    // Inline method(s)

    inline SaoCompositorChainManager::SaoCompositorChainType SaoCompositorChainManager::getSaoCompositorChain()
    {
        return m_saoChain;
    }

    //-----------------------------------------------------------------------------

    inline void SaoCompositorChainManager::setOgreViewport(::Ogre::Viewport* viewport)
    {
        m_ogreViewport = viewport;
    }

    //-----------------------------------------------------------------------------


}// namespace fwRenderOgre

#endif // __FWRENDEROGE_COMPOSITOR_SAOCOMPOSITORCHAINMANAGER_HPP
