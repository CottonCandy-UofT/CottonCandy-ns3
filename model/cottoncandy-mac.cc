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
  static TypeId tid = TypeId ("ns3::CottoncandyMac")
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
    .AddTraceSource("GatewayReqReceived",
                    "Trace source indicating a node has received a"
                    "request from the gateway",
                    MakeTraceSourceAccessor(&CottoncandyMac::m_gatewayReqReceived),
                    "ns3::TracedValueCallback::uint16_t")
    .AddTraceSource("ReplyDelivered",
                    "Trace source indicating the gateway has received a"
                    "reply from a node",
                    MakeTraceSourceAccessor(&CottoncandyMac::m_replyDelivered),
                    "ns3::TracedValueCallback::uint16_t")
    .AddTraceSource("HalfDuplexDetected",
                    "Trace source indicating a half-duplex event",
                    MakeTraceSourceAccessor(&CottoncandyMac::m_halfDuplexDetected),
                    "ns3::TracedValueCallback::uint16_t")
    .AddTraceSource("CollisionDetected",
                    "Trace source indicating a failed RX event",
                    MakeTraceSourceAccessor(&CottoncandyMac::m_collisionDetected),
                    "ns3::TracedValueCallback::uint8_t")
    .AddConstructor<CottoncandyMac> ();
  return tid;
}

CottoncandyMac::CottoncandyMac ()
{
  NS_LOG_FUNCTION (this);

  // Initialize the random variable we'll use to decide which channel to
  // transmit on.
  m_uniformRV = CreateObject<UniformRandomVariable> ();

  // Craft LoraTxParameters object
  m_params.sf = 7;
  m_params.headerDisabled = 0;
  m_params.codingRate = 1;
  m_params.bandwidthHz = 125e3;
  m_params.nPreamble = 8;
  m_params.crcEnabled = 1;
  m_params.lowDataRateOptimizationEnabled = 0;

  m_numChildren = 0;
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
  m_phy->SetHalfDuplexCallback(MakeCallback(&CottoncandyMac::ReportHalfDuplex, this));
}

void CottoncandyMac::FailedReception(Ptr<Packet const> packet){
  Ptr<Packet> packetCopy = packet->Copy();

  CottoncandyMacHeader mHdr;

  packetCopy->RemoveHeader(mHdr);

  CottoncandyAddress dest = CottoncandyAddress(mHdr.GetDest());
  
  uint8_t remainingHops = m_parent.hops + 1;

  if(dest == m_address && mHdr.GetType() == CottoncandyMacHeader::NODE_REPLY){
    NS_LOG_DEBUG("A packet RX failed due to collisions");
    m_collisionDetected(remainingHops);
  }
}

void CottoncandyMac::ReportHalfDuplex(Ptr<Packet const> packet){
  Ptr<Packet> packetCopy = packet->Copy();

  CottoncandyMacHeader mHdr;

  packetCopy->RemoveHeader(mHdr);

  CottoncandyAddress dest = CottoncandyAddress(mHdr.GetDest());

  // Report the half-duplex problem when the packet is intended for this node and 
  // is a reply message (we are not interested in other messages)
  if(dest == m_address && mHdr.GetType() == CottoncandyMacHeader::NODE_REPLY){
    NS_LOG_DEBUG("A packet RX failed due to half duplex");
    m_halfDuplexDetected(m_address.Get());
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
  NS_LOG_FUNCTION (this << unsigned(dataRate));

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
  NS_LOG_FUNCTION (this << unsigned(dataRate));

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
void CottoncandyMac::Receive(Ptr<Packet const> packet)
{

  Ptr<Packet> packetCopy = packet->Copy();

  CottoncandyMacHeader mHdr;
  packetCopy->RemoveHeader(mHdr);

  CottoncandyAddress src = CottoncandyAddress(mHdr.GetSrc());
  CottoncandyAddress dest = CottoncandyAddress(mHdr.GetDest());
  NS_LOG_INFO("Packet arrived at CottoncandyMac from " << src.Print());
  
  // Check if the message is intended for us
  if (dest != m_address && dest != BROADCAST_ADDR ){
    NS_LOG_INFO("Packet dropped. The packet is for node " << dest.Print());
    return;
  }

  uint8_t type = mHdr.GetType();

  switch(type){
    case CottoncandyMacHeader::MsgType::JOIN:
    {
      NS_LOG_DEBUG("JOIN message received");

      // If the node itself is not connected to the gateway or had enough children already
      if(m_parent.hops == 255 || m_numChildren >= 3){
        NS_LOG_DEBUG("Unable to accept child node any more " << (int)m_parent.hops << " " << (int)m_numChildren);
        break;
      }

      // Construct a JOIN_ACK packet and send back
      Ptr<Packet> packetToSend = Create<Packet>();
      
      CottoncandyMacHeader macHdr;
      macHdr.SetType(CottoncandyMacHeader::JOIN_ACK);
      macHdr.SetSrc(m_address.Get());
      macHdr.SetDest(src.Get());

      CottoncandyJoinAckHeader ackHdr;
      
      if(m_address.Get() & CottoncandyAddress::GATEWAY_MASK){
        ackHdr.SetHops(1);
      }else{
        ackHdr.SetHops(m_parent.hops + 1);
      }
      packetToSend->AddHeader(ackHdr);
      packetToSend->AddHeader(macHdr);

      Time delay = Seconds(m_uniformRV->GetValue(MIN_BACKOFF, MAX_BACKOFF));

      // Send the packet after the random backoff
      Simulator::Schedule(delay,&CottoncandyMac::Send, this, packetToSend);

      NS_LOG_DEBUG("JOIN_ACK message will be sent to node " << macHdr.GetDest());
      
      break;
    }

    case CottoncandyMacHeader::MsgType::JOIN_ACK:
    {
      NS_LOG_DEBUG("JOIN_ACK message received");
      // If thre is already a parent node
      if (m_parent.hops != 255){
        break;
      }
      CottoncandyJoinAckHeader ackHdr;
      packetCopy->RemoveHeader(ackHdr);

      uint8_t hops = ackHdr.GetHops();
      
      // Use the tag to know the RSSI strength
      LoraTag tag;
      packetCopy->RemovePacketTag(tag);
      double rxPower = tag.GetReceivePower();
      NS_LOG_DEBUG("Possible parent with rssi = " << rxPower << " and hops = " << (int)hops);

      // Update the parent candidate based on better hops
      if(bestParentCandidate.hops != 255){

        if (rxPower > -120){
          if(hops < bestParentCandidate.hops || (hops == bestParentCandidate.hops && 
                  rxPower > bestParentCandidate.rssi)){
            
            bestParentCandidate.parentAddr = src;
            bestParentCandidate.hops = hops;
            bestParentCandidate.rssi = rxPower;
            NS_LOG_DEBUG("This is a better parent node.");
          }
        }
      }else{
        bestParentCandidate.parentAddr = src;
        bestParentCandidate.hops = hops;
        bestParentCandidate.rssi = rxPower;
        NS_LOG_DEBUG("This is the first parent encountered");
      }
      break;
    }

    case CottoncandyMacHeader::MsgType::JOIN_CFM:
    {
      m_numChildren ++;
      NS_LOG_DEBUG("JOIN_CFM message received. Now have " << (int)m_numChildren << " children");
      break;
    }

    case CottoncandyMacHeader::MsgType::GATEWAY_REQ:
    {
      //Gateway itself does not need to reply
      if(m_address.Get() & CottoncandyAddress::GATEWAY_MASK){
        break;
      }

      if(src != m_parent.parentAddr){
        break;
      }

      m_gatewayReqReceived(m_address.Get());

      CottoncandyGatewayReqHeader reqHdr;
      packetCopy->RemoveHeader(reqHdr);

      m_maxBackoff = (double)reqHdr.GetMaxBackoff();

      // Construct a NodeReply packet and send back
      // We do not concern about the extra header (i.e. sequence number & datalen)
      // So we just create a zero-filled payload and send
      Ptr<Packet> replyPacket = Create<Packet>(2 + m_replyLen);
      
      CottoncandyMacHeader macHdr;
      macHdr.SetType(CottoncandyMacHeader::MsgType::NODE_REPLY);
      macHdr.SetSrc(m_address.Get());
      macHdr.SetDest(src.Get());

      replyPacket->AddHeader(macHdr);

      Time delay = Seconds(m_uniformRV->GetValue(MIN_BACKOFF, m_maxBackoff));

      NS_LOG_DEBUG("Send NodeReply to the parent");
      // Send the packet after the random backoff
      Simulator::Schedule(delay,&CottoncandyMac::Send, this, replyPacket);

      if(m_numChildren == 0){
        break;
      }

      //Re-add the headers with modified info
      mHdr.SetSrc(m_address.Get());

      reqHdr.SetMaxBackoff(m_numChildren * 3);

      packetCopy->AddHeader(reqHdr);
      packetCopy->AddHeader(mHdr);
      
      Time forwardDelay = Seconds(m_maxBackoff) + Seconds(m_uniformRV->GetValue(MIN_BACKOFF, m_maxBackoff));
      
      NS_LOG_DEBUG("Forward GatewayReq to the children");
      Simulator::Schedule(forwardDelay,&CottoncandyMac::Send, this, packetCopy);

      break;
    }
    case CottoncandyMacHeader::MsgType::NODE_REPLY:
    {
      if(m_address.Get() & CottoncandyAddress::GATEWAY_MASK){
        //Record the successful reception
        NS_LOG_DEBUG("NodeReply arrived at the gateway. Src = " << src.Print());
        m_replyDelivered(src.Get());
        break;
      }
      
      //Forward the packet by modifying the packet dest field
      mHdr.SetDest(m_parent.parentAddr.Get());

      packetCopy->AddHeader(mHdr);

      Time delay = Seconds(m_uniformRV->GetValue(MIN_BACKOFF, m_maxBackoff));

      // Send the packet after the random backoff
      Simulator::Schedule(delay,&CottoncandyMac::Send, this, packetCopy);

      break;

    }
    default:
    {
      NS_LOG_INFO("Unknown message received");
    }
  
  }
}

void CottoncandyMac::SendGatewayRequest(){

  NS_LOG_FUNCTION_NOARGS();
  
  if(m_numChildren == 0){
    return;
  }

  NS_LOG_DEBUG("Send out a gatewayReq");
  Ptr<Packet> packet = Create<Packet>();

  CottoncandyMacHeader macHdr;
  macHdr.SetType(CottoncandyMacHeader::MsgType::GATEWAY_REQ);
  macHdr.SetSrc(m_address.Get());
  macHdr.SetDest(BROADCAST_ADDR.Get());

  //Add the remaining GatewayReq header
  CottoncandyGatewayReqHeader reqHdr;
  reqHdr.SetSeqNumber(m_seqNum);
  reqHdr.SetNextReqTime(m_reqInterval);
  reqHdr.SetMaxBackoff(m_numChildren * 3);

  packet->AddHeader(reqHdr);
  packet->AddHeader(macHdr);

  Send(packet);

  if(m_seqNum != 255){
    m_seqNum ++;
    Simulator::Schedule(Seconds((double)m_reqInterval), &CottoncandyMac::SendGatewayRequest, this);
  }else{
    //Stop after we have sent 254 requests since the seqNumber will overflow
  }
  m_gatewayReqReceived(m_address.Get());
  m_replyDelivered(m_address.Get());
  
}

void CottoncandyMac::Send(Ptr<Packet> packet){

  // Should only have 1 channel
  std::vector<Ptr<LogicalLoraChannel> > logicalChannels;
  logicalChannels = m_channelHelper.GetEnabledChannelList (); 
  NS_ASSERT(logicalChannels.size() == 1);

  Ptr<LogicalLoraChannel> channel = logicalChannels.at(0);

  Time nextTxDelay = m_channelHelper.GetWaitingTime(channel);

  if(nextTxDelay == Seconds(0)){
    DoSend(packet);
  }else{
    NS_LOG_DEBUG("Need to postphone the TX");
    Simulator::Schedule(nextTxDelay, &CottoncandyMac::DoSend, this, packet);
  }
}

void CottoncandyMac::DoSend(Ptr<Packet> packet){
  m_phy->Send(packet, m_params, COTTONCANDY_FREQUENCY, 17);
}

void CottoncandyMac::Run(){
  m_phy->GetObject<EndDeviceLoraPhy>()->SetFrequency(COTTONCANDY_FREQUENCY);

  m_phy->GetObject<EndDeviceLoraPhy> ()->SwitchToStandby ();

  if(m_address.Get() & CottoncandyAddress::GATEWAY_MASK){
    NS_LOG_DEBUG("This is a gateway device");
    m_parent.hops = 0;
    m_connectionEstablished(m_address.Get(), 0, m_phy->GetMobility()->GetPosition());

    Simulator::Schedule(Seconds((double)m_reqInterval), &CottoncandyMac::SendGatewayRequest, this);
  }
  else{
    NS_LOG_DEBUG("Initialize parent hops to 255");
    m_parent = ParentNode();
    m_parent.hops = 255; 
    m_parent.parentAddr = CottoncandyAddress(0);

    bestParentCandidate = m_parent;

    // Create a join beacon
    CottoncandyMacHeader macHdr;
    macHdr.SetType(CottoncandyMacHeader::JOIN);
    macHdr.SetSrc(m_address.Get());
    macHdr.SetDest(BROADCAST_ADDR.Get());

    m_joinBeacon = Create<Packet>();
    m_joinBeacon->AddHeader(macHdr);

    Send(m_joinBeacon);

    // Synchronous sending is used in Cottoncandy
    Time airTime = m_phy->GetOnAirTime(m_joinBeacon, m_params);

    Simulator::Schedule(airTime + m_discoveryTimeout, &CottoncandyMac::DiscoveryTimeout, this);

  }
}

void CottoncandyMac::SetDevice(Ptr<NetDevice> device){
  m_device = device;
}

void CottoncandyMac::SetDeviceAddress(CottoncandyAddress addr){

  NS_LOG_DEBUG("Address set to " << addr.Print());
  m_address = addr;
}

void CottoncandyMac::SetReplyLen(uint8_t len){
  m_replyLen = len;
}

void CottoncandyMac::DiscoveryTimeout(){
  NS_LOG_DEBUG("Discovery timeout");

  // If a parent node is found
  if (bestParentCandidate.hops != 255){
    m_parent = bestParentCandidate;

    CottoncandyMacHeader macHdr;
    macHdr.SetType(CottoncandyMacHeader::JOIN_CFM);
    macHdr.SetSrc(m_address.Get());
    macHdr.SetDest(m_parent.parentAddr.Get());

    Ptr<Packet> cfmPacket = Create<Packet>();
    cfmPacket->AddHeader(macHdr);

    Send(cfmPacket);

    NS_LOG_DEBUG("Join Successful. Parent Node is " << m_parent.parentAddr.Print());

    m_connectionEstablished(m_address.Get(), m_parent.parentAddr.Get(), m_phy->GetMobility()->GetPosition());
    

  }else{
    NS_LOG_DEBUG("Restart the discovery process");

    Time delay = Seconds(m_uniformRV->GetValue(0,5));

    // Send out the join beacon
    Simulator::Schedule(delay, &CottoncandyMac::Send, this, m_joinBeacon);

    // Synchronous sending is used in Cottoncandy
    Time airTime = m_phy->GetOnAirTime(m_joinBeacon, m_params);

    Simulator::Schedule(delay + airTime + m_discoveryTimeout, &CottoncandyMac::DiscoveryTimeout, this);
  }
}

}
}
