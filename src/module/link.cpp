#include "link.hpp"
#include "../linkStatus.hpp"
#include "../callbacks/commands.hpp"

#include "syscalls/kernel.h"
#include "zephyr/kernel.h"
#include <zephyr/irq.h>


void LinkModule::execute()
{
    m_cancel = false;
    sendLinkStatus(LinkStatus::AwaitMode);
    k_sem_take(&m_waitForLinkModeCommand, K_FOREVER);
    bool keepAlive = true;

    while (keepAlive && !m_cancel)
    {
        {
            UsbSection section(m_packetLayerMode);
            m_currentSection = &section;
            keepAlive = section.process();
        }
        m_currentSection = nullptr; //kinda sketch but commands should only arrive when we have a current section

        if (keepAlive)
        {
            sendLinkStatus(LinkStatus::LinkReconnecting);
            k_sleep(K_MSEC(400));
        }
    }
}

void LinkModule::receiveCommand(std::span<const uint8_t> command)
{
    switch (static_cast<LinkModeCommand>(command[0]))
    {
        case LinkModeCommand::SetModeMaster:
            m_packetLayerMode = PacketLayer::Mode::master;
            k_sem_give(&m_waitForLinkModeCommand);
            break;
        case LinkModeCommand::SetModeSlave:
            m_packetLayerMode = PacketLayer::Mode::slave;
            k_sem_give(&m_waitForLinkModeCommand);            
            break;
        // There is no section before SetModeMaster/SetModeSlave, nor during the
        // 400 ms pause between sections; a host retrying at either moment
        // must not fault the adapter.
        case LinkModeCommand::StartHandshake:
            if (UsbSection* s = m_currentSection) s->startHandshake();
            return;
        case LinkModeCommand::ConnectLink:
            if (UsbSection* s = m_currentSection) s->connectLink();
            return;
        default: break;
    }
}

