/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/cottoncandy-mac.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CottoncandyMac");

NS_OBJECT_ENSURE_REGISTERED (CottoncandyMac);

TypeId
CottoncandyMac::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::CottoncandyMac")
    .SetParent<Object> ()
    .SetGroupName ("lorawan")
    .AddTraceSource ("SentNewPacket",
                     "Trace source indicating a new packet "
                     "arrived at the MAC layer",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_sentNewPacket),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("ReceivedPacket",
                     "Trace source indicating a packet "
                     "was correctly received at the MAC layer",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_receivedPacket),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("ConnectionEstablished",
                     "Trace source indicating a parent "
                     "has been found and the connection has established",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_connectionEstablished),
                     "ns3::TracedValueCallback::uint16_t")
    .AddTraceSource ("GatewayReqReceived",
                     "Trace source indicating a node has received a"
                     "request from the gateway",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_gatewayReqReceived),
                     "ns3::TracedValueCallback::uint16_t")
    .AddTraceSource ("ReplyDelivered",
                     "Trace source indicating the gateway has received a"
                     "reply from a node",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_replyDelivered),
                     "ns3::TracedValueCallback::uint16_t")
    .AddTraceSource ("HalfDuplexDetected", "Trace source indicating a half-duplex event",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_halfDuplexDetected),
                     "ns3::TracedValueCallback::uint16_t")
    .AddTraceSource ("CollisionDetected", "Trace source indicating a failed RX event",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_collisionDetected),
                     "ns3::TracedValueCallback::uint8_t")
    .AddTraceSource ("ChannelSwitched", "Trace source indicating a channel switch event",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_channelSwitched),
                     "ns3::TracedValueCallback::uint16_t")
    .AddConstructor<CottoncandyMac> ();
  return tid;
}

double
CottoncandyMac::GetChannelFreq (uint8_t channel)
{
  return (channel * CHANNEL_SIZE + CHANNEL_START_FREQ);
}

CottoncandyMac::CottoncandyMac ()
{
  NS_LOG_FUNCTION (this);

  // Initialize the random variable we'll use to decide which channel to
  // transmit on.
  m_uniformRV = CreateObject<UniformRandomVariable> ();

  // Craft LoraTxParameters object
  m_params.sf = 7;
  m_params.headerDisabled = false;
  m_params.codingRate = 1;
  m_params.bandwidthHz = 125e3;
  m_params.nPreamble = 8;
  m_params.crcEnabled = 1;
  m_params.lowDataRateOptimizationEnabled = 0;

  m_numChildren = 0;
  m_channelSelector = CreateObject<CottoncandyChannelSelector> ();
}

CottoncandyMac::~CottoncandyMac ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<LoraPhy>
CottoncandyMac::GetPhy (void)
{
  return m_phy;
}

void
CottoncandyMac::SetPhy (Ptr<LoraPhy> phy)
{
  // Set the phy
  m_phy = phy;

  // Connect the receive callbacks
  m_phy->SetReceiveOkCallback (MakeCallback (&CottoncandyMac::Receive, this));
  m_phy->SetReceiveFailedCallback (MakeCallback (&CottoncandyMac::FailedReception, this));
  //m_phy->SetTxFinishedCallback (MakeCallback (&CottoncandyMac::TxFinished, this));
  m_phy->SetHalfDuplexCallback (MakeCallback (&CottoncandyMac::ReportHalfDuplex, this));
}

void
CottoncandyMac::FailedReception (Ptr<Packet const> packet)
{
  Ptr<Packet> packetCopy = packet->Copy ();

  CottoncandyMacHeader mHdr;

  packetCopy->RemoveHeader (mHdr);

  CottoncandyAddress dest = CottoncandyAddress (mHdr.GetDest ());

  uint8_t remainingHops = m_parent.hops + 1;

  if (dest == m_address && mHdr.GetType () == CottoncandyMacHeader::NODE_REPLY)
    {
      NS_LOG_DEBUG ("A packet RX failed due to collisions");
      m_collisionDetected (remainingHops);
    }
}

void
CottoncandyMac::ReportHalfDuplex (Ptr<Packet const> packet)
{
  Ptr<Packet> packetCopy = packet->Copy ();

  CottoncandyMacHeader mHdr;

  packetCopy->RemoveHeader (mHdr);

  CottoncandyAddress dest = CottoncandyAddress (mHdr.GetDest ());
  //CottoncandyAddress src = CottoncandyAddress(mHdr.GetSrc());

  // Report the half-duplex problem when the packet is intended for this node and
  // is a reply message (we are not interested in other messages)
  if (mHdr.GetType () == CottoncandyMacHeader::NODE_REPLY)
    {
      if (dest == m_address)
        {
          NS_LOG_DEBUG ("A packet RX failed due to half duplex");
          m_halfDuplexDetected (m_address.Get ());
        }
    }
}

LogicalLoraChannelHelper
CottoncandyMac::GetLogicalLoraChannelHelper (void)
{
  return m_channelHelper;
}

void
CottoncandyMac::SetLogicalLoraChannelHelper (LogicalLoraChannelHelper helper)
{
  m_channelHelper = helper;
}

uint8_t
CottoncandyMac::GetSfFromDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned (dataRate));

  // Check we are in range
  if (dataRate >= m_sfForDataRate.size ())
    {
      return 0;
    }

  return m_sfForDataRate.at (dataRate);
}

double
CottoncandyMac::GetBandwidthFromDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned (dataRate));

  // Check we are in range
  if (dataRate > m_bandwidthForDataRate.size ())
    {
      return 0;
    }

  return m_bandwidthForDataRate.at (dataRate);
}

double
CottoncandyMac::GetDbmForTxPower (uint8_t txPower)
{
  NS_LOG_FUNCTION (this << unsigned (txPower));

  if (txPower > m_txDbmForTxPower.size ())
    {
      return 0;
    }

  return m_txDbmForTxPower.at (txPower);
}

void
CottoncandyMac::SetSfForDataRate (std::vector<uint8_t> sfForDataRate)
{
  m_sfForDataRate = sfForDataRate;
}

void
CottoncandyMac::SetBandwidthForDataRate (std::vector<double> bandwidthForDataRate)
{
  m_bandwidthForDataRate = bandwidthForDataRate;
}

void
CottoncandyMac::SetMaxAppPayloadForDataRate (std::vector<uint32_t> maxAppPayloadForDataRate)
{
  m_maxAppPayloadForDataRate = maxAppPayloadForDataRate;
}

void
CottoncandyMac::SetTxDbmForTxPower (std::vector<double> txDbmForTxPower)
{
  m_txDbmForTxPower = txDbmForTxPower;
}

void
CottoncandyMac::SetNPreambleSymbols (int nPreambleSymbols)
{
  m_nPreambleSymbols = nPreambleSymbols;
}

int
CottoncandyMac::GetNPreambleSymbols (void)
{
  return m_nPreambleSymbols;
}

void
CottoncandyMac::SetReplyDataRateMatrix (ReplyDataRateMatrix replyDataRateMatrix)
{
  m_replyDataRateMatrix = replyDataRateMatrix;
}

// This is where we mimic the behavior of our protocol
void
CottoncandyMac::Receive (Ptr<Packet const> packet)
{
  if (m_state == HIBERNATE)
    {
      return;
    }

  Ptr<Packet> packetCopy = packet->Copy ();

  CottoncandyMacHeader mHdr;
  packetCopy->RemoveHeader (mHdr);

  CottoncandyAddress src = CottoncandyAddress (mHdr.GetSrc ());
  CottoncandyAddress dest = CottoncandyAddress (mHdr.GetDest ());
  NS_LOG_DEBUG ("Packet arrived at CottoncandyMac from 0x" << std::hex << src.Get ());

  // Use the tag to know the RSSI strength
  LoraTag tag;
  packetCopy->RemovePacketTag (tag);
  double rxPower = tag.GetReceivePower ();

  uint8_t type = mHdr.GetType ();

  // Check if the message is intended for us
  if (dest != m_address && dest != BROADCAST_ADDR)
    {
      //NS_LOG_DEBUG ("Packet dropped. The packet is for node " << dest.Print ());
      return;
    }

  switch (type)
    {
      case CottoncandyMacHeader::MsgType::SEEK_JOIN:
        {
          NS_LOG_DEBUG ("SEEK_JOIN message received");

          CottoncandySeekJoinHeader seekHdr;
          packetCopy->RemoveHeader (seekHdr);

          uint32_t timeTillNextAcceptJoin = seekHdr.GetNextAcceptJoinTime ();

          uint8_t numChildrenOfSender = seekHdr.GetNumChildren ();
          uint8_t privateChannel = seekHdr.GetPrivateChannel ();
          uint8_t parentChannel = seekHdr.GetParentChannel ();

          if (m_state == OBSERVE)
            {

              if (numChildrenOfSender < MAX_NUM_CHILDREN
                  && m_candidateParents.find (src) == m_candidateParents.end ())
                {
                  //Another distinct parent candidate has capacity for at least one more child, add it to a list
                  m_candidateParents.insert (
                    std::pair<CottoncandyAddress, uint8_t> (src, privateChannel));

                  NS_LOG_DEBUG ("A parent candidate 0x" << std::hex << src.Get () << std::dec << ": join in " << timeTillNextAcceptJoin << ", numChidlren:"
                                                        << (unsigned int)numChildrenOfSender << ", private/parent channel:" << (unsigned int)privateChannel << "/" << (unsigned int)parentChannel);

                  if (m_candidateParents.size () == 1)
                    {
                      //If this is the first candidate, we set the time of the next accept join phase

                      //TODO: Now just use a constant 10 seconds (i.e. After a node receives a SEEK_JOIN, it receives for another
                      //10 seconds to try to get another candidate)
                      m_endObserveId =
                        Simulator::Schedule (Seconds (10), &CottoncandyMac::EndObservePhase, this);

                      //Try to join the network when the next join phase starts
                      Simulator::Schedule (Seconds (timeTillNextAcceptJoin),
                                           &CottoncandyMac::EnterJoinPhase, this);
                    }
                  else if (m_candidateParents.size () >= MAX_NUM_CANDIDATE_PARENT)
                    {

                      //If we have collected 2 parent candidates, we end the observation phase earlier
                      if (!m_endObserveId.IsExpired ())
                        {
                          EndObservePhase ();
                          m_endObserveId.Cancel ();
                        }
                    }
                }
            }
          else if (m_state == SEEK_JOIN)
            {
              NS_ASSERT (privateChannel < NUM_CHANNELS);
              NS_ASSERT (parentChannel < NUM_CHANNELS);

              m_interfererChannels[privateChannel]++;
              if (privateChannel != parentChannel)
                {
                  //the gateway may use the same parent/private channel
                  m_interfererChannels[parentChannel]++;
                }

              //for(auto iter = m_interfererChannels.begin(); iter != m_interfererChannels.end(); iter++){
              //  NS_LOG_DEBUG(*iter);
              //}

              if (src == m_parent.parentAddr)
                {
                  //All the following code should go to a schedule after the backoff delay
                  m_maxBackoff = seekHdr.GetMaxBackoff ();
                  m_parent.ulChannel = privateChannel;

                  double delay = m_uniformRV->GetValue (0, m_maxBackoff);

                  m_nextAcceptJoin = Simulator::Now () + Seconds (timeTillNextAcceptJoin);
                  Simulator::Schedule (Seconds (timeTillNextAcceptJoin),
                                       &CottoncandyMac::EnterAcceptJoinPhase, this);
                  m_nextDutyCycleKnown = true;

                  NS_LOG_DEBUG ("Next duty cycle starts after " << std::dec << timeTillNextAcceptJoin);

                  Simulator::Schedule (Seconds (delay), &CottoncandyMac::EndSeekJoinPhase, this);
                }
            }
          break;
        }
      case CottoncandyMacHeader::MsgType::JOIN:
        {
          if (m_state != ACCEPT_JOIN)
            {
              break;
            }

          NS_LOG_DEBUG ("JOIN message received");

          // If the node itself is not connected to the gateway or had enough children already
          if (m_numOutgoingJoinAck + m_numChildren >= MAX_NUM_CHILDREN)
            {
              NS_LOG_DEBUG ("Unable to accept child node any more " << (int) m_parent.hops << " "
                                                                    << (int) m_numChildren);
              break;
            }

          // Construct a JOIN_ACK packet and send back
          Ptr<Packet> packetToSend = Create<Packet> ();

          CottoncandyMacHeader macHdr;
          macHdr.SetType (CottoncandyMacHeader::JOIN_ACK);
          macHdr.SetSrc (m_address.Get ());
          macHdr.SetDest (src.Get ());

          CottoncandyJoinAckHeader ackHdr;

          if (m_address.Get () & CottoncandyAddress::GATEWAY_MASK)
            {
              ackHdr.SetHops (1);
            }
          else
            {
              ackHdr.SetHops (m_parent.hops + 1);
            }

          int rssi = (int) rxPower;

          ackHdr.SetRssi (rssi);
          ackHdr.SetNumChildren (m_numChildren);

          packetToSend->AddHeader (ackHdr);
          packetToSend->AddHeader (macHdr);

          //Reply back immediately
          Send (packetToSend, GetChannelFreq (m_channel), MAX_TX_POWER);

          NS_LOG_DEBUG ("JOIN_ACK message will be sent to node " << macHdr.GetDest ());

          m_numOutgoingJoinAck++;

          break;
        }

      case CottoncandyMacHeader::MsgType::JOIN_ACK:
        {
          // If thre is already a parent node
          if (m_state != DISCONNECTED)
            {
              break;
            }
          NS_LOG_DEBUG ("JOIN_ACK message received");

          CottoncandyJoinAckHeader ackHdr;
          packetCopy->RemoveHeader (ackHdr);

          NS_ASSERT (m_currentCandidate.parentAddr == src);

          m_currentCandidate.hops = ackHdr.GetHops ();
          m_currentCandidate.numChildren = ackHdr.GetNumChildren ();
          m_currentCandidate.linkQuality = std::min (ackHdr.GetRssi (), (int) rxPower);

          NS_LOG_DEBUG ("Possible parent with link quaility = " << std::dec << m_currentCandidate.linkQuality
                                                                << " and hops = "
                                                                << (int) m_currentCandidate.hops);

          if (m_currentCandidate.linkQuality > -110)
            {
              // Less hop is always prefered
              if (m_currentCandidate.hops == bestParentCandidate.hops)
                {
                  if (m_currentCandidate.numChildren == bestParentCandidate.numChildren)
                    {
                      // Final tiebreaker using the link quality
                      bestParentCandidate =
                        (m_currentCandidate.linkQuality > bestParentCandidate.linkQuality)
                            ? m_currentCandidate
                            : bestParentCandidate;
                    }
                  else
                    {
                      // First tiebreaker using the number of children
                      bestParentCandidate =
                        (m_currentCandidate.numChildren < bestParentCandidate.numChildren)
                            ? m_currentCandidate
                            : bestParentCandidate;
                    }
                }
              else
                {
                  bestParentCandidate = (m_currentCandidate.hops < bestParentCandidate.hops)
                                          ? m_currentCandidate
                                          : bestParentCandidate;
                }
            }
          else if (m_txPower == MAX_TX_POWER && bestParentCandidate.hops == 255)
            {
              // Take whatever possible if we have reached the max Tx power and there is still no parent
              bestParentCandidate = m_currentCandidate;
            }
          break;
        }

      case CottoncandyMacHeader::MsgType::JOIN_CFM:
        {
          auto child = pendingChildren.find (src);
          if (child != pendingChildren.end ())
            {
              m_numOutgoingJoinAck--;
              pendingChildren.erase (child);
            }

          childList.push_back (src);
          m_numChildren++;
          NS_LOG_DEBUG ("JOIN_CFM message received. Now have " << (int) m_numChildren << " children");
          break;
        }

      case CottoncandyMacHeader::MsgType::GATEWAY_REQ:
        {
          //Gateway itself does not need to reply
          if (m_address.isGateway ())
            {
              break;
            }

          //Not from parent
          if (src != m_parent.parentAddr)
            {
              break;
            }

          //Not in the right state
          if (m_state != LISTEN_TO_PARENT)
            {
              break;
            }

          NS_LOG_DEBUG ("Request from parent");

          //Logging
          m_gatewayReqReceived (m_address.Get ());

          CottoncandyGatewayReqHeader reqHdr;
          packetCopy->RemoveHeader (reqHdr);

          //Extract the ul channel

          if (!m_nextDutyCycleKnown)
            {
              uint32_t timeTillNextAcceptJoin = reqHdr.GetNextReqTime ();
              m_nextAcceptJoin = Seconds (timeTillNextAcceptJoin) + Simulator::Now ();

              Simulator::Schedule (Seconds (timeTillNextAcceptJoin), &CottoncandyMac::EnterAcceptJoinPhase, this);

              m_maxBackoff = (double) reqHdr.GetMaxBackoff ();

              m_nextDutyCycleKnown = true;
            }

          Time delay = Seconds (m_uniformRV->GetValue (MIN_BACKOFF, m_maxBackoff));
          Time airtime = Seconds (0);

          if (m_PendingData.size () > 0)
            {
              if (m_PendingData.size () == 1)
                {
                  NS_LOG_DEBUG ("Sending local reply to parent 0x" << std::hex << m_parent.parentAddr.Get ());
                  Ptr<Packet> reply =  m_PendingData.front ()->Copy ();

                  //Modify the destination field in the header
                  CottoncandyMacHeader mHdr;
                  reply->RemoveHeader (mHdr);
                  mHdr.SetDest (m_parent.parentAddr.Get ());
                  reply->AddHeader (mHdr);

                  Simulator::Schedule (delay, &CottoncandyMac::Send, this, reply,
                                       GetChannelFreq (m_parent.ulChannel), m_txPower);

                  airtime = m_phy->GetObject<EndDeviceLoraPhy>()->GetOnAirTime (reply, m_params);

                  m_PendingData.erase (m_PendingData.begin ());

                }
              else
                {

                  int aggregationSize = 0;
                  int aggregationCounter = 0;
                  Ptr<Packet> aggregatedPacket = Create<Packet> ();

                  for (auto iter = m_PendingData.begin (); iter != m_PendingData.end (); )
                    {
                      NS_LOG_DEBUG (*iter);
                      Ptr<Packet> pendingPacket = (*iter)->Copy ();

                      CottoncandyMacHeader macHdr;
                      pendingPacket->RemoveHeader (macHdr);

                      CottoncandyNodeReplyHeader replyHdr;
                      pendingPacket->RemoveHeader (replyHdr);

                      int size = 0;
                      if (replyHdr.GetOption () & 0x80)
                        {
                          size = replyHdr.GetDataLen ();
                        }
                      else
                        {
                          size =  replyHdr.GetDataLen () + 3;//Add the miniheader size
                        }

                      if (size + aggregationSize > 64)
                        {
                          break;
                        }

                      else
                        {
                          if (replyHdr.GetOption () & 0x80)
                            {
                              aggregatedPacket->AddAtEnd (pendingPacket);
                            }
                          else
                            {
                              //If the packet is not yet aggregated
                              CottoncandyNodeReplyEmbeddedHeader miniHdr;
                              miniHdr.SetSrc (macHdr.GetSrc ());
                              miniHdr.SetDataLen (replyHdr.GetDataLen ());
                              pendingPacket->AddHeader (miniHdr);

                              aggregatedPacket->AddAtEnd (pendingPacket);
                            }
                          aggregationSize += size;
                          aggregationCounter++;

                          //Remove the packet in the list
                          iter = m_PendingData.erase (iter);
                        }
                    }

                  CottoncandyMacHeader newMHdr;
                  newMHdr.SetType (CottoncandyMacHeader::MsgType::NODE_REPLY);
                  newMHdr.SetDest (m_parent.parentAddr.Get ());
                  newMHdr.SetSrc (m_address.Get ());

                  CottoncandyNodeReplyHeader replyHdr;
                  replyHdr.SetDataLen (aggregationSize);

                  uint8_t option = 0xA0;
                  if (m_PendingData.size () != 0)
                    {
                      option |= 0x40;
                    }
                  replyHdr.SetOption (option);

                  aggregatedPacket->AddHeader (replyHdr);
                  aggregatedPacket->AddHeader (newMHdr);

                  Simulator::Schedule (delay, &CottoncandyMac::Send, this, aggregatedPacket,
                                       GetChannelFreq (m_parent.ulChannel), m_txPower);

                  airtime = m_phy->GetObject<EndDeviceLoraPhy>()->GetOnAirTime (aggregatedPacket, m_params);
                }
            }
          else
            {
              NS_LOG_DEBUG ("I have no data");
            }

          if (m_numChildren == 0)
            {
              if (!m_endDCPId.IsExpired ())
                {
                  //Cancel the timeout since we are ending the data collection
                  m_endDCPId.Cancel ();
                  EndDataCollectionPhase ();
                }
            }

          m_state = (m_PendingData.size () == 0) ? TALK_TO_CHILDREN : LISTEN_TO_PARENT;

          if (m_state == TALK_TO_CHILDREN)
            {
              Simulator::Schedule (delay + airtime, &CottoncandyMac::TalkToChildren, this);
            }

          break;
        }
      case CottoncandyMacHeader::MsgType::NODE_REPLY:
        {
          if (m_state != TALK_TO_CHILDREN)
            {
              break;
            }

          if (m_address.isGateway ())
            {
              //Record the successful reception

              CottoncandyNodeReplyHeader replyHdr;
              packetCopy->RemoveHeader (replyHdr);

              m_emptyRounds = 0;

              if (replyHdr.GetOption () & 0x80)
                {
                  while (packetCopy->GetSize () > 0)
                    {
                      CottoncandyNodeReplyEmbeddedHeader miniHdr;

                      packetCopy->RemoveHeader (miniHdr);

                      m_replyDelivered (miniHdr.GetSrc ());
                      NS_LOG_DEBUG ("NodeReply arrived at the gateway. (mini)Src = 0x" << std::hex << miniHdr.GetSrc ());

                      packetCopy->RemoveAtStart (miniHdr.GetDataLen ());
                    }
                }
              else
                {
                  NS_LOG_DEBUG ("NodeReply arrived at the gateway. Src = 0x" << std::hex << src.Get ());
                  m_replyDelivered (src.Get ());
                }
            }
          else
            {
              NS_LOG_DEBUG ("Reply received from child 0x" << std::hex << src.Get ());
              //A node simply places the received packet in the buffer
              packetCopy->AddHeader (mHdr);
              m_PendingData.push_back (packetCopy);
            }

          break;
        }
      default:
        {
          NS_LOG_INFO ("Unknown message received");
        }
    }
}

void
CottoncandyMac::TalkToChildren ()
{
  m_state = TALK_TO_CHILDREN;
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_DEBUG ("Send out a gatewayReq");
  Ptr<Packet> packet = Create<Packet> ();

  CottoncandyMacHeader macHdr;
  macHdr.SetType (CottoncandyMacHeader::MsgType::GATEWAY_REQ);
  macHdr.SetSrc (m_address.Get ());
  macHdr.SetDest (BROADCAST_ADDR.Get ());

  //Add the remaining GatewayReq header
  CottoncandyGatewayReqHeader reqHdr;
  reqHdr.SetOption (0xC0); //TODO: Set the option properly
  reqHdr.SetChannel (m_channel);

  Time timeTillNextAcceptJoin = m_nextAcceptJoin - Simulator::Now ();
  reqHdr.SetNextReqTime ((uint32_t) std::ceil (timeTillNextAcceptJoin.GetSeconds ()));

  uint8_t maxBackoff = m_numChildren * 3;
  reqHdr.SetMaxBackoff (maxBackoff);

  packet->AddHeader (reqHdr);
  packet->AddHeader (macHdr);

  //Set the node to receive on the channel
  SetChannel (m_channel);

  Send (packet, GetChannelFreq (m_channel), MAX_TX_POWER);

  //After sending, the device will automatically enter STANDBY (essentially receiving)
  Time airtime = m_phy->GetOnAirTime (packet, m_params);

  Time listenDuration = (m_numChildren == 0) ? Seconds (1) : Seconds (maxBackoff);

  //Set up the timeout
  Simulator::Schedule (airtime + listenDuration, &CottoncandyMac::ReceiveTimeout,
                       this);

}

void
CottoncandyMac::ReceiveTimeout ()
{
  NS_LOG_DEBUG ("Receive timeout");
  if (m_PendingData.size () > 0)
    {
      m_state = LISTEN_TO_PARENT;

      SetChannel (m_parent.ulChannel);

      m_emptyRounds = 0;
    }
  else
    {

      m_state = HIBERNATE;

      m_emptyRounds++;

      if (m_emptyRounds >= MAX_EMPTY_ROUNDS)
        {
          //5 consecutive hibernations with no data

          //Terminte the data collection phase
          if (!m_endDCPId.IsExpired ())
            {
              //Cancel the timeout since we are ending the data collection
              m_endDCPId.Cancel ();
              EndDataCollectionPhase ();
            }
        }
      else
        {
          //Send out the request again after 10 seconds
          Simulator::Schedule (SHORT_HIBERNATION_DURATION, &CottoncandyMac::TalkToChildren, this);
        }
    }
}

void
CottoncandyMac::EndSeekJoinPhase ()
{
  if (!m_address.isGateway () && m_channel == INVALID_CHANNEL)
    {
      //The channel ranges from 0 to 63, the channel 0 is a public channel,
      //Therefore the private channels starts at 1
      int leastUsedChannel = 0;
      int leastUsedTimes = m_interfererChannels[0];
      for (int i = 0; i < NUM_CHANNELS; i++)
        {
          if (m_interfererChannels[i] == 0)
            {
              leastUsedChannel = i;
              break;
            }

          if (m_interfererChannels[i] < leastUsedTimes)
            {
              leastUsedChannel = i;
              leastUsedTimes = m_interfererChannels[i];
            }
        }

      m_channel = leastUsedChannel;
      NS_LOG_DEBUG ("My channel is " << std::dec << (unsigned int)m_channel);
    }

  NS_LOG_DEBUG ("Send out a SEEK_JOIN message");
  Ptr<Packet> packet = Create<Packet> ();

  CottoncandyMacHeader macHdr;
  macHdr.SetType (CottoncandyMacHeader::MsgType::SEEK_JOIN);
  macHdr.SetSrc (m_address.Get ());
  macHdr.SetDest (BROADCAST_ADDR.Get ());

  //Add the remaining Seekjoin header
  CottoncandySeekJoinHeader seekHdr;
  seekHdr.SetPrivateChannel (m_channel);
  seekHdr.SetParentChannel (m_parent.ulChannel);
  seekHdr.SetNumChildren (m_numChildren);

  Time timeTillNextAcceptJoin = m_nextAcceptJoin - Simulator::Now ();

  //NS time does rounding automatically when convert from double to integer, we use the floor so that a node will not wake up late
  seekHdr.SetNextAcceptJoinTime ((uint32_t) std::floor (timeTillNextAcceptJoin.GetSeconds ()));
  seekHdr.SetMaxBackoff (m_numChildren * 3);

  packet->AddHeader (seekHdr);
  packet->AddHeader (macHdr);

  Send (packet, GetChannelFreq (PUBLIC_CHANNEL), MAX_TX_POWER);

  if (m_state != SEEK_JOIN)
    {
      //If we are already in the data collection phase
      NS_LOG_DEBUG ("No longer in the SEEK_JOIN state");
      return;
    }
  else
    {
      m_state = HIBERNATE;
    }

}

void
CottoncandyMac::Send (Ptr<Packet> packet, double freq, double txPower)
{

  // Should only have 1 channel
  std::vector<Ptr<LogicalLoraChannel> > logicalChannels;
  logicalChannels = m_channelHelper.GetEnabledChannelList ();
  NS_ASSERT (logicalChannels.size () == 1);

  Ptr<LogicalLoraChannel> channel = logicalChannels.at (0);

  Time nextTxDelay = m_channelHelper.GetWaitingTime (channel);

  if (nextTxDelay == Seconds (0))
    {
      DoSend (packet, freq, txPower);
    }
  else
    {
      //Dison: This part is never actually used
      NS_LOG_DEBUG ("Need to postphone the TX");
      Simulator::Schedule (nextTxDelay, &CottoncandyMac::DoSend, this, packet, freq, txPower);
    }
}

void
CottoncandyMac::DoSend (Ptr<Packet> packet, double freq, double txPower)
{
  m_phy->Send (packet, m_params, freq, txPower);
}

void
CottoncandyMac::SetChannel (uint8_t channel)
{
  m_phy->GetObject<EndDeviceLoraPhy> ()->SetFrequency (GetChannelFreq (channel));
}

void
CottoncandyMac::Run ()
{

  m_phy->GetObject<EndDeviceLoraPhy> ()->SwitchToStandby ();

  m_numChildren = 0;

  //Initialize the list of channel usage
  std::fill (m_interfererChannels.begin (), m_interfererChannels.end (), 0);

  if (m_address.Get () & CottoncandyAddress::GATEWAY_MASK)
    {
      NS_LOG_DEBUG ("This is a gateway device");
      m_connectionEstablished (m_address.Get (), 0, m_phy->GetMobility ()->GetPosition ());

      m_parent.hops = 0;
      m_parent.ulChannel = m_channel; //The gateway sets the ulChannel to the m_channel

      EnterAcceptJoinPhase ();

      //Selects a random private channel to use
      m_channel = m_uniformRV->GetInteger (0, NUM_CHANNELS);
    }
  else
    {
      EnterObservePhase ();
    }
}

void
CottoncandyMac::SetDevice (Ptr<NetDevice> device)
{
  m_device = device;
}

void
CottoncandyMac::SetDeviceAddress (CottoncandyAddress addr)
{

  NS_LOG_DEBUG ("Address set to " << addr.Print ());
  m_address = addr;
}

void
CottoncandyMac::SetReplyLen (uint8_t len)
{
  m_replyLen = len;
}

void
CottoncandyMac::EnterSeekJoinPhase ()
{
  NS_LOG_DEBUG ("Enter SeekJoin phase");
  m_state = SEEK_JOIN;

  //Listen on the public channel
  SetChannel (PUBLIC_CHANNEL);

  Simulator::Schedule (SEEK_JOIN_DURATION, &CottoncandyMac::EnterDataCollectionPhase, this);

  // Clear the expired entries in the JOIN phase
  pendingChildren.clear ();
  m_numOutgoingJoinAck = 0;

  if (m_address.isGateway ())
    {
      Simulator::Schedule (Seconds (3), &CottoncandyMac::EndSeekJoinPhase, this);
    }
}

void
CottoncandyMac::EnterDataCollectionPhase ()
{
  NS_LOG_DEBUG ("Enter Data Collection phase");

  pendingChildren.clear ();
  m_phy->GetObject<EndDeviceLoraPhy>()->SwitchToStandby ();

  m_endDCPId = Simulator::Schedule (DCP_TIMEOUT, &CottoncandyMac::EndDataCollectionPhase, this);

  if (m_address.isGateway ())
    {
      //The gateway sends out the first gateway request message
      m_state = TALK_TO_CHILDREN;

      //The gateway should delay for a few seconds before sending out the first request due to time errors

      Simulator::Schedule (Seconds (3), &CottoncandyMac::TalkToChildren, this);
    }
  else
    {
      m_state = LISTEN_TO_PARENT;

      //Listen on the private channel of the parent
      SetChannel (m_parent.ulChannel);

      // Prepare a NodeReply packet of local data
      //Note: In simulation, we just create a zero-filled payload
      Ptr<Packet> replyPacket = Create<Packet> (m_replyLen);

      CottoncandyNodeReplyHeader replyHdr;
      replyHdr.SetOption (0x20);
      replyHdr.SetDataLen (m_replyLen);

      replyPacket->AddHeader (replyHdr);

      CottoncandyMacHeader macHdr;
      macHdr.SetType (CottoncandyMacHeader::MsgType::NODE_REPLY);
      macHdr.SetSrc (m_address.Get ());
      macHdr.SetDest (m_parent.parentAddr.Get ());

      replyPacket->AddHeader (macHdr);

      //Place the packet into the "waiting area"
      m_PendingData.push_back (replyPacket);
    }
}

void
CottoncandyMac::EnterAcceptJoinPhase ()
{
  NS_LOG_DEBUG ("Enter AcceptJoin Phase");
  m_state = ACCEPT_JOIN;

  m_phy->GetObject<EndDeviceLoraPhy>()->SwitchToStandby ();

  //Listen on the private channel of the node
  SetChannel (m_channel);

  Simulator::Schedule (ACCEPT_JOIN_DURATION, &CottoncandyMac::EnterSeekJoinPhase, this);

  if (m_address.isGateway ())
    {
      //Set the next duty cycle start time
      m_nextAcceptJoin = Simulator::Now () + Seconds (3600);
      Simulator::Schedule ( Seconds (3600), &CottoncandyMac::EnterAcceptJoinPhase, this);
    }
  else
    {
      //Initialize
      m_nextDutyCycleKnown = false;
    }
}

void
CottoncandyMac::EndDataCollectionPhase ()
{
  NS_LOG_DEBUG ("End data collection phase");

  //Reset
  m_emptyRounds = 0;
  m_PendingData.clear ();

  if (!m_address.isGateway () && !m_nextDutyCycleKnown)
    {
      //We lost synchronization with the rest of the network
      EnterObservePhase ();
    }

}

/**************************************** Phase Transitions for Network Formation ******************************************/

void
CottoncandyMac::EnterObservePhase ()
{
  NS_LOG_DEBUG ("Enter Observation Phase");
  m_state = OBSERVE;
  m_phy->GetObject<EndDeviceLoraPhy> ()->SwitchToStandby ();
  SetChannel (PUBLIC_CHANNEL);

  bestParentCandidate = ParentNode ();
  bestParentCandidate.hops = 255;

  m_channel = INVALID_CHANNEL;
}

void
CottoncandyMac::EndObservePhase ()
{
  NS_LOG_DEBUG ("End Observation Phase");
  m_state = HIBERNATE;
  //Stop receiving
}

void
CottoncandyMac::EnterJoinPhase ()
{
  NS_LOG_DEBUG ("Enter Join Phase");
  m_state = DISCONNECTED;
  m_phy->GetObject<EndDeviceLoraPhy> ()->SwitchToStandby ();

  NS_ASSERT (m_candidateParents.size () > 0);

  //Set a timeout for the join phase
  Simulator::Schedule (ACCEPT_JOIN_DURATION, &CottoncandyMac::EndJoinPhase, this);

  //Start the first join request
  Join ();
}

void
CottoncandyMac::EndJoinPhase ()
{
  NS_LOG_DEBUG ("End Join Phase");
  //At the end of join phase, if we have already joined -> EnterSeekJoin(), otherwise, we do nothing (already done at discovery timeout)
  if (m_state != CONNECTED && m_state != SEEK_JOIN)
    {
      EnterObservePhase ();
    }
}

void
CottoncandyMac::Join ()
{

  //Pick the first candidate parent
  auto iter = m_candidateParents.begin ();

  if (iter != m_candidateParents.end ())
    {

      CottoncandyAddress candidateAddr = iter->first;
      uint8_t candidateChannel = iter->second;

      m_currentCandidate.parentAddr = candidateAddr;
      m_currentCandidate.ulChannel = candidateChannel;

      //Remove the candidate
      m_candidateParents.erase (iter);

      //Listen on the parent channel
      m_phy->GetObject<EndDeviceLoraPhy> ()->SetFrequency (GetChannelFreq (candidateChannel));

      CottoncandyMacHeader mHdr;
      mHdr.SetType (CottoncandyMacHeader::JOIN);
      mHdr.SetDest (candidateAddr.Get ());
      mHdr.SetSrc (m_address.Get ());

      Ptr<Packet> joinPacket = Create<Packet> ();
      joinPacket->AddHeader (mHdr);

      //Wait for a small delay before sending (avoiding collisions with other new nodes who want to join)
      double delay = m_uniformRV->GetValue (0, MAX_BACKOFF_JOIN);

      //Send the join packet to the target node
      Simulator::Schedule (Seconds (delay), &CottoncandyMac::Send, this, joinPacket,
                           GetChannelFreq (candidateChannel), m_txPower);

      Time airtime = m_phy->GetOnAirTime (joinPacket, m_params);

      //Time out 1 second after the packet is sent
      Simulator::Schedule (Seconds (delay) + airtime + JOIN_ACK_TIMEOUT,
                           &CottoncandyMac::DiscoveryTimeout, this);

      NS_LOG_DEBUG ("Sending Join request to candidate " << std::hex << candidateAddr.Get ());
    }
}

void
CottoncandyMac::DiscoveryTimeout ()
{
  NS_LOG_DEBUG ("Discovery timeout");

  if (m_candidateParents.size () == 0)
    {
      if (bestParentCandidate.hops != 255)
        {
          m_parent = bestParentCandidate;
          //Now we can finalize the parent selection by sending a CFM to the parent
          CottoncandyMacHeader macHdr;
          macHdr.SetType (CottoncandyMacHeader::JOIN_CFM);
          macHdr.SetSrc (m_address.Get ());
          macHdr.SetDest (m_parent.parentAddr.Get ());

          Ptr<Packet> cfmPacket = Create<Packet> ();
          cfmPacket->AddHeader (macHdr);

          NS_LOG_DEBUG ("Join Successful. Parent Node is " << std::hex << m_parent.parentAddr.Get ());

          //Callback to log the connection
          m_connectionEstablished (m_address.Get (), m_parent.parentAddr.Get (),
                                   m_phy->GetMobility ()->GetPosition ());

          std::cout << m_address.Get () << " Final tx power set to " << m_txPower << std::endl;

          Send (cfmPacket, GetChannelFreq (m_parent.ulChannel), m_txPower);

          //Switch to the seekjoin phase (which should be upcoming)
          Time airtime = m_phy->GetObject<EndDeviceLoraPhy> ()->GetOnAirTime (cfmPacket, m_params);
          Simulator::Schedule (airtime, &CottoncandyMac::EnterSeekJoinPhase, this);

          m_state = CONNECTED;
        }
      else
        {
          //We cannot find a good parent after going through all candidates
          m_state = OBSERVE;

          if (m_txPower < MAX_TX_POWER)
            {
              //Increment the TX power
              m_txPower++;
            }
        }
    }
  else
    {
      //Do a join immediately
      Simulator::Schedule (Seconds (0), &CottoncandyMac::Join, this);
    }
}

} // namespace lorawan
} // namespace ns3
