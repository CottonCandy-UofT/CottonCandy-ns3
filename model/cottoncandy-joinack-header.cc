/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-joinack-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyJoinAckHeader");

CottoncandyJoinAckHeader::CottoncandyJoinAckHeader ()
{
}

CottoncandyJoinAckHeader::~CottoncandyJoinAckHeader ()
{
}

void CottoncandyJoinAckHeader::SetHops(uint8_t hops){
  NS_LOG_FUNCTION(this << hops);

  m_hops = hops;

}

uint8_t CottoncandyJoinAckHeader::GetHops() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_hops;
}

TypeId CottoncandyJoinAckHeader::GetTypeId(void){
  static TypeId tid = TypeId("CottoncandyJoinAckHeader").SetParent<Header>().AddConstructor<CottoncandyJoinAckHeader>();

  return tid;
}

TypeId CottoncandyJoinAckHeader::GetInstanceTypeId(void) const{
  return GetTypeId();
}

void CottoncandyJoinAckHeader::Serialize(Buffer::Iterator start) const{
  NS_LOG_FUNCTION_NOARGS();

  start.WriteU8(m_hops);
}


uint32_t CottoncandyJoinAckHeader::Deserialize(Buffer::Iterator start){
  NS_LOG_FUNCTION_NOARGS();

  m_hops = start.ReadU8();

  return 1;
}

void CottoncandyJoinAckHeader::Print(std::ostream &os) const{
    os << "Hops_To_Gateway=" << unsigned(m_hops) << std::endl;
}


uint32_t CottoncandyJoinAckHeader::GetSerializedSize(void) const{
  NS_LOG_FUNCTION_NOARGS();

  return 1;
}

}
}



