#pragma once

#include <cstdint>
#include <span>

// Tracks which transport (WebUSB, WebSerial/CDC-ACM, or the bridge UART to a
// companion MCU) the device should currently send status and data back on, and
// provides a dispatch layer so call sites don't need to know which transport
// is active.
namespace Transport
{
    enum class Id : uint8_t { Usb, Serial, BridgeUart };

    using ReceiveHandler = void(*)(std::span<const uint8_t>, void*);

    // Each transport layer calls this from its receive path when a host frame
    // arrives, so the device replies on the same transport the host used.
    void setActive(Id id);
    Id active();

    // The host whose frame is being handled (setActive) starts a mode: the
    // mode's status and data now go to it, until another host starts one.
    // So a second host only querying the adapter -- a launcher polling
    // GetFirmwareInfo while a companion MCU runs it over the bridge UART --
    // neither takes the mode's traffic nor gets it.
    void claimForMode();
    Id owner();

    // Register the same handler on every transport so the device accepts host
    // commands/data regardless of which one is open.
    void registerCommandHandler(ReceiveHandler handler, void* userData);
    void registerDataHandler(ReceiveHandler handler, void* userData);

    // A running mode's status and data: to the host that started it.
    bool sendStatus(std::span<const uint8_t, 2> data);
    bool sendData(std::span<const uint8_t> data);

    // The answer to the command being handled: to the host that sent it.
    bool sendReply(std::span<const uint8_t> data);
}
