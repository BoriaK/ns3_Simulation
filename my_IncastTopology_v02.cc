/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

// This was an attempt to build a Tree Topology by connecting the hub of a star topology
// to an additional node as a reciever.
//
// Network topology (default)
//       
//       n0                       .
//         \                      .
//          \                     .
//      n1---R------------S       .          
//       .  /                     .
//       . /                      .
//       nN                       .
//       
//
//  Usage (e.g.): ./ns3 run scratch/my_lineTopology_v02

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Line Toplogy v02");

int main (int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //  LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
  //  LogComponentEnable("TcpSocketImpl", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
  //  LogComponentEnable("TcpLargeTransfer", LOG_LEVEL_ALL);

  // Here, we will explicitly create all the nodes.  The first container contains
  // the star diagram above, and the second one contains nodes:
  // star hub and the reciever.  This reflects the channel connectivity, and will be used to
  // install the network interfaces and connect them with a channel.

  // Default number of nodes in the star.  Overridable by command line argument.
  //
  uint32_t N = 3; //number of sender nodes in the tree

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nSpokes", "Number of nodes to place in the tree", N);
  cmd.Parse (argc, argv);

  // build star, all spokes are sender nodes. the hub is a router node.
  // create the link device that will connect all the senders to the router
  // connect spokes to the hub and install the desired device (channel).
  
  NS_LOG_INFO ("Build star topology.");
  // Here, we will create N nodes in a star.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer serverNodes;
  NodeContainer clientNodes;
  serverNodes.Create (2);
  clientNodes.Create (N);
  NodeContainer allNodes = NodeContainer (serverNodes, clientNodes);

  // Install network stacks on the nodes
  InternetStackHelper internet;
  internet.Install (allNodes);

  //Collect an adjacency list of nodes for the p2p topology
  std::vector<NodeContainer> nodeAdjacencyList (N);
  for(uint32_t i=0; i<nodeAdjacencyList.size (); ++i)
    {
      nodeAdjacencyList[i] = NodeContainer (serverNodes.Get(0), clientNodes.Get (i));
    }

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p1; // the link between senders and router
  p2p1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p1.SetChannelAttribute ("Delay", StringValue ("2ms"));
  std::vector<NetDeviceContainer> deviceAdjacencyList (N);
  for(uint32_t i=0; i<deviceAdjacencyList.size (); ++i)
    {
      deviceAdjacencyList[i] = p2p1.Install (nodeAdjacencyList[i]);
    }
  
  PointToPointHelper p2p2;  // the link between router and Reciever
  p2p2.SetDeviceAttribute  ("DataRate", StringValue ("5Kbps"));
  p2p2.SetChannelAttribute ("Delay", StringValue ("10ms"));
  // min value for NetDevice buffer is 1p. we set it in order to observe Traffic Controll effects only.
  p2p2.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1p"));

  // then install devices and channels connecting our router to reciever.
  NetDeviceContainer serverDevice = p2p2.Install (serverNodes);

  // Assign IP addresses
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  // ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  std::vector<Ipv4InterfaceContainer> interfaceAdjacencyList (N);
  for(uint32_t i=0; i<interfaceAdjacencyList.size (); ++i)
    {
      std::ostringstream subnet;
      subnet<<"10.1."<<i+1<<".0";
      ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
      interfaceAdjacencyList[i] = ipv4.Assign (deviceAdjacencyList[i]);
    }
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer routerInterface = ipv4.Assign (serverDevice);

  // and setup ip routing tables to get total ip-level connectivity.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Create applications.");

  uint16_t servPort = 50000;

  // Create a packet sink to collect packets at the Reciever
  uint16_t port = 50000;
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (serverNodes.Get(1));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (40.0));

  // Install application on the senders
  //get the address of the reciever node NOT THE ROUTER!!!
  // // Create the OnOff applications to send TCP to the server
  // OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
  // clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  // clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  
  // FOR DEBUG: TRY TO SEND udp PACKETS FROM CLIENT TO SERVER WITHOUT OnOffApplication
  UdpClientHelper clientHelper (routerInterface.GetAddress(1), servPort);
  clientHelper.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  clientHelper.SetAttribute ("PacketSize", UintegerValue (1024));
  //normally wouldn't need a loop here but the server IP address is different
  //on each p2p subnet
  ApplicationContainer clientApps;
  for(uint32_t i=0; i<clientNodes.GetN (); ++i)
    {
      clientApps.Add (clientHelper.Install (clientNodes.Get (i)));
    }
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (2.0));

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}