#include "fwRenderOgre/compositor/SaoCompositorChainManager.hpp"

#include <fwCore/spyLog.hpp>

#include <fwCore/spyLog.hpp>

namespace fwRenderOgre {

//-----------------------------------------------------------------------------

SaoCompositorChainManager::SaoCompositorChainManager(): m_ogreViewport(0)
{
    m_saoChain.push_back("MipMap");
    m_saoChain.push_back("Test");

    // create MipMap texture
}

SaoCompositorChainManager::SaoCompositorChainManager(::Ogre::Viewport* viewport): m_ogreViewport(viewport)
{
    // create the chain
    m_saoChain.push_back("MipMap");
    //here add the second compositor
    m_saoChain.push_back("Test");

}



// This method can let the manager change the content of the original Ogre's compositor chain
::Ogre::CompositorManager* SaoCompositorChainManager::getCompositorManager()
{
    return ::Ogre::CompositorManager::getSingletonPtr();
}


// fonction that set the Sao Compositor
void SaoCompositorChainManager::setSaoState(bool state)
{
    std::cout << "try to change the state of the sao manager" << std::endl;
    // get the Compositor Manager of Ogre (of the scene)
    ::Ogre::CompositorManager* compositorManager = this->getCompositorManager();
    // if here, we add the compositor present of the chain in the Ogre chain compositor
    if (state)
    {
        // check the content of the Ogre Compositor chain

        ::Ogre::CompositorChain* compChain =
            ::Ogre::CompositorManager::getSingletonPtr()->getCompositorChain(m_ogreViewport);
        ::Ogre::CompositorChain::InstanceIterator compIter = compChain->getCompositors();

        std::cout << "Liste des compositors actuellement dans la chaine " << std::endl;


        // à faire : vérifier que MipMap et Test sont dans la liste, les ajoutés sinon avec addCompositor
        while( compIter.hasMoreElements())
        {
            ::Ogre::CompositorInstance* targetComp = compIter.getNext();
            std::cout << targetComp->getCompositor()->getName() << std::endl;
            if (targetComp->getEnabled())
                std::cout << "      Enable" << std::endl;
        }


               compIter = compChain->getCompositors();

        while( compIter.hasMoreElements())
        {
            ::Ogre::CompositorInstance* targetComp = compIter.getNext();
            std::cout << targetComp->getCompositor()->getName() << std::endl;
            if (targetComp->getEnabled())
                std::cout << "      Enable" << std::endl;

            if (targetComp->getCompositor()->getName() == "Test")
            {
                std::cout << "attach a Listener " << std::endl;
                targetComp->addListener(new SaoListener(m_ogreViewport));
            }

        }

        // add all the compositor of the chain
        std::cout << "try to enable the sao chain" << std::endl;
        for(SaoCompositorIdType compositorName : m_saoChain)
        {
            std::cout << "compositor found in the sao chain" << std::endl;
            if(this->getCompositorManager()->resourceExists(compositorName))
            {
                std::cout << "Le compositor " << static_cast<std::string>(compositorName) << " existe et a correctement été chargé " << std::endl;
                //compositorManager->addCompositor(m_ogreViewport, compositorName);
                compositorManager->setCompositorEnabled(m_ogreViewport, compositorName, true);
            }
            else
                OSLM_WARN("\"" << compositorName << "\" does not refer to an existing compositor");

        }


        // check the content of the Ogre Compositor chain

        std::cout << "Liste des compositors actuellement dans la chaine " << std::endl;




    }
    else // disable the sao Chain
    {

        for(SaoCompositorIdType compositorName : m_saoChain)
        {
            if(this->getCompositorManager()->resourceExists(compositorName))
            {
                compositorManager->setCompositorEnabled(m_ogreViewport, compositorName, false);
            }
        }
    }
}


}// namespace fwRenderOgre
