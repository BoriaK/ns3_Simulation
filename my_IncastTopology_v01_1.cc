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

//
// Network topology
//
//                  
//               n0                                .
//               .\                                .
//               . \ 10Mb/s, 10ms                  .
//               .  \    10Kb/s, 10ms              .
//               .   R-----------------S           .
//               .  /                              .
//               . / 10Mb/s, 10ms                  . 
//               ./                                .
//               n1                                .
//
// - Tracing of queues and packet receptions to file 
//   "tcp-large-transfer.tr"
// - pcap traces also generated in the following files
//   "tcp-large-transfer-$n-$i.pcap" where n and i represent node and interface
// numbers respectively
//  Usage (e.g.): ./ns3 run scratch/my_IncastTopology_v01_1

// in this version we run IncastTopology_v01 with 2 sender nodes, 2 different packet lengthes sent:
// 1024 bit UDP packets from n0
// 16 bit UDP packets from n1

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Incast Toplogy v01 - 2 different packet sizes");

int main (int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
  //  LogComponentEnable("TcpL4Protocol", LOG_LEVEL_ALL);
  //  LogComponentEnable("TcpSocketImpl", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
  //  LogComponentEnable("TcpLargeTransfer", LOG_LEVEL_ALL);

  double simulationTime = 40; //seconds
  std::string transportProt = "Udp";
  std::string socketType;
  
  CommandLine cmd (__FILE__);
  cmd.AddValue("Simulation Time", "The total time for the simulation to run", simulationTime);
  cmd.AddValue ("transportProt", "Transport protocol to use: Tcp, Udp", transportProt);
  cmd.Parse (argc, argv);

  // Here, we will explicitly create three nodes.  The first container contains
  // nodes 0 and 1 from the diagram above, and the second one contains nodes
  // 1 and 2.  This reflects the channel connectivity, and will be used to
  // install the network interfaces and connect them with a channel.
  NodeContainer clientNodes;
  clientNodes.Create (3);

  NodeContainer serverNodes;
  serverNodes.Add (clientNodes.Get (1));
  serverNodes.Create (1);

  NodeContainer allNodes = NodeContainer (serverNodes, clientNodes.Get(0), clientNodes.Get(2));

  // We create the channels first without any IP addressing information
  // First make and configure the helper, so that it will put the appropriate
  // attributes on the network interfaces and channels we are about to install.
  
  // Create the point-to-point link helpers
  PointToPointHelper p2p1;  // the link between each sender to Router
  p2p1.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
  p2p1.SetChannelAttribute ("Delay", StringValue ("5ms"));

  PointToPointHelper p2p2;  // the link between router and Reciever
  p2p2.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
  p2p2.SetChannelAttribute ("Delay", StringValue ("10ms"));


  // And then install devices and channels connecting our topology.
  NetDeviceContainer sender1 = p2p1.Install (clientNodes.Get(0), clientNodes.Get(1));
  NetDeviceContainer sender2 = p2p1.Install (clientNodes.Get(2), clientNodes.Get(1));
  // NetDeviceContainer sender3 = p2p1.Install (senders.Get(3), senders.Get(1));
  NetDeviceContainer reciever = p2p2.Install (serverNodes);

  // Now add ip/tcp stack to all nodes.
  InternetStackHelper internet;
  internet.InstallAll ();

  // // Later, we add IP addresses.
  // Ipv4AddressHelper ipv4;
  // ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  // ipv4.Assign (sender1);
  // ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  // ipv4.Assign (sender2);
  // ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  // Ipv4InterfaceContainer ipInterfs = ipv4.Assign (reciever);

  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");
  ipv4.NewNetwork ();
  Ipv4InterfaceContainer sender1Interface = ipv4.Assign (sender1);
  ipv4.NewNetwork ();
  Ipv4InterfaceContainer sender2Interface = ipv4.Assign (sender2);
  // ipv4.NewNetwork ();
  // Ipv4InterfaceContainer sender3Interface = ipv4.Assign (sender3);
  ipv4.NewNetwork ();
  Ipv4InterfaceContainer routerInterface = ipv4.Assign (reciever);

  // and setup ip routing tables to get total ip-level connectivity.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  uint16_t servPort = 50000;

  // Create a packet sink to receive these packets on n2...
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), servPort));

  ApplicationContainer apps = sink.Install (serverNodes.Get (1));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (simulationTime));

  // Install application on the senders
  //get the address of the reciever node NOT THE ROUTER!!!
  
  UdpClientHelper udpClientLong (routerInterface.GetAddress(1), servPort);
  udpClientLong.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  udpClientLong.SetAttribute ("PacketSize", UintegerValue (1024));
  UdpClientHelper udpClientShort (routerInterface.GetAddress(1), servPort);
  udpClientShort.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  udpClientShort.SetAttribute ("PacketSize", UintegerValue (16));

  ApplicationContainer sourceApps1 = udpClientLong.Install (clientNodes.Get (0));
  sourceApps1.Start (Seconds (1.0));
  sourceApps1.Stop (Seconds(2.0));
  ApplicationContainer sourceApps2 = udpClientShort.Install (clientNodes.Get (2));
  sourceApps2.Start (Seconds (1.0));
  sourceApps2.Stop (Seconds(2.0));
  // ApplicationContainer sourceApps3 = udpClient.Install (clientNodes.Get (3));
  // sourceApps2.Start (Seconds (1.0));
  // sourceApps2.Stop (Seconds(2.0));

  //Ask for ASCII and pcap traces of network traffic
  // AsciiTraceHelper ascii;
  // p2p1.EnableAsciiAll (ascii.CreateFileStream ("my_lineTopology_v02_SenderLink.tr"));
  // p2p2.EnableAsciiAll (ascii.CreateFileStream ("my_lineTopology_v02_RecieverLink.tr"));
  
  // NS_LOG_INFO ("Enable pcap tracing.");
  // p2p1.EnablePcapAll ("my_lineTopology_v02_SenderLink");
  // p2p2.EnablePcapAll ("my_lineTopology_v02_RecieverLink");

  // Finally, set up the simulator to run.  The 1000 second hard limit is a
  // failsafe in case some change above causes the simulation to never end
  Simulator::Stop (Seconds (simulationTime + 10));
  Simulator::Run ();
  Simulator::Destroy ();
}