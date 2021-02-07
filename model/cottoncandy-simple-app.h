/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COTTONCANDY_SIMPLE_APP_H
#define COTTONCANDY_SIMPLE_APP_H

#include "ns3/application.h"
#include "ns3/nstime.h"
#include "ns3/cottoncandy-mac.h"
#include "ns3/attribute.h"

namespace ns3 {
namespace lorawan {

class CottoncandySimpleApp : public Application
{
public:
  CottoncandySimpleApp ();
  CottoncandySimpleApp (Time startTime);
  ~CottoncandySimpleApp ();

  static TypeId GetTypeId (void);

  void Start();

  void StartApplication (void);

  void SetStartTime(Time startTime);

  /**
   * Stop the application.
   */
  void StopApplication (void);

private:

  /**
   * The sending event.
   */
  EventId m_sendEvent;

  /**
   * The MAC layer of this node.
   */
  Ptr<CottoncandyMac> m_mac;

  Time m_startTime;
};

} //namespace ns3

}
#endif /* COTTONCANDY SIMPLE APP */
