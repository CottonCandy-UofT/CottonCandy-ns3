/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COTTONCANDY_GATEWAYREQ_HEADER_H
#define COTTONCANDY_GATEWAYREQ_HEADER_H

#include "ns3/header.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the extra header of a Join_ack packet.
 */
class CottoncandyGatewayReqHeader : public Header
{
public:

  static TypeId GetTypeId (void);
  CottoncandyGatewayReqHeader();
  ~CottoncandyGatewayReqHeader();

  virtual TypeId GetInstanceTypeId(void) const;

  virtual uint32_t GetSerializedSize(void) const;

  virtual void Serialize(Buffer::Iterator start) const;

  virtual uint32_t Deserialize(Buffer::Iterator start);

  virtual void Print (std::ostream &os) const;

  void SetSeqNumber(uint8_t seqNum);

  uint8_t GetSeqNumber() const;

  void SetNextReqTime(uint32_t nextReqTime);

  uint32_t GetNextReqTime() const;
    
  void SetMaxBackoff(uint8_t maxBackoff);

  uint8_t GetMaxBackoff() const;

private:
  uint8_t m_seqNum;
  uint32_t m_nextReqTime;
  uint8_t m_maxBackoff;

};
}

}
#endif
