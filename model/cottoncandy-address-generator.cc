#include "ns3/cottoncandy-address-generator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyAddressGenerator");

TypeId
CottoncandyAddressGenerator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CottoncandyAddressGenerator")
    .SetParent<Object> ()
    .SetGroupName ("lorawan")
    .AddConstructor<CottoncandyAddressGenerator> ();
  return tid;
}

CottoncandyAddressGenerator::CottoncandyAddressGenerator (const uint16_t deviceAddr,
                                                          const uint16_t gatwayAddr)
{
  NS_LOG_FUNCTION (this << unsigned (deviceAddr) << unsigned (gatwayAddr));
  m_deviceAddr = deviceAddr;
  m_gatewayAddr = gatwayAddr;
}

CottoncandyAddress
CottoncandyAddressGenerator::NextGatewayAddress (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  CottoncandyAddress addr = CottoncandyAddress (m_gatewayAddr);

  m_gatewayAddr++;

  return addr;
}

CottoncandyAddress
CottoncandyAddressGenerator::NextDeviceAddress (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  CottoncandyAddress addr = CottoncandyAddress (m_deviceAddr);

  m_deviceAddr++;
  return addr;
}

} // namespace lorawan
} // namespace ns3
