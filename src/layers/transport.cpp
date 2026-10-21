#include "transport.hpp"

#include "usbLayer.hpp"
#include "serialLayer.hpp"

namespace Transport
{
    namespace
    {
        // The host whose frame is being handled: replies go back to it.
        Id g_active = Id::Usb;
        // The host whose SetMode started the running mode: the mode's status
        // and data go to it, whichever host spoke since.
        Id g_owner = Id::Usb;

        bool sendStatusTo(Id id, std::span<const uint8_t, 2> data)
        {
            if (id == Id::Serial) return SerialLayer::getInstance().sendStatus(data);
            if (id == Id::BridgeUart) return SerialLayer::getBridgeInstance().sendStatus(data);
            return UsbLayer::getInstance().sendStatus(data);
        }

        bool sendDataTo(Id id, std::span<const uint8_t> data)
        {
            if (id == Id::Serial) return SerialLayer::getInstance().sendData(data);
            if (id == Id::BridgeUart) return SerialLayer::getBridgeInstance().sendData(data);
            return UsbLayer::getInstance().sendData(data);
        }
    }

    void setActive(Id id) { g_active = id; }
    Id active() { return g_active; }

    void claimForMode() { g_owner = g_active; }
    Id owner() { return g_owner; }

    void registerCommandHandler(ReceiveHandler handler, void* userData)
    {
        UsbLayer::getInstance().setReceiveCommandHandler(handler, userData);
        SerialLayer::getInstance().setReceiveCommandHandler(handler, userData);
        SerialLayer::getBridgeInstance().setReceiveCommandHandler(handler, userData);
    }

    void registerDataHandler(ReceiveHandler handler, void* userData)
    {
        UsbLayer::getInstance().setReceiveDataHandler(handler, userData);
        SerialLayer::getInstance().setReceiveDataHandler(handler, userData);
        SerialLayer::getBridgeInstance().setReceiveDataHandler(handler, userData);
    }

    bool sendStatus(std::span<const uint8_t, 2> data) { return sendStatusTo(g_owner, data); }
    bool sendData(std::span<const uint8_t> data) { return sendDataTo(g_owner, data); }
    bool sendReply(std::span<const uint8_t> data) { return sendDataTo(g_active, data); }
}
