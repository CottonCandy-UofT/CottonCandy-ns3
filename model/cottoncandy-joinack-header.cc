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

void
CottoncandyJoinAckHeader::SetHops (uint8_t hops)
{
  NS_LOG_FUNCTION (this << hops);

  m_hops = hops;
}

void
CottoncandyJoinAckHeader::SetNumChildren (uint8_t numChildren)
{
  NS_LOG_FUNCTION (this << numChildren);

  m_numChildren = numChildren;
}

void
CottoncandyJoinAckHeader::SetRssi (int rssi)
{

  NS_LOG_FUNCTION (this << rssi);

  m_rssiFeedback = rssi;
}
/*-------------------------------------------------------------*/
uint8_t
CottoncandyJoinAckHeader::GetHops () const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_hops;
}

uint8_t
CottoncandyJoinAckHeader::GetNumChildren () const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_numChildren;
}

int
CottoncandyJoinAckHeader::GetRssi () const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_rssiFeedback;
}

TypeId
CottoncandyJoinAckHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("CottoncandyJoinAckHeader")
                          .SetParent<Header> ()
                          .AddConstructor<CottoncandyJoinAckHeader> ();

  return tid;
}

TypeId
CottoncandyJoinAckHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
CottoncandyJoinAckHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  start.WriteU8 (m_hops);
  start.WriteU8 (m_numChildren);

  uint8_t positiveRssi = (uint8_t)std::abs(m_rssiFeedback);
  start.WriteU8 (positiveRssi);
}

uint32_t
CottoncandyJoinAckHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_hops = start.ReadU8 ();
  m_numChildren = start.ReadU8 ();
  m_rssiFeedback = -(int)start.ReadU8();

  return 3;
}

void
CottoncandyJoinAckHeader::Print (std::ostream &os) const
{
  os << "Hops_To_Gateway=" << unsigned (m_hops) << std::endl;
}

uint32_t
CottoncandyJoinAckHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return 3;
}

} // namespace lorawan
} // namespace ns3
