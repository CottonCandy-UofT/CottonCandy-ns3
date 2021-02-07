#ifndef COTTONCANDY_ADDRESS_GENERATOR_H
#define COTTONCANDY_ADDRESS_GENERATOR_H

#include "ns3/cottoncandy-address.h"
#include "ns3/object.h"

namespace ns3{
namespace lorawan{


class CottoncandyAddressGenerator : public Object{
public:
  static TypeId GetTypeId (void);

  CottoncandyAddressGenerator(const uint16_t deviceAddr = 0x0001, const uint16_t gatwayAddr = 0x8000);

  CottoncandyAddress NextDeviceAddress();

  CottoncandyAddress NextGatewayAddress();

private:
  uint16_t m_deviceAddr;
  uint16_t m_gatewayAddr;
};


}
}
#endif