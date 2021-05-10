/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef COTTONCANDY_SIMPLE_APP_HELPER_H
#define COTTONCANDY_SIMPLE_APP_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/cottoncandy-simple-app.h"
#include <stdint.h>
#include <string>

namespace ns3 {
namespace lorawan {

/**
 * This class can be used to install CottoncandySimpleApp applications on multiple
 * nodes at once.
 */
class CottoncandySimpleAppHelper
{
public:
  CottoncandySimpleAppHelper ();

  ~CottoncandySimpleAppHelper ();

  void SetAttribute (std::string name, const AttributeValue &value);

  ApplicationContainer Install (NodeContainer c) const;

  ApplicationContainer Install (Ptr<Node> node) const;

  void SetStartTime (double startTime);

  void SetReplyLen (uint8_t len);

private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;

  double m_startTime; //!< Time at which the simple app will be configured to
                   //start the packet

  uint8_t m_replyLen;
  Ptr<UniformRandomVariable> m_uniformRv;
};

} // namespace ns3

}
#endif /* COTTONCANDY_SIMPLE_APP_HELPER_H */
