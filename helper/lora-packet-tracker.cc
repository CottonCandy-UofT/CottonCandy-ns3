/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Padova
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

#include "lora-packet-tracker.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/cottoncandy-mac-header.h"
#include <iostream>
#include <fstream>

namespace ns3 {
namespace lorawan {
NS_LOG_COMPONENT_DEFINE ("LoraPacketTracker");

LoraPacketTracker::LoraPacketTracker ()
{
  NS_LOG_FUNCTION (this);
}

LoraPacketTracker::~LoraPacketTracker ()
{
  NS_LOG_FUNCTION (this);
}

/////////////////
// MAC metrics //
/////////////////

void LoraPacketTracker::CottoncandyConnectionCallback(uint16_t childAddr, uint16_t parentAddr, Vector childPosition, uint8_t txPwr){

  if(m_cottoncandyTopology.find(childAddr) != m_cottoncandyTopology.end()){
    //Self-healing event (i.e. node has joined the network before)
    m_cottoncandyTopology[childAddr].parentAddr = parentAddr;
    m_cottoncandyTopology[childAddr].txPwr = txPwr;
    m_cottoncandyTopology[childAddr].numSelfHealing ++;
  }else{
    CottoncandyStatus status;
    status.parentAddr = parentAddr;
    status.position = childPosition;
    status.numReplyDelivered = 0;
    status.numReqReceived = 0;

    status.txPwr = txPwr;

    //Fresh join
    status.numSelfHealing = 0;
    status.timeFirstJoin = Simulator::Now().GetSeconds();
    m_cottoncandyTopology.insert(std::pair<uint16_t, CottoncandyStatus> (childAddr,status));

    if(m_cottoncandyTopology.size() == m_numNodes){
      NS_LOG_DEBUG("All nodes (gateway) have joined the network");
      m_joinCompleteTime = status.timeFirstJoin;
    }
  }
  
  NS_LOG_DEBUG("Insert edge " << parentAddr << " to " << childAddr);
}

void LoraPacketTracker::CottoncandySetNumNodes(int numNodes){
  m_numNodes = numNodes;
}

double LoraPacketTracker::CottoncandyGetJoinCompletionTime(){
  return m_joinCompleteTime;
}

void LoraPacketTracker::CottoncandyReceiveReqCallback(uint16_t nodeAddr){
  auto it = m_cottoncandyTopology.find (nodeAddr);

  if(it != m_cottoncandyTopology.end()){
    it->second.numReqReceived ++;
  }
}

void LoraPacketTracker::CottoncandyReplyDeliveredCallback(uint16_t nodeAddr){
  if(m_cottoncandyTopology.size() < m_numNodes){
    //We do not count the reply deliver rate until all nodes have joined the network
    return;
  }

  auto it = m_cottoncandyTopology.find (nodeAddr);

  if(it != m_cottoncandyTopology.end()){
    it->second.numReplyDelivered ++;
  }
}

void
LoraPacketTracker::CottoncandyChannelSwitchCallback(uint16_t nodeAddr, uint8_t seqNum){
  auto it = m_cottoncandyChannelSwitch.find(nodeAddr);

  if(it == m_cottoncandyChannelSwitch.end()){
    //Create the history array
    std::vector<uint8_t> history;
    history.push_back(seqNum);

    m_cottoncandyChannelSwitch.insert(std::pair<uint16_t, std::vector<uint8_t>>(nodeAddr,history));
  }else{
    //The vector should be non-empty as it is initialized with a value (see above)
    if(!it->second.empty()){
      it->second.push_back(seqNum);
    }
  }
}

std::string LoraPacketTracker::PrintCottoncandyChannelStats(){
  std::stringstream ss;
  for (auto iter = m_cottoncandyChannelSwitch.begin(); iter != m_cottoncandyChannelSwitch.end(); iter++){
    ss << std::dec << iter->first;

    std::vector<uint8_t> seqHistory = iter->second;

    for (auto iter2 = seqHistory.begin(); iter2 != seqHistory.end(); iter2++){
      ss << " " << std::dec << (int)*iter2;
    }

    ss << "\n";
  }

  return ss.str();
}


std::string LoraPacketTracker::PrintCottoncandyEdges(){
  //NS_LOG_DEBUG(m_cottoncandyTopology.size());
  std::stringstream ss;
  for (auto iter = m_cottoncandyTopology.begin(); iter != m_cottoncandyTopology.end(); iter++){
    CottoncandyStatus status = iter->second;
    ss << std::hex << iter->first << " " << status.position.x << " " << status.position.y << " " 
       << std::hex << status.parentAddr << " " << std::dec << status.numReqReceived <<  " " 
       << status.numReplyDelivered << " " << status.numSelfHealing << " " << (int)status.txPwr << "\n";
  }

  return ss.str();
}

void
LoraPacketTracker::MacTransmissionCallback (Ptr<Packet const> packet)
{
  if (IsUplink (packet))
    {
      NS_LOG_INFO ("A new packet was sent by the MAC layer");

      MacPacketStatus status;
      status.packet = packet;
      status.sendTime = Simulator::Now ();
      status.senderId = Simulator::GetContext ();
      status.receivedTime = Time::Max ();

      m_macPacketTracker.insert (std::pair<Ptr<Packet const>, MacPacketStatus>
                                   (packet, status));
    }
}

void
LoraPacketTracker::RequiredTransmissionsCallback (uint8_t reqTx, bool success,
                                                  Time firstAttempt,
                                                  Ptr<Packet> packet)
{
  NS_LOG_INFO ("Finished retransmission attempts for a packet");
  NS_LOG_DEBUG ("Packet: " << packet << "ReqTx " << unsigned(reqTx) <<
                ", succ: " << success << ", firstAttempt: " <<
                firstAttempt.GetSeconds ());

  RetransmissionStatus entry;
  entry.firstAttempt = firstAttempt;
  entry.finishTime = Simulator::Now ();
  entry.reTxAttempts = reqTx;
  entry.successful = success;

  m_reTransmissionTracker.insert (std::pair<Ptr<Packet>, RetransmissionStatus>
                                    (packet, entry));
}

void
LoraPacketTracker::MacGwReceptionCallback (Ptr<Packet const> packet)
{
  if (IsUplink (packet))
    {
      NS_LOG_INFO ("A packet was successfully received" <<
                   " at the MAC layer of gateway " <<
                   Simulator::GetContext ());

      // Find the received packet in the m_macPacketTracker
      auto it = m_macPacketTracker.find (packet);
      if (it != m_macPacketTracker.end ())
        {
          (*it).second.receptionTimes.insert (std::pair<int, Time>
                                                (Simulator::GetContext (),
                                                Simulator::Now ()));
        }
      else
        {
          NS_ABORT_MSG ("Packet not found in tracker");
        }
    }
}

/////////////////
// PHY metrics //
/////////////////

/* The following callbacks are for cottoncandy so no need to check uplink or not*/
void
LoraPacketTracker::CottoncandyTransmissionCallback (Ptr<Packet const> packet, uint32_t edId)
{

  if(CottoncandyIsInterested(packet)){
    NS_LOG_INFO ("PHY packet " << packet
                                << " was transmitted by device "
                                << edId);
    // Create a packetStatus
    PacketStatus status;
    status.packet = packet;
    status.sendTime = Simulator::Now ();
    status.senderId = edId;

    m_packetTracker.insert (std::pair<Ptr<Packet const>, PacketStatus> (packet, status));
  }
}

/*
void
LoraPacketTracker::CottoncandyPacketReceptionCallback (Ptr<Packet const> packet, uint32_t gwId)
{

  if(CottoncandyIsInterested(packet)){
    // Remove the successfully received packet from the list of sent ones
    NS_LOG_INFO ("PHY packet " << packet
                                << " was successfully received at gateway "
                                << gwId);

    std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
    (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                          RECEIVED));
  }
}

void
LoraPacketTracker::CottoncandyInterferenceCallback (Ptr<Packet const> packet, uint32_t gwId)
{
    if(CottoncandyIsInterested(packet)){
      NS_LOG_INFO ("PHY packet " << packet
                                  << " was interfered at gateway "
                                  << gwId);

      std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
      (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                            INTERFERED));
    }
}

void
LoraPacketTracker::CottoncandyNoMoreReceiversCallback (Ptr<Packet const> packet, uint32_t gwId)
{
  
    NS_LOG_INFO ("PHY packet " << packet
                                << " was lost because no more receivers at gateway "
                                << gwId);
    std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
    (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                          NO_MORE_RECEIVERS));
  
}

void
LoraPacketTracker::CottoncandyUnderSensitivityCallback (Ptr<Packet const> packet, uint32_t gwId)
{
      NS_LOG_INFO ("PHY packet " << packet
                                 << " was lost because under sensitivity at gateway "
                                 << gwId);

      std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
      (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                           UNDER_SENSITIVITY));
    
}

void
LoraPacketTracker::CottoncandyLostBecauseTxCallback (Ptr<Packet const> packet, uint32_t gwId)
{

      NS_LOG_INFO ("PHY packet " << packet
                                 << " was lost because of GW transmission at gateway "
                                 << gwId);

      std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
      (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                           LOST_BECAUSE_TX));
    
}*/

void
LoraPacketTracker::CottoncandyLostBecauseTxCallback (uint16_t address)
{

  NS_LOG_DEBUG ("PHY packet " << " was lost because of transmission at node "
                              << address);

  m_cottoncandyPhyPerf.numReplyHalfDuplex++;

}

void 
LoraPacketTracker::CottoncandyLostBecauseCollisionCallback(uint8_t hops)
{
  m_cottoncandyCollisionLoc.push_back(hops);
}

bool LoraPacketTracker::CottoncandyIsInterested(Ptr<Packet const> packet){
  NS_LOG_FUNCTION (this);

  CottoncandyMacHeader mHdr;
  Ptr<Packet> copy = packet->Copy ();
  copy->RemoveHeader (mHdr);
  uint8_t type = mHdr.GetType();
  return (type == CottoncandyMacHeader::GATEWAY_REQ || type == CottoncandyMacHeader::NODE_REPLY);
}

/* The following callbacks are for lorawan*/

void
LoraPacketTracker::TransmissionCallback (Ptr<Packet const> packet, uint32_t edId)
{
  if (IsUplink (packet))
    {
      NS_LOG_INFO ("PHY packet " << packet
                                 << " was transmitted by device "
                                 << edId);
      // Create a packetStatus
      PacketStatus status;
      status.packet = packet;
      status.sendTime = Simulator::Now ();
      status.senderId = edId;

      m_packetTracker.insert (std::pair<Ptr<Packet const>, PacketStatus> (packet, status));
    }
}

void
LoraPacketTracker::PacketReceptionCallback (Ptr<Packet const> packet, uint32_t gwId)
{
  if (IsUplink (packet))
    {
      // Remove the successfully received packet from the list of sent ones
      NS_LOG_INFO ("PHY packet " << packet
                                 << " was successfully received at gateway "
                                 << gwId);

      std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
      (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                           RECEIVED));
    }
}

void
LoraPacketTracker::InterferenceCallback (Ptr<Packet const> packet, uint32_t gwId)
{
  if (IsUplink (packet))
    {
      NS_LOG_INFO ("PHY packet " << packet
                                 << " was interfered at gateway "
                                 << gwId);

      std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
      (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                           INTERFERED));
    }
}

void
LoraPacketTracker::NoMoreReceiversCallback (Ptr<Packet const> packet, uint32_t gwId)
{
  if (IsUplink (packet))
    {
      NS_LOG_INFO ("PHY packet " << packet
                                 << " was lost because no more receivers at gateway "
                                 << gwId);
      std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
      (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                           NO_MORE_RECEIVERS));
    }
}

void
LoraPacketTracker::UnderSensitivityCallback (Ptr<Packet const> packet, uint32_t gwId)
{
  if (IsUplink (packet))
    {
      NS_LOG_INFO ("PHY packet " << packet
                                 << " was lost because under sensitivity at gateway "
                                 << gwId);

      std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
      (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                           UNDER_SENSITIVITY));
    }
}

void
LoraPacketTracker::LostBecauseTxCallback (Ptr<Packet const> packet, uint32_t gwId)
{
  if (IsUplink (packet))
    {
      NS_LOG_INFO ("PHY packet " << packet
                                 << " was lost because of GW transmission at gateway "
                                 << gwId);

      std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
      (*it).second.outcomes.insert (std::pair<int, enum PhyPacketOutcome> (gwId,
                                                                           LOST_BECAUSE_TX));
    }
}

bool
LoraPacketTracker::IsUplink (Ptr<Packet const> packet)
{
  NS_LOG_FUNCTION (this);

  LorawanMacHeader mHdr;
  Ptr<Packet> copy = packet->Copy ();
  copy->RemoveHeader (mHdr);
  return mHdr.IsUplink ();
}

////////////////////////
// Counting Functions //
////////////////////////

std::vector<int>
LoraPacketTracker::CountPhyPacketsPerGw (Time startTime, Time stopTime,
                                         int gwId)
{
  // Vector packetCounts will contain - for the interval given in the input of
  // the function, the following fields: totPacketsSent receivedPackets
  // interferedPackets noMoreGwPackets underSensitivityPackets lostBecauseTxPackets

  std::vector<int> packetCounts (6, 0);

  for (auto itPhy = m_packetTracker.begin ();
       itPhy != m_packetTracker.end ();
       ++itPhy)
    {
      if ((*itPhy).second.sendTime >= startTime && (*itPhy).second.sendTime <= stopTime)
        {
          packetCounts.at (0)++;

          NS_LOG_DEBUG ("Dealing with packet " << (*itPhy).second.packet);
          NS_LOG_DEBUG ("This packet was received by " <<
                        (*itPhy).second.outcomes.size () << " gateways");

          if ((*itPhy).second.outcomes.count (gwId) > 0)
            {
              switch ((*itPhy).second.outcomes.at (gwId))
                {
                case RECEIVED:
                  {
                    packetCounts.at (1)++;
                    break;
                  }
                case INTERFERED:
                  {
                    packetCounts.at (2)++;
                    break;
                  }
                case NO_MORE_RECEIVERS:
                  {
                    packetCounts.at (3)++;
                    break;
                  }
                case UNDER_SENSITIVITY:
                  {
                    packetCounts.at (4)++;
                    break;
                  }
                case LOST_BECAUSE_TX:
                  {
                    packetCounts.at (5)++;
                    break;
                  }
                case UNSET:
                  {
                    break;
                  }
                }
            }
        }
    }

  return packetCounts;
}

std::string LoraPacketTracker::GetHalfDuplexPacketCount()
{
  std::stringstream ss;

  ss << "Replies lost due to half-duplex: " << m_cottoncandyPhyPerf.numReplyHalfDuplex << std::endl;

  return ss.str();
}

std::string LoraPacketTracker::GetCollisionStats()
{
  std::stringstream ss;

  int total = m_cottoncandyCollisionLoc.size();

  ss << "Total number of collisions: " << total << std::endl;

  uint8_t max = 0; 
  for(auto el : m_cottoncandyCollisionLoc){
    if (el > max){
      max = el;
    }
  }

  for(uint8_t i = 1; i <= max; i++){
    int count = 0;
    for(auto el : m_cottoncandyCollisionLoc){
      if(el == i){
        count ++;
      }
    }
    ss << "At " << (int)i << " hops left: " << count << std::endl;
  }

  return ss.str();
}


std::string
LoraPacketTracker::PrintPhyPacketsPerGw (Time startTime, Time stopTime,
                                         int gwId)
{
  // Vector packetCounts will contain - for the interval given in the input of
  // the function, the following fields: totPacketsSent receivedPackets
  // interferedPackets noMoreGwPackets underSensitivityPackets lostBecauseTxPackets

  std::vector<int> packetCounts (6, 0);

  for (auto itPhy = m_packetTracker.begin ();
       itPhy != m_packetTracker.end ();
       ++itPhy)
    {
      if ((*itPhy).second.sendTime >= startTime && (*itPhy).second.sendTime <= stopTime)
        {
          packetCounts.at (0)++;

          NS_LOG_DEBUG ("Dealing with packet " << (*itPhy).second.packet);
          NS_LOG_DEBUG ("This packet was received by " <<
                        (*itPhy).second.outcomes.size () << " gateways");

          if ((*itPhy).second.outcomes.count (gwId) > 0)
            {
              switch ((*itPhy).second.outcomes.at (gwId))
                {
                case RECEIVED:
                  {
                    packetCounts.at (1)++;
                    break;
                  }
                case INTERFERED:
                  {
                    packetCounts.at (2)++;
                    break;
                  }
                case NO_MORE_RECEIVERS:
                  {
                    packetCounts.at (3)++;
                    break;
                  }
                case UNDER_SENSITIVITY:
                  {
                    packetCounts.at (4)++;
                    break;
                  }
                case LOST_BECAUSE_TX:
                  {
                    packetCounts.at (5)++;
                    break;
                  }
                case UNSET:
                  {
                    break;
                  }
                }
            }
        }
    }

  std::string output ("");
  for (int i = 0; i < 6; ++i)
    {
      output += std::to_string (packetCounts.at (i)) + " ";
    }

  return output;
}

  std::string
  LoraPacketTracker::CountMacPacketsGlobally (Time startTime, Time stopTime)
  {
    NS_LOG_FUNCTION (this << startTime << stopTime);

    double sent = 0;
    double received = 0;
    for (auto it = m_macPacketTracker.begin ();
         it != m_macPacketTracker.end ();
         ++it)
      {
        if ((*it).second.sendTime >= startTime && (*it).second.sendTime <= stopTime)
          {
            sent++;
            if ((*it).second.receptionTimes.size ())
              {
                received++;
              }
          }
      }

    return std::to_string (sent) + " " +
      std::to_string (received);
  }

  std::string
  LoraPacketTracker::CountMacPacketsGloballyCpsr (Time startTime, Time stopTime)
  {
    NS_LOG_FUNCTION (this << startTime << stopTime);

    double sent = 0;
    double received = 0;
    for (auto it = m_reTransmissionTracker.begin ();
         it != m_reTransmissionTracker.end ();
         ++it)
      {
        if ((*it).second.firstAttempt >= startTime && (*it).second.firstAttempt <= stopTime)
          {
            sent++;
            NS_LOG_DEBUG ("Found a packet");
            NS_LOG_DEBUG ("Number of attempts: " << unsigned(it->second.reTxAttempts) <<
                          ", successful: " << it->second.successful);
            if (it->second.successful)
              {
                received++;
              }
          }
      }

    return std::to_string (sent) + " " +
      std::to_string (received);
  }

}
}
