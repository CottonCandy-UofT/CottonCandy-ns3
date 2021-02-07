/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COTTONCANDY_MAC_HEADER_H
#define COTTONCANDY_MAC_HEADER_H

#include "ns3/header.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the Mac header of a LoRaWAN packet.
 */
class CottoncandyMacHeader : public Header
{
public:

  static TypeId GetTypeId (void);
  CottoncandyMacHeader();
  ~CottoncandyMacHeader();

  enum MsgType{
    JOIN = 1,
    JOIN_ACK = 2,
    JOIN_CFM = 3,
    GATEWAY_REQ = 6,
    NODE_REPLY = 7
  };

  virtual TypeId GetInstanceTypeId(void) const;

  virtual uint32_t GetSerializedSize(void) const;

  virtual void Serialize(Buffer::Iterator start) const;

  virtual uint32_t Deserialize(Buffer::Iterator start);

  virtual void Print (std::ostream &os) const;

  void SetType(enum MsgType type);

  void SetSrc(uint16_t src);

  void SetDest(uint16_t dest);

  uint8_t GetType() const;

  uint16_t GetSrc() const;

  uint16_t GetDest() const;

private:
  uint8_t m_mtype;
  
  uint16_t m_srcAddr;

  uint16_t m_destAddr; 

};
}

}
#endif
