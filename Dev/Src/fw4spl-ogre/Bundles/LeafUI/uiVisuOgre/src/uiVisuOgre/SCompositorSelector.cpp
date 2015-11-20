/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "uiVisuOgre/SCompositorSelector.hpp"

#include <fwCom/Slots.hxx>

#include <fwData/Composite.hpp>

#include <fwGuiQt/container/QtContainer.hpp>

#include <fwServices/Base.hpp>
#include <fwServices/IService.hxx>

#include <fwRenderOgre/SRender.hpp>

#include <material/Plugin.hpp>

#include <QWidget>
#include <QListWidgetItem>

#include <OGRE/OgreCompositorManager.h>
#include <OGRE/OgreResource.h>
#include <OGRE/OgreResourceManager.h>

namespace uiVisuOgre
{

fwServicesRegisterMacro( ::gui::editor::IEditor, ::uiVisuOgre::SCompositorSelector, ::fwData::Composite);

const ::fwCom::Slots::SlotKeyType SCompositorSelector::s_INIT_COMPOSITOR_LIST_SLOT = "initCompositorList";

//------------------------------------------------------------------------------

SCompositorSelector::SCompositorSelector() throw() :
    m_currentLayer(nullptr)
{
    newSlot(s_INIT_COMPOSITOR_LIST_SLOT, &SCompositorSelector::initCompositorList, this);

    m_connections = ::fwServices::helper::SigSlotConnection::New();
}

//------------------------------------------------------------------------------

SCompositorSelector::~SCompositorSelector() throw()
{
}

//------------------------------------------------------------------------------

void SCompositorSelector::starting() throw(::fwTools::Failed)
{
    this->create();

    ::fwGuiQt::container::QtContainer::sptr qtContainer = ::fwGuiQt::container::QtContainer::dynamicCast(
        this->getContainer() );
    QWidget* const container = qtContainer->getQtContainer();
    SLM_ASSERT("container not instantiated", container);

    m_layersBox       = new QComboBox(container);
    m_compositorChain = new QListWidget(container);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(m_layersBox);
    layout->addWidget(m_compositorChain);

    container->setLayout( layout );

    this->refreshRenderers();

    QObject::connect(m_layersBox, SIGNAL(activated(int)), this,
                     SLOT(onSelectedLayerItem(int)));
    QObject::connect(m_compositorChain, SIGNAL(itemChanged(QListWidgetItem*)), this,
                     SLOT(onSelectedCompositorItem(QListWidgetItem*)));
}

//------------------------------------------------------------------------------

void SCompositorSelector::stopping() throw(::fwTools::Failed)
{
    m_connections->disconnect();

    QObject::disconnect(m_layersBox, SIGNAL(activated(const QString &)), this,
                        SLOT(onSelectedLayerItem(const QString &)));
    QObject::disconnect(m_compositorChain, SIGNAL(itemChanged(QListWidgetItem*)), this,
                        SLOT(onSelectedCompositorItem(QListWidgetItem*)));

    this->getContainer()->clean();
    this->destroy();
}

//------------------------------------------------------------------------------

void SCompositorSelector::configuring() throw(::fwTools::Failed)
{
    this->initialize();
}

//------------------------------------------------------------------------------

void SCompositorSelector::updating() throw(::fwTools::Failed)
{
}

//------------------------------------------------------------------------------

void SCompositorSelector::onSelectedLayerItem(int index)
{
    // Empty the list widget
    m_compositorChain->clear();

    // Update the current layer
    m_currentLayer = m_layers[static_cast<size_t>(index)];
    // Update the layer's compositor chain
    this->synchroniseWithLayerCompositorChain();

    // We need the ogre's viewport in order to add the compositors,
    // this is why we have to ckeck the viewport's existence
    if(m_currentLayer->getViewport())
    {
        // Fill the list widget
        this->updateCompositorList();
        // Iterates through the compositors and checks the enabled ones
        this->checkEnabledCompositors();
    }
}

//------------------------------------------------------------------------------

void SCompositorSelector::onSelectedCompositorItem(QListWidgetItem* compositorItem)
{
    ::std::string compositorName = compositorItem->text().toStdString();
    bool isChecked = (compositorItem->checkState() == ::Qt::Checked);
    m_currentLayer->updateCompositorState(compositorName, isChecked);
}

//------------------------------------------------------------------------------

void SCompositorSelector::initCompositorList(fwRenderOgre::Layer::sptr layer)
{
    m_currentLayer = m_layers[0];

    if(layer == m_currentLayer)
    {
        onSelectedLayerItem(0);
    }
}

//------------------------------------------------------------------------------

void SCompositorSelector::refreshRenderers()
{
    m_layersBox->clear();

    // Fill layer box with all enabled layers
    ::fwServices::registry::ObjectService::ServiceVectorType renderers =
        ::fwServices::OSR::getServices("::fwRenderOgre::SRender");

    int nbRenderer = 1;
    for(auto srv : renderers)
    {
        ::fwRenderOgre::SRender::sptr render = ::fwRenderOgre::SRender::dynamicCast(srv);

        for(auto &layerMap : render->getLayers())
        {
            const std::string id       = layerMap.first;
            const std::string renderID = render->getID();
            m_layersBox->addItem(QString::fromStdString(renderID + " : " + id));
            m_layers.push_back(layerMap.second);

            m_connections->connect(layerMap.second, ::fwRenderOgre::Layer::s_INIT_LAYER_SIG,
                                   this->getSptr(), s_INIT_COMPOSITOR_LIST_SLOT);
        }
        nbRenderer++;
    }
}

//------------------------------------------------------------------------------

void SCompositorSelector::synchroniseWithLayerCompositorChain()
{
    m_layerCompositorChain = m_currentLayer->getCompositorChain();
}

//------------------------------------------------------------------------------

void SCompositorSelector::updateCompositorList()
{
    // Retrieving all compositors
    ::Ogre::ResourceManager::ResourceMapIterator iter = ::Ogre::CompositorManager::getSingleton().getResourceIterator();
    while(iter.hasMoreElements())
    {
        ::Ogre::ResourcePtr compositor = iter.getNext();
        if (compositor->getGroup() == ::material::s_COMPOSITOR_RESOURCEGROUP_NAME)
        {
            QString compositorName = compositor.getPointer()->getName().c_str();
            m_currentLayer->addAvailableCompositor(compositorName.toStdString());

            QListWidgetItem* newCompositor = new QListWidgetItem(compositorName, m_compositorChain);
            newCompositor->setFlags(newCompositor->flags() | ::Qt::ItemIsUserCheckable);
            newCompositor->setCheckState(::Qt::Unchecked);
        }
    }
}

//------------------------------------------------------------------------------

void SCompositorSelector::checkEnabledCompositors()
{
    if(!m_layerCompositorChain.empty())
    {
        for(int i(0); i < m_compositorChain->count(); ++i)
        {
            QListWidgetItem* currentCompositor = m_compositorChain->item(i);
            std::string currentCompositorName  = currentCompositor->text().toStdString();

            auto layerCompositor = std::find_if(m_layerCompositorChain.begin(),
                                                m_layerCompositorChain.end(),
                                                ::fwRenderOgre::CompositorChainManager::FindCompositorByName(
                                                    currentCompositorName));

            if(layerCompositor != m_layerCompositorChain.end())
            {
                if(layerCompositor->second)
                {
                    currentCompositor->setCheckState(::Qt::Checked);
                    m_currentLayer->updateCompositorState(currentCompositor->text().toStdString(), true);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------

void SCompositorSelector::uncheckCompositors()
{
    for(int i(0); i < m_compositorChain->count(); ++i)
    {
        QListWidgetItem* currentCompositor = m_compositorChain->item(i);
        currentCompositor->setCheckState(::Qt::Unchecked);
    }
}

//------------------------------------------------------------------------------

bool SCompositorSelector::isEnabledCompositor(std::string compositorName)
{
    auto layerCompositor = std::find_if(m_layerCompositorChain.begin(),
                                        m_layerCompositorChain.end(),
                                        ::fwRenderOgre::CompositorChainManager::FindCompositorByName(
                                            compositorName));

    if(layerCompositor != m_layerCompositorChain.end())
    {
        return layerCompositor->second;
    }

    return false;
}

//------------------------------------------------------------------------------

} // namespace uiVisuOgre
