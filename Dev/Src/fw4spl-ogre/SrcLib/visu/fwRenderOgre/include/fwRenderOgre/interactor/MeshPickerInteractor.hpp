/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __FWRENDEROGRE_INTERACTOR_MESHPICKERINTERACTOR_HPP__
#define __FWRENDEROGRE_INTERACTOR_MESHPICKERINTERACTOR_HPP__

#include <fwRenderOgre/interactor/IPickerInteractor.hpp>
#include <fwRenderOgre/config.hpp>
#include <fwRenderOgre/picker/IPicker.hpp>

#include <fwCom/Signal.hpp>
#include <fwCom/Signals.hpp>
#include <fwCom/Slots.hpp>
#include <fwCom/Slot.hpp>

#include <fwData/Point.hpp>

#include <OGRE/OgreSceneManager.h>

namespace fwRenderOgre
{

namespace interactor
{

/**
 * @class MeshPickerInteractor
 * @brief Implementation for selection of points on Mesh
 */
class FWRENDEROGRE_CLASS_API MeshPickerInteractor : public ::fwRenderOgre::interactor::IPickerInteractor
{

public:

    /// Constructor.
    FWRENDEROGRE_API MeshPickerInteractor() throw();

    /// Destructor. Does nothing
    FWRENDEROGRE_API ~MeshPickerInteractor() throw();

protected:

    /**
     * @brief Mouse's left button pressed.
     *        The picker will do ray cast command.
     * @param x The mouse's X displacement
     * @param y The mouse's Y displacement
     * @param width the render window width
     * @param height the render window height
     */
    FWRENDEROGRE_API void mouseClickEvent(int x, int y, int width, int height);
};
} //namespace itneractor
} //namespace fwRenderOgre

#endif // __FWRENDEROGRE_INTERACTOR_MESHPICKERINTERACTOR_HPP__
