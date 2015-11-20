/* ***** BEGIN LICENSE BLOCK *****
 * FW4SPL - Copyright (C) IRCAD, 2014-2015.
 * Distributed under the terms of the GNU Lesser General Public License (LGPL) as
 * published by the Free Software Foundation.
 * ****** END LICENSE BLOCK ****** */

#ifndef __UIVISUOGRE_PLUGIN_HPP__
#define __UIVISUOGRE_PLUGIN_HPP__

#include <fwRuntime/Plugin.hpp>

#include "uiVisuOgre/config.hpp"

namespace uiVisuOgre
{
/**
 * @brief   This class is started when the bundles is loaded.
 * @class   Plugin
 */
struct UIVISUOGRE_CLASS_API Plugin : public ::fwRuntime::Plugin
{
    /**
     * @brief   destructor
     */
    UIVISUOGRE_API ~Plugin() throw();

    // Overrides
    UIVISUOGRE_API void start() throw(::fwRuntime::RuntimeException);

    // Overrides
    UIVISUOGRE_API void stop() throw();

};

} // namespace uiVisuOgre

#endif //__UIVISUOGRE_PLUGIN_HPP__
