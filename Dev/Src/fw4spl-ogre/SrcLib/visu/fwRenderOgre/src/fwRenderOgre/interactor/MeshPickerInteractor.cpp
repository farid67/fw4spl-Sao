/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "fwRenderOgre/interactor/MeshPickerInteractor.hpp"

#include <fwCom/Signal.hxx>

#include <fwRenderOgre/registry/macros.hpp>

#include <fwRenderOgre/interactor/IInteractor.hpp>

fwRenderOgreRegisterInteractorMacro( ::fwRenderOgre::interactor::MeshPickerInteractor );

namespace fwRenderOgre
{

namespace interactor
{

MeshPickerInteractor::MeshPickerInteractor() throw()
{
}

//------------------------------------------------------------------------------

MeshPickerInteractor::~MeshPickerInteractor() throw()
{
}

//------------------------------------------------------------------------------

void MeshPickerInteractor::mouseClickEvent(int x, int y, int width, int height)
{
    if(m_picker->hasSceneManager())
    {
        if(m_picker->executeRaySceneQuery(x, y, width, height))
        {
            ::Ogre::Vector3 click = m_picker->getIntersectionInWorldSpace();

            ::fwData::Point::sptr point                = fwData::Point::New();
            ::fwData::Point::PointCoordArrayType cords = {{click.x,click.y,click.z}};
            point->setCoord(cords);

            m_sigPointClicked->asyncEmit(::fwData::Object::dynamicCast(point));
        }
    }
    else
    {
        SLM_WARN("The picker scene hasn't been initialized, you are not using this interactor correctly");
    }
}

//------------------------------------------------------------------------------

} //namespace interactor
} //namespace fwRenderOgre


