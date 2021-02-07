/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-simple-app-helper.h"
#include "ns3/cottoncandy-simple-app.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandySimpleAppHelper");

CottoncandySimpleAppHelper::CottoncandySimpleAppHelper ()
{
  m_factory.SetTypeId ("ns3::CottoncandySimpleApp");
  m_uniformRv = CreateObject<UniformRandomVariable>();
}

CottoncandySimpleAppHelper::~CottoncandySimpleAppHelper ()
{
}

void
CottoncandySimpleAppHelper::SetStartTime (double startTime)
{
  m_startTime = startTime;
}

void
CottoncandySimpleAppHelper::SetAttribute (std::string name,
                                   const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
CottoncandySimpleAppHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
CottoncandySimpleAppHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
CottoncandySimpleAppHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node->GetId ());

  Ptr<CottoncandySimpleApp> app = m_factory.Create<CottoncandySimpleApp> ();

  double randomDelay = m_uniformRv->GetValue(0, m_startTime);

  NS_LOG_DEBUG("Random start delay = " << randomDelay);

  Time startDelay = Seconds(randomDelay);

  app->SetStartTime (startDelay);

  app->SetNode (node);
  node->AddApplication (app);

  return app;
}
}
} // namespace ns3
