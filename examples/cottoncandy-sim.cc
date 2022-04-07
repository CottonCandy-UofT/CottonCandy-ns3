/*
 * This script simulates a simple network in which one end device sends one
 * packet to the gateway.
 */

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/cottoncandy-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/cottoncandy-simple-app.h"
#include "ns3/cottoncandy-simple-app-helper.h"
#include "ns3/cottoncandy-mac-helper.h"
#include "ns3/cottoncandy-address-generator.h"
#include "ns3/command-line.h"
#include "ns3/core-module.h"
//#include "ns3/netanim-module.h"
#include <algorithm>
#include <ctime>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("MultiStage");

const std::string logDir = "cottoncandy_sim_logs";

int
main (int argc, char *argv[])
{

  // Set up logging
  LogComponentEnable ("MultiStage", LOG_LEVEL_ALL);
  //LogComponentEnable ("MobilityHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraChannel", LOG_LEVEL_INFO);
  //LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
  //LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraInterferenablenceHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("CottoncandyMac", LOG_DEBUG);
  //LogComponentEnable ("GatewayLorawanMac", LOG_LEVEL_ALL);
  //LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("LogicalLoraChannel", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraPhyHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("SimpleEndDeviceLoraPhy", LOG_LEVEL_DEBUG);
  //LogComponentEnable ("CottoncandySimpleAppHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraPacketTracker", LOG_LEVEL_DEBUG);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  double radius = 20000;
  int numNodes = 100;
  std::string positionAlloc = "grid";
  int simulationTime = 255;
  uint8_t replyLen = 2;
  
  double gridDelta = 2000;
  double gridWidth = 10;

  double gatewayX = 0;
  double gatewayY = 0;

  int simMode = CottonCandySimulationMode::FULL_SIMULATION;
  int numChannels = 64;

  std::string outputFileName = "topology.txt";

  /**
   * Use commandline arguments to specify network paramters
   */ 
  CommandLine cmd;
 
  cmd.AddValue("radius", "Radius of the network in meters (for uniform disk allocation)",
              radius);
  cmd.AddValue("positionModel", "grid or disk", positionAlloc); 
  cmd.AddValue("numNodes", "Number of nodes (exclude the gateway)", numNodes);
  cmd.AddValue("simulationTime", "Simulation time in hours", simulationTime);
  cmd.AddValue("replyLen", "Length of the NodeReply payload", replyLen);

  cmd.AddValue("gridDelta", "Vertical/Horizontal distance between two adjacent nodes in a grid",
              gridDelta);
  cmd.AddValue("fileName", "File name for the output data", outputFileName);
  cmd.AddValue("numChannels", "Number of channels. Default is 64", numChannels);
  cmd.AddValue("mode", "Full Simulation - 0, Test Static TX discovery only - 1, Test Proximity-Based Discovery Only - 2. Default is 0", simMode);

  cmd.Parse(argc, argv);

  /************************
  *  Create the channel  *
  ************************/

  NS_LOG_INFO ("Creating the channel...");

  // Create the lora channel object
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  
  //TODO: Those parameters should be changed based on the empirical measurement
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO ("Setting up helpers...");

  MobilityHelper mobility;
  
  if (positionAlloc == "disk")
  {
    NS_LOG_INFO("Use disk allocation with radius=" << radius);
    // Use random unifrom disk distribution
    mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator", 
                                 "rho", DoubleValue (radius),
                                 "X", DoubleValue (gatewayX), 
                                 "Y", DoubleValue (gatewayY));
  }
  else if (positionAlloc == "grid")
  {
  NS_LOG_INFO("Use default grid allocation with radius=" << radius);

  double minX = - gridWidth / 2 * gridDelta + gridDelta/2;
  double minY = minX;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (minX),
                                  "MinY", DoubleValue (minY),
                                  "DeltaX", DoubleValue (gridDelta),
                                  "DeltaY", DoubleValue (gridDelta),
                                  "GridWidth", UintegerValue (gridWidth),
                                  "LayoutType", StringValue ("RowFirst"));
  }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LorawanMacHelper
  CottoncandyMacHelper macHelper = CottoncandyMacHelper ();

  Ptr<CottoncandyAddressGenerator> generator = CreateObject<CottoncandyAddressGenerator> ();

  macHelper.SetAddressGenerator (generator);

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();
  helper.EnablePacketTracking (); // Output filename
  
  LoraPacketTracker &tracker = helper.GetPacketTracker();
  tracker.CottoncandySetNumNodes(numNodes + 1); // +1 for the gateway

  /************************
  *  Create End Devices  *
  ************************/

  NS_LOG_INFO ("Creating the end device...");

  // Create a set of nodes
  NodeContainer endDevices;
  endDevices.Create (numNodes);

  // Assign a mobility model to the node
  mobility.Install (endDevices);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (CottoncandyMacHelper::ED_A);
  helper.Install (phyHelper, macHelper, endDevices);

  /*********************
  *  Create Gateways  *
  *********************/

  NS_LOG_INFO ("Creating the gateway...");
  NodeContainer gateways;
  gateways.Create (1);

  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();
  // Make it so that nodes are at a certain height > 0
  allocator->Add (Vector (gatewayX, gatewayY, 1.0));
  mobility.SetPositionAllocator (allocator);
  mobility.Install (gateways);

  // Create a netdevice for each gateway
  // In our case, the gateway physical layer is similar to an end device
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (CottoncandyMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  /*********************************************
  *  Install applications on the end devices  *
  *********************************************/
  CottoncandySimpleAppHelper appHelper = CottoncandySimpleAppHelper ();

  //For end nodes, they should start at a random back-off
  appHelper.SetStartTime (0);
  appHelper.SetReplyLen(replyLen);
  appHelper.SetSimulationMode(simMode);
  appHelper.SetNumChannels(numChannels);
  appHelper.Install (endDevices);

  //The gateway should be up at the beginning at the simulation
  appHelper.SetStartTime (0);
  appHelper.Install (gateways);
  
  /****************
  *  Simulation  *
  ****************/

  Simulator::Stop (Hours (simulationTime));

  //AnimationInterface anim ("multistage.xml");
  //anim.SetMobilityPollInterval (Seconds (1));
  //anim.UpdateNodeSize (6, 1.5, 1.5);

  Simulator::Run ();

  Simulator::Destroy ();

  /*
  std::time_t t = std::time(0);
  std::tm* now = std::localtime(&t);
  
  
  std::stringstream ss;
  ss << (now->tm_year + 1900) << '_' << (now->tm_mon + 1) << '_' << (now->tm_mday) << '_' 
     << (now->tm_hour) << (now->tm_min) << (now->tm_sec) << "_n" << numNodes;
  std::string filename = ss.str();
  */

  std::ofstream outputFile;
  outputFile.open (outputFileName, std::ofstream::out | std::ofstream::trunc);
  outputFile << tracker.PrintCottoncandyEdges() << std::endl;
  outputFile.close();

  //filename = "channelHistory.txt";

  //std::ofstream channelHistory;
  //channelHistory.open (filename, std::ofstream::out | std::ofstream::trunc);
  //channelHistory << tracker.PrintCottoncandyChannelStats() << std::endl;
  //channelHistory.close();

  std::cout << "Join Completion at " << tracker.CottoncandyGetJoinCompletionTime() << " seconds" << std::endl;
  std::cout << tracker.GetCollisionStats() << std::endl;

  return 0;
}
