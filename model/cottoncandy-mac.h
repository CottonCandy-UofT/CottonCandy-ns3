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

#ifndef COTTONCANDY_MAC_H
#define COTTONCANDY_MAC_H

#include "ns3/object.h"
#include "ns3/vector.h"
#include "ns3/logical-lora-channel-helper.h"
#include "ns3/packet.h"
#include "ns3/lora-phy.h"
#include "end-device-lora-phy.h"
#include "ns3/cottoncandy-address.h"
#include "ns3/cottoncandy-mac-header.h"
#include "ns3/cottoncandy-joinack-header.h"
#include "ns3/cottoncandy-gatewayreq-header.h"
#include "ns3/cottoncandy-seekjoin-header.h"
#include "ns3/cottoncandy-nodereply-header.h"
#include "ns3/cottoncandy-nodereply-embedded-header.h"
#include "ns3/lora-tag.h"

#include "ns3/cottoncandy-channel-selector.h"
#include <array>
#include <algorithm>
#include <math.h>

namespace ns3 {
namespace lorawan {

enum CottonCandySimulationMode{
  FULL_SIMULATION,
  TEST_STATIC_TX_DISCOVERY_ONLY,
  TEST_PROXIMITY_BASED_DISCOVERY_ONLY,
  TEST_MULTI_CHANNEL_WITH_PROXIMITY_BASED_DISCOVERY,
  TEST_RANDOM_CHANNEL_WITH_PROXIMITY_BASED_DISCOVERY
};

enum CottonCandyChannelAlgorithm{
  RANDOM_CHANNEL,
  CHANNEL_ANNOUNCEMENT
};

enum CottonCandyDiscoveryMode{
  STATIC_TX_PWR,
  ADAPTIVE_TX_PWR
};

enum CottonCandyState{
  DISCONNECTED,
  CONNECTED,
  OBSERVE,
  SEEK_JOIN_WINDOW,
  LISTEN_TO_PARENT,
  TALK_TO_CHILDREN,
  HIBERNATE,
  ACCEPT_JOIN
};

/**************** Channel Assignment ******************/
static const uint8_t PUBLIC_CHANNEL = 64;
static const double CHANNEL_SIZE = 0.2;
static const double CHANNEL_START_FREQ = 902.0;
static const uint8_t DEFAULT_NUM_CHANNELS = 64;
static const uint8_t INVALID_CHANNEL = 255;

/**************** Backoff parameters **************************/
static const double MIN_BACKOFF = 0.1;
static const uint8_t BACKOFF_MULTIPLIER = 3;
static const double MAX_BACKOFF_JOIN = 1;

static const double REQUIRED_COLLISION_RATE = 0.05;
static const double MAX_AIR_TIME = 0.118;

/**************** Parameters used for joining **************************/
static const double RSSI_THRESHOLD = -115;
static const int MAX_NUM_CHILDREN = 3;
static const int MAX_NUM_CANDIDATE_PARENT = 3;
static const uint8_t MAX_NUM_HOPS = 10;

/**************** Timeout durations **************************/
static const Time SEEK_JOIN_DURATION = Seconds(120);
static const Time ACCEPT_JOIN_DURATION = Seconds(6);
static const Time SHORT_HIBERNATION_DURATION = Seconds(10);

static const uint8_t MAX_EMPTY_ROUNDS = 5;
//Set this to very long since node failure will not happen in simulations
static const Time DCP_TIMEOUT = Seconds(900);

static const Time JOIN_ACK_TIMEOUT = Seconds(1);

/***************** Transmission power ************************/
static const uint8_t MIN_TX_POWER = 8;
static const uint8_t MAX_TX_POWER = 17;
static const uint8_t TX_POWER_INCREMENT = 1;

typedef struct{
  CottoncandyAddress parentAddr;
  uint8_t hops;
  uint8_t numChildren;
  int linkQuality;
  uint8_t ulChannel;
} ParentNode;

typedef struct{
  bool replyReceived;
  uint8_t missingDutyCycles;
} ChildStatus;

class LoraPhy;

static const CottoncandyAddress BROADCAST_ADDR = CottoncandyAddress(0xff);

/**
 * Class representing the LoRaWAN MAC layer.
 *
 * This class is meant to be extended differently based on whether the layer
 * belongs to an End Device or a Gateway, while holding some functionality that
 * is common to both.
 */
class CottoncandyMac : public Object
{
public:
  static TypeId GetTypeId (void);

  static double GetChannelFreq(uint8_t channel);

  CottoncandyMac ();
  virtual ~CottoncandyMac ();

  typedef std::array<std::array<uint8_t, 6>, 8> ReplyDataRateMatrix;

  /**
   * Set the underlying PHY layer
   *
   * \param phy the phy layer
   */
  void SetPhy (Ptr<LoraPhy> phy);

  /**
   * Get the underlying PHY layer
   *
   * \return The PHY layer that this MAC is connected to.
   */
  Ptr<LoraPhy> GetPhy (void);

  /**
   * Send a packet.
   *
   * \param packet The packet to send.
   */
  virtual void Send (Ptr<Packet> packet, double freq, double txPower);

  /**
   * Receive a packet from the lower layer.
   *
   * \param packet the received packet
   */
  virtual void Receive (Ptr<Packet const> packet);

  /**
   * Get the logical lora channel helper associated with this MAC.
   *
   * \return The instance of LogicalLoraChannelHelper that this MAC is using.
   */
  LogicalLoraChannelHelper GetLogicalLoraChannelHelper (void);

  /**
   * Set the LogicalLoraChannelHelper this MAC instance will use.
   *
   * \param helper The instance of the helper to use.
   */
  void SetLogicalLoraChannelHelper (LogicalLoraChannelHelper helper);

  /**
   * Get the SF corresponding to a data rate, based on this MAC's region.
   *
   * \param dataRate The Data Rate we need to convert to a Spreading Factor
   * value.
   * \return The SF that corresponds to a Data Rate in this MAC's region, or 0
   * if the dataRate is not valid.
   */
  uint8_t GetSfFromDataRate (uint8_t dataRate);

  /**
   * Get the BW corresponding to a data rate, based on this MAC's region
   *
   * \param dataRate The Data Rate we need to convert to a bandwidth value.
   * \return The bandwidth that corresponds to the parameter Data Rate in this
   * MAC's region, or 0 if the dataRate is not valid.
   */
  double GetBandwidthFromDataRate (uint8_t dataRate);

  /**
   * Get the transmission power in dBm that corresponds, in this region, to the
   * encoded 8-bit txPower.
   *
   * \param txPower The 8-bit encoded txPower to convert.
   *
   * \return The corresponding transmission power in dBm, or 0 if the encoded
   * power was not recognized as valid.
   */
  double GetDbmForTxPower (uint8_t txPower);

  /**
   * Set the vector to use to check up correspondence between SF and DataRate.
   *
   * \param sfForDataRate A vector that contains at position i the SF that
   * should correspond to DR i.
   */
  void SetSfForDataRate (std::vector<uint8_t> sfForDataRate);

  /**
   * Set the vector to use to check up correspondence between bandwidth and
   * DataRate.
   *
   * \param bandwidthForDataRate A vector that contains at position i the
   * bandwidth that should correspond to DR i in this MAC's region.
   */
  void SetBandwidthForDataRate (std::vector<double> bandwidthForDataRate);

  /**
   * Set the maximum App layer payload for a set DataRate.
   *
   * \param maxAppPayloadForDataRate A vector that contains at position i the
   * maximum Application layer payload that should correspond to DR i in this
   * MAC's region.
   */
  void SetMaxAppPayloadForDataRate (std::vector<uint32_t>
                                    maxAppPayloadForDataRate);

  /**
   * Set the vector to use to check up which transmission power in Dbm
   * corresponds to a certain TxPower value in this MAC's region.
   *
   * \param txDbmForTxPower A vector that contains at position i the
   * transmission power in dBm that should correspond to a TXPOWER value of i in
   * this MAC's region.
   */
  void SetTxDbmForTxPower (std::vector<double> txDbmForTxPower);

  /**
   * Set the matrix to use when deciding with which DataRate to respond. Region
   * based.
   *
   * \param replyDataRateMatrix A matrix containing the reply DataRates, based
   * on the sending DataRate and on the value of the RX1DROffset parameter.
   */
  void SetReplyDataRateMatrix (ReplyDataRateMatrix replyDataRateMatrix);

  /**
   * Set the number of PHY preamble symbols this MAC is set to use.
   *
   * \param nPreambleSymbols The number of preamble symbols to use (typically 8).
   */
  void SetNPreambleSymbols (int nPreambleSymbols);

  /**
   * Get the number of PHY preamble symbols this MAC is set to use.
   *
   * \return The number of preamble symbols to use (typically 8).
   */
  int GetNPreambleSymbols (void);

  void Run();

  void DiscoveryTimeout(void);

    /**
   * Set the device this MAC layer is installed on.
   *
   * \param device The NetDevice this MAC layer will refer to.
   */
  void SetDevice (Ptr<NetDevice> device);

  //void Stop();

  void SetDeviceAddress(CottoncandyAddress addr);

  void SetChannel(uint8_t channel);

  void SetReplyLen(uint8_t len);

  void SetSimulationMode(int mode);

  void SetNumChannels(int numChannels);

  void DoSend(Ptr<Packet> packet,double freq, double txPower);

  

  void ReportHalfDuplex(Ptr<Packet const> packet);

  void FailedReception(Ptr<Packet const> packet);

  void EnterObservePhase();
  void EndObservePhase();

  void EnterJoinPhase();
  void EndJoinPhase();

  void EnterAcceptJoinPhase();
  
  void EnterSeekJoinPhase();
  void EndSeekJoinPhase();

  void EnterDataCollectionPhase();
  void EndDataCollectionPhase();

  void TalkToChildren();
  void ReceiveTimeout();

  void Join();

protected:
  /**
  * The trace source that is fired when a connection is established with a prent node
  *
  * \see class CallBackTraceSource
  */
  TracedCallback<uint16_t, uint16_t, ns3::Vector, uint8_t> m_connectionEstablished;

  TracedCallback<uint16_t> m_gatewayReqReceived;

  TracedCallback<uint16_t> m_replyDelivered;

  TracedCallback<uint16_t> m_halfDuplexDetected;

  TracedCallback<uint8_t> m_collisionDetected;

  TracedCallback<uint16_t, uint8_t> m_channelSelected;

  TracedCallback<uint16_t, uint8_t> m_numInterferersDetected;

  /**
   * Trace source that is fired when a packet reaches the MAC layer.
   */
  TracedCallback<Ptr<const Packet> > m_receivedPacket;

  /**
   * Trace source that is fired when a new APP layer packet arrives at the MAC
   * layer.
   */
  TracedCallback<Ptr<Packet const> > m_sentNewPacket;

  /**
   * The PHY instance that sits under this MAC layer.
   */
  Ptr<LoraPhy> m_phy;

    /**
   * The device this MAC layer is installed on.
   */
  Ptr<NetDevice> m_device;

  /**
   * The LogicalLoraChannelHelper instance that is assigned to this MAC.
   */
  LogicalLoraChannelHelper m_channelHelper;

  /**
   * A vector holding the SF each Data Rate corresponds to.
   */
  std::vector<uint8_t> m_sfForDataRate;

  /**
   * A vector holding the bandwidth each Data Rate corresponds to.
   */
  std::vector<double> m_bandwidthForDataRate;

  /**
   * A vector holding the maximum app payload size that corresponds to a
   * certain DataRate.
   */
  std::vector<uint32_t> m_maxAppPayloadForDataRate;

  /**
   * The number of symbols to use in the PHY preamble.
   */
  int m_nPreambleSymbols;

  /**
   * A vector holding the power that corresponds to a certain TxPower value.
   */
  std::vector<double> m_txDbmForTxPower;

  /**
   * The matrix that decides the DR the GW will use in a reply based on the ED's
   * sending DR and on the value of the RX1DROffset parameter.
   */
  ReplyDataRateMatrix m_replyDataRateMatrix;

  int m_state = DISCONNECTED;
  /**
   * @brief Basic topology information
   * 
   */
  CottoncandyAddress m_address;   
  ParentNode m_parent;
  uint8_t m_numChildren;

  std::map<CottoncandyAddress, Time> pendingChildren = std::map<CottoncandyAddress, Time>();
  std::map<CottoncandyAddress, ChildStatus> childList = std::map<CottoncandyAddress, ChildStatus>();

  uint8_t m_numOutgoingJoinAck = 0;

  ParentNode bestParentCandidate;

  uint8_t m_replyLen = 0;

  /**
   * An uniform random variable, used by the Shuffle method to randomly reorder
   * the channel list.
   */
  Ptr<UniformRandomVariable> m_uniformRV;

  LoraTxParameters m_params;

  uint32_t m_dutyCycleInterval = 3600;

  double m_maxBackoff = BACKOFF_MULTIPLIER;

  uint8_t m_channel;


  double m_txPower = MIN_TX_POWER; 

  //Absolute time when the next Accept_Join phase starts
  Time m_nextAcceptJoin;

  std::list<Ptr<Packet>> m_PendingData = std::list<Ptr<Packet>>();

  EventId m_endDCPId;
  EventId m_endObserveId;
  EventId m_endDiscoveryId;

  Time m_endDCPTime;

  uint8_t m_emptyRounds = 0;

  std::vector<int> m_interfererChannels;
  
  std::map<CottoncandyAddress, uint8_t> m_candidateParents = std::map<CottoncandyAddress, uint8_t>();

  ParentNode m_currentCandidate;

  bool m_nextDutyCycleKnown = false;
  bool m_channelConflict = true;
  bool m_repliesFromChildren = false;

  int m_simMode = CottonCandySimulationMode::FULL_SIMULATION;
  int m_channelAlg = CottonCandyChannelAlgorithm::CHANNEL_ANNOUNCEMENT;
  int m_discoveryMode =  CottonCandyDiscoveryMode::ADAPTIVE_TX_PWR;

  int m_numChannels = DEFAULT_NUM_CHANNELS;
  
};

} /* namespace ns3 */

}
#endif /* LORAWAN_MAC_H */
