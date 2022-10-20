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
//  Usage (e.g.): ./ns3 run scratch/my_lineTopology_v01

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
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Traffic_Control_Example_Incast_Topology_v01");

void
TcPacketsInQueueTrace (uint32_t oldValue, uint32_t newValue)
{
  std::cout << "TcPacketsInQueue " << oldValue << " to " << newValue << std::endl;
}

void
DevicePacketsInQueueTrace (uint32_t oldValue, uint32_t newValue)
{
  std::cout << "DevicePacketsInQueue " << oldValue << " to " << newValue << std::endl;
}

void
SojournTimeTrace (Time sojournTime)
{
  std::cout << "Sojourn time " << sojournTime.ToDouble (Time::MS) << "ms" << std::endl;
}

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

  // Install network stacks on the nodes
  InternetStackHelper internet;
  internet.Install (allNodes);

  // We create the channels first without any IP addressing information
  // First make and configure the helper, so that it will put the appropriate
  // attributes on the network interfaces and channels we are about to install.
  
  // Create the point-to-point link helpers
  PointToPointHelper p2p1;  // the link between each sender to Router
  p2p1.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
  p2p1.SetChannelAttribute ("Delay", StringValue ("5ms"));

  PointToPointHelper p2p2;  // the link between router and Reciever
  p2p2.SetDeviceAttribute  ("DataRate", StringValue ("10Kbps"));
  p2p2.SetChannelAttribute ("Delay", StringValue ("10ms"));
  // min value for NetDevice buffer is 1p. we set it in order to observe Traffic Controll effects only.
  p2p2.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1p"));


  // And then install devices and channels connecting our topology.
  NetDeviceContainer sender1 = p2p1.Install (clientNodes.Get(0), clientNodes.Get(1));
  NetDeviceContainer sender2 = p2p1.Install (clientNodes.Get(2), clientNodes.Get(1));
  // NetDeviceContainer sender3 = p2p1.Install (senders.Get(3), senders.Get(1));
  NetDeviceContainer reciever = p2p2.Install (serverNodes);


  TrafficControlHelper tch;
  tch.SetRootQueueDisc ("ns3::RedQueueDisc", "MaxSize", StringValue ("10p"));
  QueueDiscContainer qdiscs = tch.Install (reciever);

  // Ptr<QueueDisc> q = qdiscs.Get (1); // original code - doesn't show values
  Ptr<QueueDisc> q = qdiscs.Get (0); // look at the router queue - shows actual values
  // The Next Line Displayes "PacketsInQueue" statistic at the Traffic Controll Layer
  // q->TraceConnectWithoutContext ("PacketsInQueue", MakeCallback (&TcPacketsInQueueTrace));
  Config::ConnectWithoutContextFailSafe ("/NodeList/1/$ns3::TrafficControlLayer/RootQueueDiscList/0/SojournTime",
                                 MakeCallback (&SojournTimeTrace));

  // Ptr<NetDevice> nd = serverDevice.Get (1);  // original value
  Ptr<NetDevice> nd = reciever.Get (0);  //router side? fits queue-discs-benchmark example
  Ptr<PointToPointNetDevice> ptpnd = DynamicCast<PointToPointNetDevice> (nd);
  Ptr<Queue<Packet> > queue = ptpnd->GetQueue ();
  // The Next Line Displayes "PacketsInQueue" statistic at the NetDevice Layer
  // queue->TraceConnectWithoutContext ("PacketsInQueue", MakeCallback (&DevicePacketsInQueueTrace));


  // // Later, we add IP addresses.
  // Ipv4AddressHelper ipv4;
  // ipv4.SetBase ("10.0.1.0", "255.255.255.0");
  // ipv4.Assign (sender1);
  // ipv4.SetBase ("10.0.2.0", "255.255.255.0");
  // ipv4.Assign (sender2);
  // ipv4.SetBase ("10.0.3.0", "255.255.255.0");
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

  ApplicationContainer sinkApp = sink.Install (serverNodes.Get (1));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (simulationTime));

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

  NS_LOG_INFO ("Run Simulation.");
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.Install(allNodes);

  Simulator::Stop (Seconds (simulationTime + 10));
  Simulator::Run ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  std::cout << std::endl << "*** Flow monitor statistics ***" << std::endl;
  std::cout << "  Tx Packets/Bytes:   " << stats[1].txPackets
            << " / " << stats[1].txBytes << std::endl;
  std::cout << "  Offered Load: " << stats[1].txBytes * 8.0 / (stats[1].timeLastTxPacket.GetSeconds () - stats[1].timeFirstTxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
  std::cout << "  Rx Packets/Bytes:   " << stats[1].rxPackets
            << " / " << stats[1].rxBytes << std::endl;
  uint32_t packetsDroppedByQueueDisc = 0;
  uint64_t bytesDroppedByQueueDisc = 0;
  if (stats[1].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE_DISC)
    {
      packetsDroppedByQueueDisc = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
      bytesDroppedByQueueDisc = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
    }
  std::cout << "  Packets/Bytes Dropped by Queue Disc:   " << packetsDroppedByQueueDisc
            << " / " << bytesDroppedByQueueDisc << std::endl;
  uint32_t packetsDroppedByNetDevice = 0;
  uint64_t bytesDroppedByNetDevice = 0;
  if (stats[1].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE)
    {
      packetsDroppedByNetDevice = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE];
      bytesDroppedByNetDevice = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE];
    }
  std::cout << "  Packets/Bytes Dropped by NetDevice:   " << packetsDroppedByNetDevice
            << " / " << bytesDroppedByNetDevice << std::endl;
  std::cout << "  Throughput: " << stats[1].rxBytes * 8.0 / (stats[1].timeLastRxPacket.GetSeconds () - stats[1].timeFirstRxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
  std::cout << "  Mean delay:   " << stats[1].delaySum.GetSeconds () / stats[1].rxPackets << std::endl;
  std::cout << "  Mean jitter:   " << stats[1].jitterSum.GetSeconds () / (stats[1].rxPackets - 1) << std::endl;
  auto dscpVec = classifier->GetDscpCounts (1);
  for (auto p : dscpVec)
    {
      std::cout << "  DSCP value:   0x" << std::hex << static_cast<uint32_t> (p.first) << std::dec
                << "  count:   "<< p.second << std::endl;
    }

  Simulator::Destroy ();

  std::cout << std::endl << "*** Application statistics ***" << std::endl;
  double thr = 0;
  uint64_t totalPacketsThr = DynamicCast<PacketSink> (sinkApp.Get (0))->GetTotalRx ();
  thr = totalPacketsThr * 8 / (simulationTime * 1000000.0); //Mbit/s
  std::cout << "  Rx Bytes: " << totalPacketsThr << std::endl;
  std::cout << "  Average Goodput: " << thr << " Mbit/s" << std::endl;
  std::cout << std::endl << "*** TC Layer statistics ***" << std::endl;
  std::cout << q->GetStats () << std::endl;
  return 0;
}