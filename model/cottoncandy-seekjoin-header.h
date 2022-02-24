/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COTTONCANDY_SEEKJOIN_HEADER_H
#define COTTONCANDY_SEEKJOIN_HEADER_H

#include "ns3/header.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the extra header of a SEEKJOIN packet.
 */
class CottoncandySeekJoinHeader : public Header
{
public:

  static TypeId GetTypeId (void);
  CottoncandySeekJoinHeader();
  ~CottoncandySeekJoinHeader();

  virtual TypeId GetInstanceTypeId(void) const;

  virtual uint32_t GetSerializedSize(void) const;

  virtual void Serialize(Buffer::Iterator start) const;

  virtual uint32_t Deserialize(Buffer::Iterator start);

  virtual void Print (std::ostream &os) const;

  void SetNextAcceptJoinTime(uint32_t nextAcceptJoinTime);

  uint32_t GetNextAcceptJoinTime() const;
    
  void SetMaxBackoff(uint8_t maxBackoff);

  uint8_t GetMaxBackoff() const;

  void SetPrivateChannel(uint8_t privateChannel);

  uint8_t GetPrivateChannel() const;

  void SetParentChannel(uint8_t parenthannel);

  uint8_t GetParentChannel() const;

  void SetNumChildren(uint8_t numChildren);

  uint8_t GetNumChildren() const;

private:
  uint8_t m_privateChannel;
  uint8_t m_parentChannel;
  uint8_t m_maxBackoff;
  uint8_t m_numChildren;
  uint32_t m_nextAcceptJoinTime;
  

};
}

}
#endif
