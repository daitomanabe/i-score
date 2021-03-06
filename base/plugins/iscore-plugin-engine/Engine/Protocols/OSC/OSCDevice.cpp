#include <QDebug>
#include <QString>
#include <QVariant>
#include <memory>

#include <Device/Protocol/DeviceSettings.hpp>
#include <ossia/network/generic/generic_device.hpp>
#include <ossia/network/generic/generic_address.hpp>
#include <ossia/network/osc/osc.hpp>
#include "OSCDevice.hpp"
#include <Engine/Protocols/OSC/OSCSpecificSettings.hpp>

namespace Engine
{
namespace Network
{
OSCDevice::OSCDevice(const Device::DeviceSettings &settings):
    OwningOSSIADevice{settings}
{
    using namespace ossia;

    reconnect();
}

bool OSCDevice::reconnect()
{
    disconnect();

    try {
        auto stgs = settings().deviceSpecificSettings.value<OSCSpecificSettings>();
        std::unique_ptr<ossia::net::protocol_base> ossia_settings = std::make_unique<ossia::net::osc_protocol>(
                    stgs.host.toStdString(),
                    stgs.inputPort,
                    stgs.outputPort);
        m_dev = std::make_unique<ossia::net::generic_device>(
                    std::move(ossia_settings),
                    settings().name.toStdString());
        setLogging_impl(isLogging());
    }
    catch(...)
    {
        ISCORE_TODO;
    }

    return connected();
}
}
}
