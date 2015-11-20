/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __VISUOGREADAPTOR_SINTERACTORSTYLE_HPP__
#define __VISUOGREADAPTOR_SINTERACTORSTYLE_HPP__

#include <fwRenderOgre/IAdaptor.hpp>

#include <fwCom/Signal.hpp>
#include <fwCom/Signals.hpp>
#include <fwCom/Slot.hpp>
#include <fwCom/Slots.hpp>

#include <fwData/Object.hpp>

#include <visuOgreAdaptor/config.hpp>

namespace visuOgreAdaptor
{

/**
 * @brief   Managing interactor style for Ogre
 * @class   SInteractorStyle
 */
class VISUOGREADAPTOR_CLASS_API SInteractorStyle : public ::fwRenderOgre::IAdaptor
{

public:

    fwCoreServiceClassDefinitionsMacro((SInteractorStyle)(::fwRenderOgre::IAdaptor));

    /**
     * @name Signals API
     * @{
     */

    typedef ::fwCom::Signal< void ( ::fwData::Object::sptr ) > PointClickedSignalType;
    VISUOGREADAPTOR_API static const ::fwCom::Signals::SignalKeyType s_POINT_CLICKED_SIG;

    /** @} */

    /**
     * @name Slots API
     * @{
     */

    VISUOGREADAPTOR_API static const ::fwCom::Slots::SlotKeyType s_POINT_CLICKED_SLOT;

    /** @} */

    /// Constructor. Creates signals and slots
    VISUOGREADAPTOR_API SInteractorStyle() throw();

    /// Destructor. Does nothing
    VISUOGREADAPTOR_API virtual ~SInteractorStyle() throw();

protected:

    /**
     * @brief Configuring method.
     * Catch interaptor style
     * @verbatim
         <adaptor id="interactorAdaptor" class="::visuOgreAdaptor::SInteractorStyle" objectId="self">
             <config render="layerID" type="InteractionType" style="InteractorStyle" />
         </adaptor>
       @endverbatim
     * - \b render Layer on which the interactions will be done
     * - \b type Type of the interactor either Movement or Selection
     * - \b Style Style of the interactor depending on the type. if \b Movement, Default or Fixed can be used,
     * if \b Selection, Mesh or Video can be used.

     */
    VISUOGREADAPTOR_API void doConfigure() throw(fwTools::Failed);

    /// Starting method
    VISUOGREADAPTOR_API void doStart() throw(fwTools::Failed);

    /// Update the interactor
    VISUOGREADAPTOR_API void doUpdate() throw(fwTools::Failed);

    /// Stopping method
    VISUOGREADAPTOR_API void doStop() throw(fwTools::Failed);

    /// Swaping method, do nothing
    VISUOGREADAPTOR_API void doSwap() throw(fwTools::Failed);

private:

    /**
     * @name Slots methods
     * @{
     */

    /// SLOT : sends a signal when the interactor has recieved a clicked point signal
    void clickedPoint(fwData::Object::sptr obj);

    /**
     * @}
     */

    /// Set interactor style
    void setInteractorStyle();

    /// Type of the configured style
    std::string m_configuredStyle;

    /**
     * @name Signals attributes
     * @{
     */

    /// Pointer to the generic signal
    PointClickedSignalType::sptr m_sigPointClicked;

    /**
     * @}
     */

    ///Connection service, needed for slot/signal association
    ::fwServices::helper::SigSlotConnection::sptr m_connections;

    /// map containing all the classes associated to their xml designations (e.g. Default -> TrackballInteractor)
    std::map<std::string, std::string> m_interactorStyles;
};

} //namespace visuOgreAdaptor

#endif // __VISUOGREADAPTOR_SINTERACTORSTYLE_HPP__
