/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "ns3/cottoncandy-simple-app.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/lora-net-device.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandySimpleApp");

NS_OBJECT_ENSURE_REGISTERED (CottoncandySimpleApp);

TypeId
CottoncandySimpleApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CottoncandySimpleApp")
    .SetParent<Application> ()
    .AddConstructor<CottoncandySimpleApp> ()
    .SetGroupName ("lorawan");
  return tid;
}

CottoncandySimpleApp::CottoncandySimpleApp ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

CottoncandySimpleApp::CottoncandySimpleApp (Time startTime)
  : m_startTime (startTime)
{
  NS_LOG_FUNCTION_NOARGS ();
}

CottoncandySimpleApp::~CottoncandySimpleApp ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void CottoncandySimpleApp::SetStartTime(Time startTime){
  m_startTime = startTime;
}

void CottoncandySimpleApp::Start(){
  m_mac->Run();
}

void
CottoncandySimpleApp::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // Make sure we have a MAC layer
  if (m_mac == 0)
    {
      // Assumes there's only one device
      Ptr<LoraNetDevice> loraNetDevice = m_node->GetDevice (0)->GetObject<LoraNetDevice> ();

      m_mac = loraNetDevice->GetCMac ();
      NS_ASSERT (m_mac != 0);
    }

  Simulator::Schedule (m_startTime, &CottoncandySimpleApp::Start, this);
}

void
CottoncandySimpleApp::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  //Simulator::Cancel (m_sendEvent);
}
}
}
