/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-seekjoin-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandySeekJoinHeader");

CottoncandySeekJoinHeader::CottoncandySeekJoinHeader ()
{
}

CottoncandySeekJoinHeader::~CottoncandySeekJoinHeader ()
{
}

void CottoncandySeekJoinHeader::SetNextAcceptJoinTime(uint32_t nextAcceptJoinTime){
  NS_LOG_FUNCTION(this << nextAcceptJoinTime);

  m_nextAcceptJoinTime = nextAcceptJoinTime;
}

uint32_t CottoncandySeekJoinHeader::GetNextAcceptJoinTime() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_nextAcceptJoinTime;
}
    
void CottoncandySeekJoinHeader::SetMaxBackoff(uint8_t maxBackoff){
  NS_LOG_FUNCTION(this << maxBackoff);

  m_maxBackoff = maxBackoff;
}

uint8_t CottoncandySeekJoinHeader::GetMaxBackoff() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_maxBackoff;
}
void CottoncandySeekJoinHeader::SetParentChannel(uint8_t parentChannel){
  NS_LOG_FUNCTION(this << parentChannel);

  m_parentChannel = parentChannel;
}

uint8_t CottoncandySeekJoinHeader::GetParentChannel() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_parentChannel;
}

void CottoncandySeekJoinHeader::SetPrivateChannel(uint8_t privateChannel){
  NS_LOG_FUNCTION(this << privateChannel);

  m_privateChannel = privateChannel;
}

uint8_t CottoncandySeekJoinHeader::GetPrivateChannel() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_privateChannel;
}

void CottoncandySeekJoinHeader::SetNumChildren(uint8_t numChildren){
  NS_LOG_FUNCTION(this << numChildren);

  m_numChildren = numChildren;
}

uint8_t CottoncandySeekJoinHeader::GetNumChildren() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_numChildren;
}

TypeId CottoncandySeekJoinHeader::GetTypeId(void){
  static TypeId tid = TypeId("CottoncandySeekJoinHeader").SetParent<Header>().AddConstructor<CottoncandySeekJoinHeader>();

  return tid;
}

TypeId CottoncandySeekJoinHeader::GetInstanceTypeId(void) const{
  return GetTypeId();
}

void CottoncandySeekJoinHeader::Serialize(Buffer::Iterator start) const{
  NS_LOG_FUNCTION_NOARGS();

  start.WriteU8(m_privateChannel);
  start.WriteU8(m_parentChannel);
  start.WriteU8(m_numChildren);
  start.WriteU8(m_maxBackoff);
  start.WriteU32(m_nextAcceptJoinTime);
  
}


uint32_t CottoncandySeekJoinHeader::Deserialize(Buffer::Iterator start){
  NS_LOG_FUNCTION_NOARGS();

  m_privateChannel = start.ReadU8();
  m_parentChannel = start.ReadU8();
  m_numChildren = start.ReadU8();
  m_maxBackoff = start.ReadU8();
  m_nextAcceptJoinTime = start.ReadU32();

  return 8;
}

void CottoncandySeekJoinHeader::Print(std::ostream &os) const{
    os << "private channel =" << unsigned(m_privateChannel) << ", parent channel =" << unsigned(m_parentChannel)
     << ", numChildren =" << m_numChildren << ", max backoff time =" << m_maxBackoff 
     << ", nextReqTime = " << m_nextAcceptJoinTime <<std::endl;
}


uint32_t CottoncandySeekJoinHeader::GetSerializedSize(void) const{
  NS_LOG_FUNCTION_NOARGS();

  return 8;
}

}
}



