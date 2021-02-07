/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-address.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyAddress");

// CottoncandyAddress
////////////////////

CottoncandyAddress::CottoncandyAddress ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

CottoncandyAddress::CottoncandyAddress (uint16_t address)
{
  NS_LOG_FUNCTION (this << address);

  Set (address);
}

void
CottoncandyAddress::Serialize (uint8_t buf[2]) const
{
  NS_LOG_FUNCTION (this << &buf);

  uint16_t address = Get ();

  buf[0] = (address >> 8) & 0xff;
  buf[1] = (address >> 0) & 0xff;
}

CottoncandyAddress
CottoncandyAddress::Deserialize (const uint8_t buf[2])
{
  NS_LOG_FUNCTION (&buf);

  // Craft the address from the buffer
  uint16_t address = 0;
  address |= buf[0];
  address <<= 8;
  address |= buf[1];

  return CottoncandyAddress (address);
}

Address
CottoncandyAddress::ConvertTo (void) const
{
  NS_LOG_FUNCTION (this);

  uint8_t addressBuffer[2];
  Serialize (addressBuffer);
  return Address (GetType (), addressBuffer, 2);
}

CottoncandyAddress
CottoncandyAddress::ConvertFrom (const Address &address)
{
  // Create the new, empty address
  CottoncandyAddress ad;
  uint8_t addressBuffer[2];

  // Check that the address we want to convert is compatible with a
  // CottoncandyAddress
  NS_ASSERT (address.CheckCompatible (GetType (), 2));
  address.CopyTo (addressBuffer);
  ad = Deserialize (addressBuffer);
  return ad;
}

uint8_t
CottoncandyAddress::GetType (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  static uint8_t type = Address::Register ();
  return type;
}

uint16_t
CottoncandyAddress::Get (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_DEBUG ("Address = " << std::bitset<16> (m_address));

  return m_address;
}

void
CottoncandyAddress::Set (uint16_t address)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_address = address;
}

bool CottoncandyAddress::isGateway(void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return ((GATEWAY_MASK & m_address) == GATEWAY_MASK);
}

std::string
CottoncandyAddress::Print (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  std::string result;
  result += std::bitset<16> (m_address).to_string ();
  return result;
}

bool CottoncandyAddress::operator==
  (const CottoncandyAddress &other) const
{
  return this->Get () == other.Get ();
}

bool CottoncandyAddress::operator!=
  (const CottoncandyAddress &other) const
{
  return this->Get () != other.Get ();
}

bool CottoncandyAddress::operator<
  (const CottoncandyAddress &other) const
{
  return this->Get () < other.Get ();
}

bool CottoncandyAddress::operator>
  (const CottoncandyAddress &other) const
{
  return !(this->Get () < other.Get ());
}

std::ostream& operator<< (std::ostream& os, const CottoncandyAddress &address)
{
  os << address.Print ();
  return os;
}
}
}
