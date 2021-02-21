/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-mac-helper.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/lora-net-device.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyMacHelper");

CottoncandyMacHelper::CottoncandyMacHelper ()
{
}

void
CottoncandyMacHelper::Set (std::string name, const AttributeValue &v)
{
  m_mac.Set (name, v);
}

void
CottoncandyMacHelper::SetAddressGenerator (Ptr<CottoncandyAddressGenerator> generator)
{
  m_addrGenerator = generator;
}

void
CottoncandyMacHelper::SetDeviceType (enum DeviceType dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_mac.SetTypeId ("ns3::CottoncandyMac");
  m_deviceType = dt;
}

void
CottoncandyMacHelper::SetRegion (enum CottoncandyMacHelper::Regions region)
{
  m_region = region;
}

Ptr<CottoncandyMac>
CottoncandyMacHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
{
  Ptr<CottoncandyMac> mac = m_mac.Create<CottoncandyMac> ();
  mac->SetDevice (device);

  NS_ASSERT(m_addrGenerator!=0);

  // If we are operating on an end device, add an address to it
  if (m_deviceType == ED_A)
    {
      CottoncandyAddress addr = m_addrGenerator->NextDeviceAddress();
      mac->GetObject<CottoncandyMac> ()->SetDeviceAddress (addr);
    }
  else
    {
      CottoncandyAddress addr = m_addrGenerator->NextGatewayAddress();
      mac->GetObject<CottoncandyMac> ()->SetDeviceAddress (addr);
    }

  // Add a basic list of channels based on the region where the device is
  // operating
  /*
  if (m_deviceType == ED_A)
    {
      Ptr<ClassAEndDeviceLorawanMac> edMac = mac->GetObject<ClassAEndDeviceLorawanMac> ();
      switch (m_region)
        {
          case CottoncandyMacHelper::EU: {
            ConfigureForEuRegion (edMac);
            break;
          }
          case CottoncandyMacHelper::SingleChannel: {
            ConfigureForSingleChannelRegion (edMac);
            break;
          }
          case CottoncandyMacHelper::ALOHA: {
            ConfigureForAlohaRegion (edMac);
            break;
          }
          default: {
            NS_LOG_ERROR ("This region isn't supported yet!");
            break;
          }
        }
    }
  else
    {
      Ptr<GatewayLorawanMac> gwMac = mac->GetObject<GatewayLorawanMac> ();
      switch (m_region)
        {
          case CottoncandyMacHelper::EU: {
            ConfigureForEuRegion (gwMac);
            break;
          }
          case CottoncandyMacHelper::SingleChannel: {
            ConfigureForSingleChannelRegion (gwMac);
            break;
          }
          case CottoncandyMacHelper::ALOHA: {
            ConfigureForAlohaRegion (gwMac);
            break;
          }
          default: {
            NS_LOG_ERROR ("This region isn't supported yet!");
            break;
          }
        }
    }
    */

  LogicalLoraChannelHelper channelHelper;
  channelHelper.AddSubBand (900, 915.6, 1, 17);

  //////////////////////
  // Default channels //
  //////////////////////
  Ptr<LogicalLoraChannel> lc1 = CreateObject<LogicalLoraChannel> (915.0, 0, 5);
  channelHelper.AddChannel (lc1);

  mac->SetLogicalLoraChannelHelper (channelHelper);
  return mac;
}

} // namespace lorawan
} // namespace ns3
