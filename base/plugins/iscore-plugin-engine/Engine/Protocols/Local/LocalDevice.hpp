#pragma once
#include <Engine/Protocols/OSSIADevice.hpp>
#include <ossia/network/base/device.hpp>
#include <ossia/network/base/node.hpp>
#include <nano_observer.hpp>

namespace ossia
{
namespace net
{
class generic_device;
class local_protocol;
}
}
namespace Engine
{
namespace Network
{
class LocalDevice final :
        public OSSIADevice,
        public Nano::Observer
{
    public:
        LocalDevice(
                ossia::net::generic_device& dev,
                const iscore::DocumentContext& ctx,
                const Device::DeviceSettings& settings);

        ~LocalDevice();

        void setRemoteSettings(const Device::DeviceSettings&);

        ossia::net::device_base* getDevice() const override
        { return &m_dev; }

        void nodeCreated(const ossia::net::node_base& n) { OSSIADevice::nodeCreated(n); }
        void nodeRemoving(const ossia::net::node_base& n) { OSSIADevice::nodeRemoving(n); }
        void nodeRenamed(const ossia::net::node_base& n, std::string s) { OSSIADevice::nodeRenamed(n, s); }
        void addressCreated(const ossia::net::address_base& n) { OSSIADevice::addressCreated(n); }
        void addressUpdated(const ossia::net::address_base& n) { OSSIADevice::addressUpdated(n); }
    private:
        void disconnect() override;
        bool reconnect() override;

        Device::Node refresh() override;

        ossia::net::device_base& m_dev;
        ossia::net::local_protocol* m_proto;
        using OSSIADevice::refresh;
};
}
}
