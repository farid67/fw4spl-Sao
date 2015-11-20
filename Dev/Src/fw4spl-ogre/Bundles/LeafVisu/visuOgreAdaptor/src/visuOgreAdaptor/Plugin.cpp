/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#include "visuOgreAdaptor/Plugin.hpp"

#include <fwRenderOgre/Utils.hpp>

#include <fwRuntime/utils/GenericExecutableFactoryRegistrar.hpp>

#include <fwServices/Base.hpp>
#include <fwServices/macros.hpp>

namespace visuOgreAdaptor
{

using namespace ::fwRuntime::utils;

static GenericExecutableFactoryRegistrar<Plugin> registrar("visuOgreAdaptor::Plugin");

Plugin::~Plugin() throw()
{
}

void Plugin::start() throw(::fwRuntime::RuntimeException)
{
}

void Plugin::stop() throw()
{
}

} // namespace visuOgreAdaptor
