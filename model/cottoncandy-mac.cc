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
    .AddTraceSource ("CannotSendBecauseDutyCycle",
                     "Trace source indicating a packet "
                     "could not be sent immediately because of duty cycle limitations",
                     MakeTraceSourceAccessor (&CottoncandyMac::m_cannotSendBecauseDutyCycle),
                     "ns3::Packet::TracedCallback")
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
  //m_phy->SetReceiveFailedCallback (MakeCallback (&CottoncandyMac::FailedReception, this));
  //m_phy->SetTxFinishedCallback (MakeCallback (&CottoncandyMac::TxFinished, this));
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
  NS_LOG_DEBUG("Packet arrived at CottoncandyMac from " << src.Print());
  
  // Check if the message is intended for us
  if (dest != m_address && dest != BROADCAST_ADDR ){
    NS_LOG_DEBUG("Packet dropped. The packet is for node " << dest.Print());
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
      Ptr<Packet> packet = Create<Packet>();
      
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
      packet->AddHeader(ackHdr);
      packet->AddHeader(macHdr);

      Time delay = Seconds(m_uniformRV->GetValue(MIN_BACKOFF, MAX_BACKOFF));

      // Send the packet after the random backoff
      Simulator::Schedule(delay,&CottoncandyMac::Send, this, packet);

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
      NS_LOG_INFO("JOIN_CFM message received. Now have " << (int)m_numChildren << " children");
      break;
    }
    default:
    {
      NS_LOG_DEBUG("Unknown message received");
    }
  
  }
}

void CottoncandyMac::Send(Ptr<Packet> packet){

  NS_LOG_DEBUG("Sending a packet");

  m_phy->Send(packet, m_params, COTTONCANDY_FREQUENCY, 17);
}

void CottoncandyMac::Run(){
  m_phy->GetObject<EndDeviceLoraPhy>()->SetFrequency(COTTONCANDY_FREQUENCY);

  m_phy->GetObject<EndDeviceLoraPhy> ()->SwitchToStandby ();

  if(m_address.Get() & CottoncandyAddress::GATEWAY_MASK){
    NS_LOG_DEBUG("This is a gateway device");
    m_parent.hops = 0;
  }
  else{
    NS_LOG_DEBUG("Initialize parent hops to 255");
    m_parent = ParentNode();
    m_parent.hops = 255; 

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

    NS_LOG_INFO("Join Successful. Parent Node is " << m_parent.parentAddr.Print());

  }else{
    NS_LOG_INFO("Restart the discovery process");

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
