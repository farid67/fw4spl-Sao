/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include <fwRenderOgre/interactor/VideoPickerInteractor.hpp>
#include <fwRenderOgre/registry/macros.hpp>
#include <fwRenderOgre/interactor/IInteractor.hpp>

#include <fwCom/Signal.hxx>

fwRenderOgreRegisterInteractorMacro( ::fwRenderOgre::interactor::VideoPickerInteractor );

namespace fwRenderOgre
{

namespace interactor
{

VideoPickerInteractor::VideoPickerInteractor() throw()
{
}

//------------------------------------------------------------------------------

VideoPickerInteractor::~VideoPickerInteractor() throw()
{
}

//------------------------------------------------------------------------------

void VideoPickerInteractor::mouseClickEvent(int x, int y, int width, int height)
{
    if(m_picker->hasSceneManager())
    {
        if( m_picker->executeRaySceneQuery(x, y, width, height) )
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


