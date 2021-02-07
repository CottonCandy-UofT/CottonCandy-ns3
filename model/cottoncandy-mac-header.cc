/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-mac-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyMacHeader");

CottoncandyMacHeader::CottoncandyMacHeader ()
{
}

CottoncandyMacHeader::~CottoncandyMacHeader ()
{
}

void CottoncandyMacHeader::SetType(enum MsgType type){
  NS_LOG_FUNCTION(this << type);

  m_mtype = type;
}

void CottoncandyMacHeader::SetSrc(uint16_t src){
  NS_LOG_FUNCTION(this << src);

  m_srcAddr = src;
}

void CottoncandyMacHeader::SetDest(uint16_t dest){
  NS_LOG_FUNCTION(this << dest);

  m_destAddr = dest;
}

uint8_t CottoncandyMacHeader::GetType() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_mtype;
}

uint16_t CottoncandyMacHeader::GetSrc() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_srcAddr;
}

uint16_t CottoncandyMacHeader::GetDest() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_destAddr;
}

TypeId CottoncandyMacHeader::GetTypeId(void){
  static TypeId tid = TypeId("CottoncandyMacHeader").SetParent<Header>().AddConstructor<CottoncandyMacHeader>();

  return tid;
}

TypeId CottoncandyMacHeader::GetInstanceTypeId(void) const{
  return GetTypeId();
}

void CottoncandyMacHeader::Serialize(Buffer::Iterator start) const{
  NS_LOG_FUNCTION_NOARGS();

  start.WriteU8(m_mtype);
  start.WriteU16(m_srcAddr);
  start.WriteU16(m_destAddr);
}


uint32_t CottoncandyMacHeader::Deserialize(Buffer::Iterator start){
  NS_LOG_FUNCTION_NOARGS();

  m_mtype = start.ReadU8();
  m_srcAddr = start.ReadU16();
  m_destAddr = start.ReadU16();

  return 5;
}

void CottoncandyMacHeader::Print(std::ostream &os) const{
    os << "MessageType=" << unsigned(m_mtype) << std::endl;
    os << "Src=" << unsigned(m_srcAddr) << std::endl;
    os << "Dest=" << unsigned(m_destAddr) << std::endl;
}


uint32_t CottoncandyMacHeader::GetSerializedSize(void) const{
  NS_LOG_FUNCTION_NOARGS();

  return 5;
}

}
}



