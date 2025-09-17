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

#pragma once

#include <map>
#include <string>

#include "keyple/core/plugin/spi/PluginFactorySpi.hpp"
#include "keyple/core/plugin/spi/PluginSpi.hpp"
#include "keyple/core/util/cpp/Pattern.hpp"
#include "keyple/plugin/pcsc/PcscPluginFactory.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::plugin::spi::PluginFactorySpi;
using keyple::core::plugin::spi::PluginSpi;
using keyple::core::util::cpp::Pattern;

/**
 * Factory of PcscPlugin.
 *
 * @since 2.0.0
 */
class PcscPluginFactoryAdapter final
: public PcscPluginFactory, public PluginFactorySpi {
public:
    /**
     * (package-private)<br>
     * The plugin name
     *
     * @since 2.0.0
     */
    static const std::string PLUGIN_NAME;

    /**
     * (package-private)<br>
     * Creates an instance, sets the fields from the factory builder.
     *
     * @since 2.0.0
     */
    PcscPluginFactoryAdapter(
        const std::shared_ptr<Pattern> contactlessReaderIdentificationFilterPattern,
        const std::map<std::string, std::string>& protocolRulesMap,
        const int cardMonitoringCycleDuration);

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string getPluginApiVersion() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string getCommonApiVersion() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    const std::string& getPluginName() const override;

    /**
     * {@inheritDoc}
     *
     * @since 2.0.0
     */
    std::shared_ptr<PluginSpi> getPlugin() override;

private:
    /**
     *
     */
    const std::map<std::string, std::string> mProtocolRulesMap;

    /**
     *
     */
    const std::shared_ptr<Pattern>
        mContactlessReaderIdentificationFilterPattern;

    /**
     *
     */
    const int mCardMonitoringCycleDuration;
};

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
