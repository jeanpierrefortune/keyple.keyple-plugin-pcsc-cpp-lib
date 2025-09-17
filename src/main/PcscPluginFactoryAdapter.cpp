/******************************************************************************
 * Copyright (c) 2025 Calypso Networks Association https://calypsonet.org/    *
 *                                                                            *
 * See the NOTICE file(s) distributed with this work for additional           *
 * information regarding copyright ownership.                                 *
 *                                                                            *
 * This program and the accompanying materials are made available under the   *
 * terms of the Eclipse Public License 2.0 which is available at              *
 * http://www.eclipse.org/legal/epl-2.0                                       *
 *                                                                            *
 * SPDX-License-Identifier: EPL-2.0                                           *
 ******************************************************************************/

#include "keyple/plugin/pcsc/PcscPluginFactoryAdapter.hpp"

#include "keyple/core/common/CommonApiProperties.hpp"
#include "keyple/core/plugin/PluginApiProperties.hpp"
#include "keyple/plugin/pcsc/PcscPluginAdapter.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactory.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::common::CommonApiProperties_VERSION;
using keyple::core::plugin::PluginApiProperties_VERSION;

const std::string PcscPluginFactoryAdapter::PLUGIN_NAME = "PcscPlugin";

PcscPluginFactoryAdapter::PcscPluginFactoryAdapter(
    const std::shared_ptr<Pattern> contactlessReaderIdentificationFilterPattern,
    const std::map<std::string, std::string>& protocolRulesMap,
    const int cardMonitoringCycleDuration)
: mProtocolRulesMap(protocolRulesMap)
, mContactlessReaderIdentificationFilterPattern(
      contactlessReaderIdentificationFilterPattern)
, mCardMonitoringCycleDuration(cardMonitoringCycleDuration)
{
}

const std::string
PcscPluginFactoryAdapter::getPluginApiVersion() const
{
    return PluginApiProperties_VERSION;
}

const std::string
PcscPluginFactoryAdapter::getCommonApiVersion() const
{
    return CommonApiProperties_VERSION;
}

const std::string&
PcscPluginFactoryAdapter::getPluginName() const
{
    return PLUGIN_NAME;
}

std::shared_ptr<PluginSpi>
PcscPluginFactoryAdapter::getPlugin()
{
    std::shared_ptr<PcscPluginAdapter> plugin
        = PcscPluginAdapter::getInstance();

    plugin
        ->setContactlessReaderIdentificationFilterPattern(
            mContactlessReaderIdentificationFilterPattern)
        .addProtocolRulesMap(mProtocolRulesMap)
        .setCardMonitoringCycleDuration(mCardMonitoringCycleDuration);

    return plugin;
}

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
