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

#include "keyple/plugin/pcsc/PcscReaderAdapter.hpp"

#include "keyple/core/plugin/CardIOException.hpp"
#include "keyple/core/plugin/ReaderIOException.hpp"
#include "keyple/core/plugin/TaskCanceledException.hpp"
#include "keyple/core/util/HexUtil.hpp"
#include "keyple/core/util/cpp/exception/Exception.hpp"
#include "keyple/core/util/cpp/exception/IllegalArgumentException.hpp"
#include "keyple/core/util/cpp/exception/IllegalStateException.hpp"
#include "keyple/plugin/pcsc/PcscPluginAdapter.hpp"
#include "keyple/plugin/pcsc/cpp/exception/CardException.hpp"

namespace keyple {
namespace plugin {
namespace pcsc {

using keyple::core::plugin::CardIOException;
using keyple::core::plugin::ReaderIOException;
using keyple::core::plugin::TaskCanceledException;
using keyple::core::util::HexUtil;
using keyple::core::util::cpp::exception::Exception;
using keyple::core::util::cpp::exception::IllegalArgumentException;
using keyple::core::util::cpp::exception::IllegalStateException;
using keyple::plugin::pcsc::cpp::exception::CardException;

PcscReaderAdapter::PcscReaderAdapter(
    std::shared_ptr<CardTerminal> terminal,
    std::shared_ptr<PcscPluginAdapter> pluginAdapter,
    const int cardMonitoringCycleDuration)
: mIsInitialized(false)
, mIsPhysicalChannelOpen(false)
, mTerminal(terminal)
, mName(terminal->getName())
, mPluginAdapter(pluginAdapter)
, mCardMonitoringCycleDuration(cardMonitoringCycleDuration)
, mIsContactless(false)
, mIsModeExclusive(false)
, mLoopWaitCard(false)
, mLoopWaitCardRemoval(false)
{
#if defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
    mIsWindows = true;
#else
    mIsWindows = false;
#endif
}

void
PcscReaderAdapter::waitForCardInsertion()
{
    mLogger->trace(
        "Reader [%]: start waiting card insertion (loop latency: % ms)\n",
        getName(),
        mCardMonitoringCycleDuration);

    /* Activate loop */
    mLoopWaitCard = true;

    try {
        while (mLoopWaitCard) {
            if (mTerminal->waitForCardPresent(mCardMonitoringCycleDuration)) {
                /* Card inserted */
                mLogger->trace("Reader [%]: card inserted\n", getName());
                return;
            }

            // if (Thread.interrupted()) {
            //     break;
            // }
        }

        mLogger->trace(
            "Reader [%]: waiting card insertion stopped\n", getName());

    } catch (const CardException& e) {
        /* Here, it is a communication failure with the reader */
        throw ReaderIOException(
            mName + ": an error occurred while waitœing for a card insertion.",
            std::make_shared<CardException>(e));
    }

    throw TaskCanceledException(
        mName + ": the wait for a card insertion task has been cancelled.");
}

void
PcscReaderAdapter::stopWaitForCardInsertion()
{
    mLoopWaitCard = false;
}

bool
PcscReaderAdapter::isProtocolSupported(const std::string& readerProtocol) const
{
    /* C++ specific: getProtocolRule throws when protocol not found instead of
     * returning null */
    try {
        const std::string rulmIsPhysicalChannelOpen
            = mPluginAdapter->getProtocolRule(readerProtocol);

        return true;

    } catch (const IllegalArgumentException&) {
        /* Do nothing */
    }

    return false;
}

void
PcscReaderAdapter::activateProtocol(const std::string& readerProtocol)
{
    mLogger->trace(
        "%: stop waiting for card insertion requested.\n", getName());
    mLogger->trace(
        "Reader [%]: activating protocol [%] takes no action\n",
        getName(),
        readerProtocol);

    mLoopWaitCard = false;
}

void
PcscReaderAdapter::deactivateProtocol(const std::string& readerProtocol)
{
    mLogger->trace(
        "Reader [%]: de-activating protocol [%] takes no action\n",
        getName(),
        readerProtocol);
}

bool
PcscReaderAdapter::isCurrentProtocol(const std::string& readerProtocol) const
{
    /*
     * C++ specific: getProtocolRule throws when protocol not found instead of
     * returning null.
     */
    try {
        bool isCurrentProtocol = false;

        const std::string protocolRule
            = mPluginAdapter->getProtocolRule(readerProtocol);
        if (!protocolRule.empty()) {
            const std::string atr = HexUtil::toHex(mTerminal->getATR());
            isCurrentProtocol
                = Pattern::compile(protocolRule)->matcher(atr)->matches();
        }

        return isCurrentProtocol;

    } catch (const IllegalArgumentException&) {
        /* Do nothing */
    }

    return false;
}

void
PcscReaderAdapter::onStartDetection()
{
    /* Nothing to do here in this reader */
}

void
PcscReaderAdapter::onStopDetection()
{
    /* Nothing to do here in this reader */
}

const std::string&
PcscReaderAdapter::getName() const
{
    return mName;
}

void
PcscReaderAdapter::openPhysicalChannel()
{
    if (mIsPhysicalChannelOpen == true) {
        return;
    }

    /*
     * Init of the card physical channel: if not yet established, opening of a
     * new physical channel.
     */
    try {
        mLogger->debug(
            "Reader [%]: open card physical channel for protocol [%]\n",
            getName(),
            mProtocol);

        mTerminal->openAndConnect(mProtocol);
        if (mIsModeExclusive) {
            mTerminal->beginExclusive();
            mLogger->debug(
                "Reader [%]: open card physical channel in exclusive mode\n",
                getName());

        } else {
            mLogger->debug(
                "%: opening of a card physical channel in shared mode\n",
                getName());
        }

        mIsPhysicalChannelOpen = true;

    } catch (const CardException& e) {
        throw ReaderIOException(
            getName() + ": Error while opening Physical Channel",
            std::make_shared<CardException>(e));
    }
}

void
PcscReaderAdapter::closePhysicalChannel()
{
    if (mIsPhysicalChannelOpen == false) {
        return;
    }

    try {
        if (mTerminal != nullptr) {
            mTerminal->closeAndDisconnect(mDisconnectionMode);
        }

    } catch (const CardException& e) {
        throw ReaderIOException(
            "Error while closing physical channel",
            std::make_shared<CardException>(e));
    }

    resetContext();
}

bool
PcscReaderAdapter::isPhysicalChannelOpen() const
{
    return mIsPhysicalChannelOpen;
}

bool
PcscReaderAdapter::checkCardPresence()
{
    try {
        const bool isCardPresent = mTerminal->isCardPresent(false);
        closePhysicalChannelSafely();

        return isCardPresent;

    } catch (const CardException& e) {
        throw ReaderIOException(
            "Exception occurred in isCardPresent",
            std::make_shared<CardException>(e));
    }
}

void
PcscReaderAdapter::closePhysicalChannelSafely()
{
    try {
        if (mIsPhysicalChannelOpen == true) {
            /*
             * Force reconnection next time.
             * Do not reset the card after disconnecting.
             */
            mTerminal->closeAndDisconnect(DisconnectionMode::LEAVE);
        }

    } catch (const Exception&) {
        /* Do nothing */
    }

    resetContext();
}

void
PcscReaderAdapter::resetContext()
{
    mIsPhysicalChannelOpen = false;
}

const std::string
PcscReaderAdapter::getPowerOnData() const
{
    return HexUtil::toHex(mTerminal->getATR());
}

const std::vector<uint8_t>
PcscReaderAdapter::transmitApdu(const std::vector<uint8_t>& apduCommandData)
{
    std::vector<uint8_t> apduResponseData;

    if (mIsPhysicalChannelOpen) {
        try {
            apduResponseData = mTerminal->transmitApdu(apduCommandData);

        } catch (const CardException& e) {
            if (e.getMessage().find("REMOVED") != std::string::npos) {
                throw CardIOException(
                    getName() + ":" + e.getMessage(),
                    std::make_shared<CardException>(e));
            } else {
                throw ReaderIOException(
                    getName() + ":" + e.getMessage(),
                    std::make_shared<CardException>(e));
            }

        } catch (const IllegalStateException& e) {
            /* Card could have been removed prematurely */
            throw CardIOException(
                getName() + ":" + e.getMessage(),
                std::make_shared<IllegalStateException>(e));
        }

    } else {
        /* Could occur if the card was removed */
        throw CardIOException(getName() + ": null channel.");
    }

    return apduResponseData;
}

bool
PcscReaderAdapter::isContactless()
{
    if (!mIsInitialized) {
        /*
         * First time initialisation, the transmission mode has not yet been
         * determined or fixed explicitly, let's ask the plugin to determine it
         * (only once)
         */
        mIsContactless = mPluginAdapter->isContactless(getName());
    }

    return mIsContactless;
}

void
PcscReaderAdapter::onUnregister()
{
    /* Nothing to do here in this reader */
}

void
PcscReaderAdapter::monitorCardPresenceDuringProcessing()
{
    waitForCardRemoval();
}

void
PcscReaderAdapter::stopCardPresenceMonitoringDuringProcessing()
{
    stopWaitForCardRemoval();
}

void
PcscReaderAdapter::waitForCardRemoval()
{
    mLogger->trace(
        "Reader [%]: start waiting card removal (loop latency: % ms)\n",
        mName,
        mCardMonitoringCycleDuration);

    /* Activate loop */
    mLoopWaitCardRemoval = true;

    try {
        while (mLoopWaitCardRemoval) {
            if (mTerminal->waitForCardAbsent(mCardMonitoringCycleDuration)) {
                /* Card removed */
                mLogger->trace("Reader [%]: card removed\n", mName);
                return;
            }

            // if (Thread.interrupted()) {
            //     break;
            // }
        }

        mLogger->trace("Reader [%]: waiting card removal stopped\n", mName);

    } catch (const CardException& e) {
        /* Here, it is a communication failure with the reader */
        throw ReaderIOException(
            mName + ": an error occurred while waiting for the card removal.",
            std::make_shared<CardException>(e));
    }

    throw TaskCanceledException(
        mName + ": the wait for the card removal task has been cancelled.");
}

void
PcscReaderAdapter::stopWaitForCardRemoval()
{
    mLoopWaitCardRemoval = false;
}

PcscReader&
PcscReaderAdapter::setSharingMode(const SharingMode sharingMode)
{
    mLogger->trace(
        "Reader [%]: set sharing mode to [%]\n", getName(), sharingMode);

    if (sharingMode == SharingMode::SHARED) {
        /* If a card is present, change the mode immediately */
        if (mIsPhysicalChannelOpen) {
            try {
                mTerminal->endExclusive();

            } catch (const CardException& e) {
                throw IllegalStateException(
                    "Couldn't disable exclusive mode",
                    std::make_shared<CardException>(e));
            }
        }

        mIsModeExclusive = false;

    } else if (sharingMode == SharingMode::EXCLUSIVE) {
        mIsModeExclusive = true;
    }

    return *this;
}

PcscReader&
PcscReaderAdapter::setContactless(const bool contactless)
{
    mLogger->trace(
        "Reader [%]: set contactless type to [%]\n", getName(), contactless);

    mIsContactless = contactless;
    mIsInitialized = true;

    return *this;
}

PcscReader&
PcscReaderAdapter::setIsoProtocol(const IsoProtocol& isoProtocol)
{
    mLogger->trace(
        "Reader [%]: set ISO protocol to [%] (%)\n",
        getName(),
        isoProtocol,
        isoProtocol.getValue());

    mProtocol = isoProtocol.getValue();

    return *this;
}

PcscReader&
PcscReaderAdapter::setDisconnectionMode(
    const DisconnectionMode disconnectionMode)
{
    mLogger->trace(
        "Reader [%]: set disconnection mode to [%]\n",
        getName(),
        disconnectionMode);

    mDisconnectionMode = disconnectionMode;

    return *this;
}

const std::vector<uint8_t>
PcscReaderAdapter::transmitControlCommand(
    const int commandId, const std::vector<uint8_t>& command)
{
    bool temporaryConnection = false;
    std::vector<uint8_t> response;
    const int controlCode
        = mIsWindows ? 0x00310000 | (commandId << 2) : 0x42000000 | commandId;

    try {
        if (mTerminal != nullptr) {

            if (!mTerminal->isConnected()) {
                temporaryConnection = true;
                mTerminal->openAndConnect("DIRECT");
            }

            response = mTerminal->transmitControlCommand(controlCode, command);

            if (temporaryConnection) {
                mTerminal->closeAndDisconnect(DisconnectionMode::LEAVE);
            }

        } else {
            /* C++ don't do virtual cards for now... */
            throw IllegalStateException("Reader failure.");
        }

    } catch (const CardException& e) {
        throw IllegalStateException(
            "Reader failure.", std::make_shared<CardException>(e));
    }

    return response;
}

int
PcscReaderAdapter::getIoctlCcidEscapeCommandId() const
{
    return mIsWindows ? 3500 : 1;
}

} /* namespace pcsc */
} /* namespace plugin */
} /* namespace keyple */
