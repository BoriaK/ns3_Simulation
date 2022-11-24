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
#include <numeric>

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
#include "tutorial-app.h"  
#include "custom_onoff-application.h" 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Traffic_Control_Example_Incast_Topology_v01_1");

// static void
// CwndChange (uint32_t oldCwnd, uint32_t newCwnd)
// {
//   NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << newCwnd);
// }

void
TcPacketsInQueueTrace (uint32_t oldValue, uint32_t newValue)
{
  // std::cout << "TcPacketsInQueue " << oldValue << " to " << newValue << std::endl;
  std::cout << "TcPacketsInQueue " << newValue << std::endl;
}

// added by me///////
void
TcHighPriorityPacketsInQueueTrace (uint32_t oldValue, uint32_t newValue)
{
  // std::cout << "TcPacketsInQueue " << oldValue << " to " << newValue << std::endl;
  std::cout << "TcHighPriorityPacketsInQueue " << newValue << std::endl;
}

void
TcLowPriorityPacketsInQueueTrace (uint32_t oldValue, uint32_t newValue)
{
  // std::cout << "TcPacketsInQueue " << oldValue << " to " << newValue << std::endl;
  std::cout << "TcLowPriorityPacketsInQueue " << newValue << std::endl;
}

void
QueueThresholdHighTrace (uint32_t oldValue, uint32_t newValue)  // added by me, to monitor Threshold
{
  std::cout << "HighPriorityQueueThreshold " << newValue << " packets " << std::endl;
  // std::cout << "QueueThreshold " << newValue << std::endl;
}

void
QueueThresholdLowTrace (uint32_t oldValue, uint32_t newValue)  // added by me, to monitor Threshold
{
  std::cout << "LowPriorityQueueThreshold " << newValue << " packets " << std::endl;
  // std::cout << "QueueThreshold " << newValue << std::endl;
}
///end of code///
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
  // Set up some default values for the simulation.
  double simulationTime = 50; //seconds
  std::string applicationType = "customOnOff"; // "standardClient"/"OnOff"/"customApplication"/"customOnOff"
  std::string transportProt = "Udp";
  std::string socketType;
  std::string queue_capacity;

  CommandLine cmd (__FILE__);
  cmd.AddValue("Simulation Time", "The total time for the simulation to run", simulationTime);
  cmd.AddValue ("applicationType", "Application type to use to send data: standardClient, customApplication, OnOff, customOnOff", applicationType);
  cmd.AddValue ("transportProt", "Transport protocol to use: Tcp, Udp", transportProt);
  cmd.Parse (argc, argv);
  
  // Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
  // Config::SetDefault ("ns3::UdpSocket::InitialCwnd", UintegerValue (1));
  // Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType", TypeIdValue (TypeId::LookupByName ("ns3::TcpClassicRecovery")));

  // Application type dependent parameters
  if (applicationType.compare("standardClient") == 0)
    {
      queue_capacity = "20p"; // B, the total space on the buffer
    }
  else if (applicationType.compare("OnOff") == 0 || applicationType.compare("customOnOff") == 0 || applicationType.compare("customApplication") == 0)
    {
      queue_capacity = "100p"; // B, the total space on the buffer [packets]
    }
  // client type dependant parameters:
  if (transportProt.compare ("Tcp") == 0)
    {
      socketType = "ns3::TcpSocketFactory";
    }
  else
    {
      socketType = "ns3::UdpSocketFactory";
    }
  
  // Application and Client type dependent parameters
  // select the desired components to output data
  if (applicationType.compare("standardClient") == 0 && transportProt.compare ("Tcp") == 0)
  {
    LogComponentEnable ("TcpClient", LOG_LEVEL_INFO);
  }
  else if (applicationType.compare("standardClient") == 0 && transportProt.compare ("Udp") == 0)
  {
    LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  }
  else if ((applicationType.compare("OnOff") == 0 || applicationType.compare("customOnOff") == 0 || applicationType.compare("customApplication") == 0)&& transportProt.compare ("Tcp") == 0)
  {
    LogComponentEnable("TcpSocketImpl", LOG_LEVEL_INFO);
  }
  else if ((applicationType.compare("OnOff") == 0 || applicationType.compare("customOnOff") == 0 || applicationType.compare("customApplication") == 0) && transportProt.compare ("Udp") == 0)
  {
    LogComponentEnable("UdpSocketImpl", LOG_LEVEL_INFO);
  }
  
  LogComponentEnable("PacketSink", LOG_LEVEL_INFO); 

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
  PointToPointHelper p2p1_l;  // the link between each sender to Router
  p2p1_l.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
  p2p1_l.SetChannelAttribute ("Delay", StringValue ("5ms"));

  PointToPointHelper p2p1_h;  // the link between each sender to Router
  p2p1_h.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
  p2p1_h.SetChannelAttribute ("Delay", StringValue ("5ms"));

  PointToPointHelper p2p2;  // the link between router and Reciever
  p2p2.SetDeviceAttribute  ("DataRate", StringValue ("1Mbps"));
  p2p2.SetChannelAttribute ("Delay", StringValue ("10ms"));
  // minimal value for NetDevice buffer is 1p. we set it in order to observe Traffic Controll effects only.
  p2p2.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1p"));


  // And then install devices and channels connecting our topology.
  NetDeviceContainer sender1 = p2p1_l.Install (clientNodes.Get(0), clientNodes.Get(1));
  NetDeviceContainer sender2 = p2p1_h.Install (clientNodes.Get(2), clientNodes.Get(1));
  // NetDeviceContainer sender3 = p2p1.Install (senders.Get(3), senders.Get(1));
  NetDeviceContainer reciever = p2p2.Install (serverNodes);


  TrafficControlHelper tch;
  // tch.SetRootQueueDisc ("ns3::RedQueueDisc", "MaxSize", StringValue ("10p"));
  // tch.SetRootQueueDisc ("ns3::PfifoFastQueueDisc", "MaxSize", StringValue ("10p"));
  tch.SetRootQueueDisc ("ns3::DT_FifoQueueDisc_v02", "MaxSize", StringValue ("100p"));
  
  QueueDiscContainer qdiscs = tch.Install (reciever);

  // Ptr<QueueDisc> q = qdiscs.Get (1); // original code - doesn't show values
  Ptr<QueueDisc> q = qdiscs.Get (0); // look at the router queue - shows actual values
  // The Next Line Displayes "PacketsInQueue" statistic at the Traffic Controll Layer
  // q->TraceConnectWithoutContext ("PacketsInQueue", MakeCallback (&TcPacketsInQueueTrace));
  q->TraceConnectWithoutContext ("HighPriorityPacketsInQueue", MakeCallback (&TcHighPriorityPacketsInQueueTrace));  // ### ADDED BY ME #####
  q->TraceConnectWithoutContext ("LowPriorityPacketsInQueue", MakeCallback (&TcLowPriorityPacketsInQueueTrace));  // ### ADDED BY ME #####
  q->TraceConnectWithoutContext("EnqueueingThreshold_High", MakeCallback (&QueueThresholdHighTrace)); // ### ADDED BY ME #####
  q->TraceConnectWithoutContext("EnqueueingThreshold_Low", MakeCallback (&QueueThresholdLowTrace)); // ### ADDED BY ME #####
  Config::ConnectWithoutContextFailSafe ("/NodeList/1/$ns3::TrafficControlLayer/RootQueueDiscList/0/SojournTime",
                                 MakeCallback (&SojournTimeTrace));

  // Ptr<NetDevice> nd = serverDevice.Get (1);  // original value
  Ptr<NetDevice> nd = reciever.Get (0);  //router side? fits queue-discs-benchmark example
  Ptr<PointToPointNetDevice> ptpnd = DynamicCast<PointToPointNetDevice> (nd);
  Ptr<Queue<Packet> > queue = ptpnd->GetQueue ();
  // The Next Line Displayes "PacketsInQueue" statistic at the NetDevice Layer
  // queue->TraceConnectWithoutContext ("PacketsInQueue", MakeCallback (&DevicePacketsInQueueTrace));


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
  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), servPort));
  // Create a packet sink to receive these packets on n2
  PacketSinkHelper sink (socketType, sinkLocalAddress);                       
  ApplicationContainer sinkApp = sink.Install (serverNodes.Get (1));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (simulationTime + 0.1));
  
  uint32_t longPayloadSize = 1024;
  uint32_t shortPayloadSize = 16;
  uint32_t payloadSize = 1024;
  uint32_t numOfPackets = 100;  // number of packets to send in one stream for custom application

  // Install application on the senders
  //get the address of the reciever node NOT THE ROUTER!!!
  if (applicationType.compare("standardClient") == 0)
  {
    UdpClientHelper udpClientShort (routerInterface.GetAddress(1), servPort);
    udpClientShort.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
    udpClientShort.SetAttribute ("PacketSize", UintegerValue (shortPayloadSize));
    UdpClientHelper udpClientLong (routerInterface.GetAddress(1), servPort);
    udpClientLong.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
    udpClientLong.SetAttribute ("PacketSize", UintegerValue (longPayloadSize));

    ApplicationContainer sourceApps1 = udpClientShort.Install (clientNodes.Get (0));
    sourceApps1.Start (Seconds (1.0));
    sourceApps1.Stop (Seconds(3.0));
    ApplicationContainer sourceApps2 = udpClientLong.Install (clientNodes.Get (2));
    sourceApps2.Start (Seconds (1.0));
    sourceApps2.Stop (Seconds(3.0));
    // ApplicationContainer sourceApps3 = udpClientLong.Install (clientNodes.Get (3));
    // sourceApps3.Start (Seconds (1.0));
    // sourceApps3.Stop (Seconds(3.0));
  }
  else if (applicationType.compare("OnOff") == 0)
  {
   // Create the OnOff applications to send TCP/UDP to the server
    InetSocketAddress socketAddressUp = InetSocketAddress (routerInterface.GetAddress(1), servPort);
    
    OnOffHelper clientShortHelper (socketType, Address ());
    OnOffHelper clientLongHelper (socketType, Address ());
    
    clientShortHelper.SetAttribute ("Remote", AddressValue (socketAddressUp));
    clientShortHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    clientShortHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    clientShortHelper.SetAttribute ("PacketSize", UintegerValue (shortPayloadSize));
    clientShortHelper.SetAttribute ("DataRate", StringValue ("1Mb/s"));

    clientLongHelper.SetAttribute ("Remote", AddressValue (socketAddressUp));
    clientLongHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    clientLongHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    clientLongHelper.SetAttribute ("PacketSize", UintegerValue (longPayloadSize));
    clientLongHelper.SetAttribute ("DataRate", StringValue ("1Mb/s"));
    
  
    ApplicationContainer sourceApps1 = clientShortHelper.Install (clientNodes.Get (0));
    sourceApps1.Start (Seconds (1.0));
    sourceApps1.Stop (Seconds(3.0));
    ApplicationContainer sourceApps2 = clientLongHelper.Install (clientNodes.Get (2));
    sourceApps2.Start (Seconds (1.0));
    sourceApps2.Stop (Seconds(3.0));
    // ApplicationContainer sourceApps3 = clientLongHelper.Install (clientNodes.Get (3));
    // sourceApps3.Start (Seconds (1.0));
    // sourceApps3.Stop (Seconds(3.0));
  }
  else if (applicationType.compare("customOnOff") == 0)
  {
    // Create the Custom application to send TCP/UDP to the server
    Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (clientNodes.Get (0), UdpSocketFactory::GetTypeId ());
    Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (clientNodes.Get (2), UdpSocketFactory::GetTypeId ());
    // ns3UdpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
    
    InetSocketAddress socketAddressUp = InetSocketAddress (routerInterface.GetAddress(1), servPort);
    
    Ptr<CustomOnOffApplication> customOnOffApp1 = CreateObject<CustomOnOffApplication> ();
    customOnOffApp1->Setup(ns3UdpSocket1);
    customOnOffApp1->SetAttribute("Remote", AddressValue (socketAddressUp));
    customOnOffApp1->SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"));
    customOnOffApp1->SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.1]"));
    customOnOffApp1->SetAttribute("PacketSize", UintegerValue (payloadSize));
    customOnOffApp1->SetAttribute("DataRate", StringValue ("2Mb/s"));
    customOnOffApp1->SetStartTime (Seconds (1.0));
    customOnOffApp1->SetStopTime (Seconds(3.0));
    clientNodes.Get (0)->AddApplication (customOnOffApp1);

    Ptr<CustomOnOffApplication> customOnOffApp2 = CreateObject<CustomOnOffApplication> ();
    customOnOffApp2->Setup(ns3UdpSocket2);
    customOnOffApp2->SetAttribute("Remote", AddressValue (socketAddressUp));
    customOnOffApp2->SetAttribute("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.2]"));
    customOnOffApp2->SetAttribute("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.1]"));
    customOnOffApp2->SetAttribute("PacketSize", UintegerValue (payloadSize));
    customOnOffApp2->SetAttribute("DataRate", StringValue ("2Mb/s"));
    customOnOffApp2->SetStartTime (Seconds (1.0));
    customOnOffApp2->SetStopTime (Seconds(3.0));
    clientNodes.Get (2)->AddApplication (customOnOffApp2);
  }
  else if (applicationType.compare("customApplication") == 0)
  {
    // Create the Custom application to send TCP/UDP to the server
    Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (clientNodes.Get (0), UdpSocketFactory::GetTypeId ());
    Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (clientNodes.Get (2), UdpSocketFactory::GetTypeId ());
    // ns3UdpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&CwndChange));
    
    InetSocketAddress socketAddressUp = InetSocketAddress (routerInterface.GetAddress(1), servPort);

    Ptr<TutorialApp> customApp1 = CreateObject<TutorialApp> ();
    customApp1->Setup (ns3UdpSocket1, socketAddressUp, payloadSize, numOfPackets, DataRate ("1Mbps"));
    clientNodes.Get (0)->AddApplication (customApp1);
    customApp1->SetStartTime (Seconds (1.0));
    customApp1->SetStopTime (Seconds(3.0));

    Ptr<TutorialApp> customApp2 = CreateObject<TutorialApp> ();
    customApp2->Setup (ns3UdpSocket2, socketAddressUp, payloadSize, numOfPackets, DataRate ("1Mbps"));
    clientNodes.Get (2)->AddApplication (customApp2);
    customApp2->SetStartTime (Seconds (1.0));
    customApp2->SetStopTime (Seconds(3.0));
  }

  NS_LOG_INFO ("Run Simulation.");
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.Install(allNodes);

  Simulator::Stop (Seconds (simulationTime + 10));
  Simulator::Run ();

  // monitor->SerializeToXmlFile("myTrafficControl_IncastTopology_v01_1.xml", true, true);

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  std::cout << std::endl << "*** Flow monitor statistics ***" << std::endl;
// a loop to sum the Tx/Rx Packets/Bytes from all nodes
  uint32_t txPackets = 0; 
  uint64_t txBytes = 0;
  uint32_t rxPackets = 0; 
  uint64_t rxBytes = 0;
  for (size_t i = 1; i <= stats.size(); i++)
  {
    txPackets = txPackets + stats[i].txPackets;
    txBytes = txBytes + stats[i].txBytes;
    rxPackets = rxPackets + stats[i].rxPackets;
    rxBytes = rxBytes + stats[i].rxBytes;
  }

  std::cout << "  Tx Packets/Bytes:   " << txPackets
            << " / " << txBytes << std::endl;
  // std::cout << "  Tx Packets/Bytes:   " << stats[1].txPackets
  //           << " / " << stats[1].txBytes << std::endl;
  // std::cout << "  Offered Load: " << stats[1].txBytes * 8.0 / (stats[1].timeLastTxPacket.GetSeconds () - stats[1].timeFirstTxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
  std::cout << "  Rx Packets/Bytes:   " << rxPackets
            << " / " << rxBytes << std::endl;
  // std::cout << "  Rx Packets/Bytes:   " << stats[1].rxPackets
  //           << " / " << stats[1].rxBytes << std::endl;

  uint32_t packetsDroppedByQueueDisc = 0;
  uint64_t bytesDroppedByQueueDisc = 0;
  if (stats[1].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE_DISC)
    {
      packetsDroppedByQueueDisc = stats[1].packetsDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
      bytesDroppedByQueueDisc = stats[1].bytesDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
    }
  // add packets/bytes count from the other node
  if (stats[2].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE_DISC)
    {
      packetsDroppedByQueueDisc = packetsDroppedByQueueDisc + stats[2].packetsDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
      bytesDroppedByQueueDisc = bytesDroppedByQueueDisc + stats[2].bytesDropped[Ipv4FlowProbe::DROP_QUEUE_DISC];
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
  // add packets/bytes count from the other node  
  if (stats[2].packetsDropped.size () > Ipv4FlowProbe::DROP_QUEUE)
    {
      packetsDroppedByNetDevice = packetsDroppedByNetDevice + stats[2].packetsDropped[Ipv4FlowProbe::DROP_QUEUE];
      bytesDroppedByNetDevice = bytesDroppedByNetDevice + stats[2].bytesDropped[Ipv4FlowProbe::DROP_QUEUE];
    }  
  std::cout << "  Packets/Bytes Dropped by NetDevice:   " << packetsDroppedByNetDevice
            << " / " << bytesDroppedByNetDevice << std::endl;
  // std::cout << "  Throughput: " << stats[1].rxBytes * 8.0 / (stats[1].timeLastRxPacket.GetSeconds () - stats[1].timeFirstRxPacket.GetSeconds ()) / 1000000 << " Mbps" << std::endl;
  // std::cout << "  Mean delay:   " << stats[1].delaySum.GetSeconds () / stats[1].rxPackets << std::endl;
  // std::cout << "  Mean jitter:   " << stats[1].jitterSum.GetSeconds () / (stats[1].rxPackets - 1) << std::endl;
  auto dscpVec = classifier->GetDscpCounts (1);
  for (auto p : dscpVec)
    {
      std::cout << "  DSCP value:   0x" << std::hex << static_cast<uint32_t> (p.first) << std::dec
                << "  count:   "<< p.second << std::endl;
    }

  // Simulator::Destroy ();

  std::cout << std::endl << "*** Application statistics ***" << std::endl;
  double thr = 0;
  uint64_t totalPacketsThr = DynamicCast<PacketSink> (sinkApp.Get (0))->GetTotalRx ();
  thr = totalPacketsThr * 8 / (simulationTime * 1000000.0); //Mbit/s
  std::cout << "  Rx Bytes: " << totalPacketsThr << std::endl;
  std::cout << "  Average Goodput: " << thr << " Mbit/s" << std::endl;
  std::cout << std::endl << "*** TC Layer statistics ***" << std::endl;
  std::cout << q->GetStats () << std::endl;
  return 0;
  Simulator::Destroy ();
}