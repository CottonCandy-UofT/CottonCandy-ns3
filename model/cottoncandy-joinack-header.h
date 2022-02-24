/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COTTONCANDY_JOINACK_HEADER_H
#define COTTONCANDY_JOINACK_HEADER_H

#include "ns3/header.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the extra header of a Join_ack packet.
 */
class CottoncandyJoinAckHeader : public Header
{
public:

  static TypeId GetTypeId (void);
  CottoncandyJoinAckHeader();
  ~CottoncandyJoinAckHeader();

  virtual TypeId GetInstanceTypeId(void) const;

  virtual uint32_t GetSerializedSize(void) const;

  virtual void Serialize(Buffer::Iterator start) const;

  virtual uint32_t Deserialize(Buffer::Iterator start);

  virtual void Print (std::ostream &os) const;

  void SetHops(uint8_t hops);

  void SetRssi(int m_rssiFeedback);

  void SetNumChildren(uint8_t m_numChidlren);

  uint8_t GetHops() const;

  int GetRssi() const;

  uint8_t GetNumChildren() const;

private:
  uint8_t m_hops;

  int m_rssiFeedback;

  uint8_t m_numChildren;
};
}

}
#endif
