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

  start.WriteU8(m_option);
  start.WriteU8(m_dataLen);
}


uint32_t CottoncandyNodeReplyHeader::Deserialize(Buffer::Iterator start){
  NS_LOG_FUNCTION_NOARGS();

  
  m_option = start.ReadU8();

  m_dataLen = start.ReadU8();
  
  return 2;
}

void CottoncandyNodeReplyHeader::Print(std::ostream &os) const{
    os << "Option=" << unsigned(m_option) << ", Data length" << unsigned(m_dataLen) << std::endl;
}


uint32_t CottoncandyNodeReplyHeader::GetSerializedSize() const{
    return 2;
}

void CottoncandyNodeReplyHeader::SetOption(uint8_t option){
    m_option = option;
}

void CottoncandyNodeReplyHeader::SetDataLen(uint8_t len){
    m_dataLen= len;
}

uint8_t CottoncandyNodeReplyHeader::GetOption() const{
    return m_option;
}

uint8_t CottoncandyNodeReplyHeader::GetDataLen() const{
    return m_dataLen;
}



}
}