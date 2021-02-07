/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */


#ifndef COTTONCANDY_ADDRESS_H
#define COTTONCANDY_ADDRESS_H

#include "ns3/address.h"
#include <string>

namespace ns3 {
namespace lorawan {



/**
 * This class represents the device address of a LoraWAN End Device.
 */
class CottoncandyAddress
{
public:

  static const uint16_t GATEWAY_MASK = 0x8000;
  

  CottoncandyAddress ();

  /**
   * Build a new address from a 16-bit integer.
   */
  CottoncandyAddress (uint16_t address);


  /**
   * Convert this address to a buffer.
   */
  void Serialize (uint8_t buf[2]) const;

  /**
   * Convert the input buffer into a new address.
   */
  static CottoncandyAddress Deserialize (const uint8_t buf[2]);

  /**
   * Convert from an ordinary address to a CottoncandyAddress instance.
   */
  static CottoncandyAddress ConvertFrom (const Address &address);

  /**
   * Set the address as a 16 bit integer.
   */
  void Set (uint16_t address);

  /**
   * Get the address in 32-bit integer form.
   */
  uint16_t Get (void) const;

  bool isGateway(void) const;

  /**
   * Print the address bit-by-bit to a human-readable string.
   *
   * \return The string containing the network address.
   */
  std::string Print (void) const;

  bool operator== (const CottoncandyAddress &other) const;
  bool operator!= (const CottoncandyAddress &other) const;
  bool operator< (const CottoncandyAddress &other) const;
  bool operator> (const CottoncandyAddress &other) const;

private:
  /**
   * Convert this instance of CottoncandyAddress to an Address
   */
  Address ConvertTo (void) const;
  static uint8_t GetType (void);
  
  uint16_t m_address;
};

/**
 * Operator overload to correctly handle logging when an address is passed as
 * an argument.
 */
std::ostream& operator<< (std::ostream& os, const CottoncandyAddress &address);

}
}
#endif
