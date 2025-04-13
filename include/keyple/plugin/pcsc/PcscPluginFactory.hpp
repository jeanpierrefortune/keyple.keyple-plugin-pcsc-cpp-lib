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

#include "keyple/core/common/KeyplePluginExtensionFactory.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::common::KeyplePluginExtensionFactory;

/**
 * PC/SC specific KeyplePluginExtensionFactory to be provided to the Keyple SmartCard
 * service to register the PC/SC plugin, built by PcscPluginFactoryBuilder.
 *
 * @since 2.0.0
 */
class PcscPluginFactory : public KeyplePluginExtensionFactory {
public:
    /**
     *
     */
    virtual ~PcscPluginFactory() = default;
};

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
