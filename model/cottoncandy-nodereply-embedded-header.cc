/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-nodereply-embedded-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyNodeReplyEmbeddedHeader");

CottoncandyNodeReplyEmbeddedHeader::CottoncandyNodeReplyEmbeddedHeader ()
{
}

CottoncandyNodeReplyEmbeddedHeader::~CottoncandyNodeReplyEmbeddedHeader ()
{
}

TypeId CottoncandyNodeReplyEmbeddedHeader::GetTypeId(void){
  static TypeId tid = TypeId("CottoncandyNodeReplyEmbeddedHeader").SetParent<Header>().AddConstructor<CottoncandyNodeReplyEmbeddedHeader>();

  return tid;
}

TypeId CottoncandyNodeReplyEmbeddedHeader::GetInstanceTypeId(void) const{
  return GetTypeId();
}

void CottoncandyNodeReplyEmbeddedHeader::Serialize(Buffer::Iterator start) const{
  NS_LOG_FUNCTION_NOARGS();

  start.WriteU16(m_srcAddr);

  start.WriteU8(m_dataLen);
}


uint32_t CottoncandyNodeReplyEmbeddedHeader::Deserialize(Buffer::Iterator start){
  NS_LOG_FUNCTION_NOARGS();

  m_srcAddr = start.ReadU16();
  m_dataLen = start.ReadU8();

  return 3;
}

void CottoncandyNodeReplyEmbeddedHeader::Print(std::ostream &os) const{
    os << "Source Address=" << unsigned(m_srcAddr) << ", Data length" << unsigned(m_dataLen) << std::endl;
}


uint32_t CottoncandyNodeReplyEmbeddedHeader::GetSerializedSize() const{
    return 3;
}

void CottoncandyNodeReplyEmbeddedHeader::SetSrc(uint16_t srcAddr){
    m_srcAddr = srcAddr;
}

void CottoncandyNodeReplyEmbeddedHeader::SetDataLen(uint8_t len){
    m_dataLen= len;
}

uint16_t CottoncandyNodeReplyEmbeddedHeader::GetSrc() const{
    return m_srcAddr;
}

uint8_t CottoncandyNodeReplyEmbeddedHeader::GetDataLen() const{
    return m_dataLen;
}
}
}