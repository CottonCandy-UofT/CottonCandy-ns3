/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-nodereply-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyNodeReplyHeader");

CottoncandyNodeReplyHeader::CottoncandyNodeReplyHeader ()
{
}

CottoncandyNodeReplyHeader::~CottoncandyNodeReplyHeader ()
{
}

TypeId CottoncandyNodeReplyHeader::GetTypeId(void){
  static TypeId tid = TypeId("CottoncandyNodeReplyHeader").SetParent<Header>().AddConstructor<CottoncandyNodeReplyHeader>();

  return tid;
}

TypeId CottoncandyNodeReplyHeader::GetInstanceTypeId(void) const{
  return GetTypeId();
}

void CottoncandyNodeReplyHeader::Serialize(Buffer::Iterator start) const{
  NS_LOG_FUNCTION_NOARGS();

  start.WriteU8(m_seqNum);

  uint8_t b = m_dataLen;

  if(m_aggregated){
      b |= 0x80; //set the highest bit to 1
  }else{
      b &= 0x7F; //set the highest bit to 0
  }

  start.WriteU8(b);
}


uint32_t CottoncandyNodeReplyHeader::Deserialize(Buffer::Iterator start){
  NS_LOG_FUNCTION_NOARGS();

  m_seqNum = start.ReadU8();
  uint8_t b = start.ReadU8();

  if (b & 0x80){
      m_aggregated = true;
  }else{
      m_aggregated = false;
  }

  m_dataLen = b & 0x7F;
  

  return 2;
}

void CottoncandyNodeReplyHeader::Print(std::ostream &os) const{
    os << "Sequence Number=" << unsigned(m_seqNum) << ", Data length" << unsigned(m_dataLen) << std::endl;
}


uint32_t CottoncandyNodeReplyHeader::GetSerializedSize() const{
    return 2;
}

void CottoncandyNodeReplyHeader::SetSeqNum(uint8_t seqNum){
    m_seqNum = seqNum;
}

void CottoncandyNodeReplyHeader::SetDataLen(uint8_t len){
    m_dataLen= len;
}

void CottoncandyNodeReplyHeader::SetAggregated(bool aggregated){
    m_aggregated = aggregated;
}

uint8_t CottoncandyNodeReplyHeader::GetSeqNum() const{
    return m_seqNum;
}

uint8_t CottoncandyNodeReplyHeader::GetDataLen() const{
    return m_dataLen;
}

bool CottoncandyNodeReplyHeader::GetAggregated() const{
    return m_aggregated;
}



}
}