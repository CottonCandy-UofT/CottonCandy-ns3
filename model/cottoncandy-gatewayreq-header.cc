/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-gatewayreq-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyGatewayReqHeader");

CottoncandyGatewayReqHeader::CottoncandyGatewayReqHeader ()
{
}

CottoncandyGatewayReqHeader::~CottoncandyGatewayReqHeader ()
{
}

void CottoncandyGatewayReqHeader::SetNextReqTime(uint32_t nextReqTime){
  NS_LOG_FUNCTION(this << nextReqTime);

  m_nextReqTime = nextReqTime;
}

uint32_t CottoncandyGatewayReqHeader::GetNextReqTime() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_nextReqTime;
}
    
void CottoncandyGatewayReqHeader::SetMaxBackoff(uint8_t maxBackoff){
  NS_LOG_FUNCTION(this << maxBackoff);

  m_maxBackoff = maxBackoff;
}

uint8_t CottoncandyGatewayReqHeader::GetMaxBackoff() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_maxBackoff;
}

void CottoncandyGatewayReqHeader::SetOption(uint8_t option){
  NS_LOG_FUNCTION(this << option);

  m_option = option;
}

uint8_t CottoncandyGatewayReqHeader::GetOption() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_option;
}

void CottoncandyGatewayReqHeader::SetChannel(uint8_t channel){
  NS_LOG_FUNCTION(this << channel);

  m_channel = channel;
}

uint8_t CottoncandyGatewayReqHeader::GetChannel() const{
  NS_LOG_FUNCTION_NOARGS();

  return m_channel;
}

TypeId CottoncandyGatewayReqHeader::GetTypeId(void){
  static TypeId tid = TypeId("CottoncandyGatewayReqHeader").SetParent<Header>().AddConstructor<CottoncandyGatewayReqHeader>();

  return tid;
}

TypeId CottoncandyGatewayReqHeader::GetInstanceTypeId(void) const{
  return GetTypeId();
}

void CottoncandyGatewayReqHeader::Serialize(Buffer::Iterator start) const{
  NS_LOG_FUNCTION_NOARGS();

  start.WriteU8(m_option);
  start.WriteU8(m_channel);


  start.WriteU32(m_nextReqTime);

  start.WriteU8(m_maxBackoff);
  
}


uint32_t CottoncandyGatewayReqHeader::Deserialize(Buffer::Iterator start){
  NS_LOG_FUNCTION_NOARGS();

  m_option = start.ReadU8();
  m_channel = start.ReadU8();

  m_nextReqTime = start.ReadU32();

  m_maxBackoff = start.ReadU8();


  return 7;
}

void CottoncandyGatewayReqHeader::Print(std::ostream &os) const{
    os << "option =" << unsigned(m_option) << ", channel =" << unsigned(m_channel) <<
    ", nextReqTime = " << m_nextReqTime << ", max backoff time =" << m_maxBackoff <<std::endl;
}


uint32_t CottoncandyGatewayReqHeader::GetSerializedSize(void) const{
  NS_LOG_FUNCTION_NOARGS();

  return 7;
}

}
}



