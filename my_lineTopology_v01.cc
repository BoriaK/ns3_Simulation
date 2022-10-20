/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
// was made from: tcp-bbr-example.cc
// This program simulates the following topology:
//
//           1000 Mbps           10Mbps         
//  Sender  -------------- R1 -------------- Receiver
//              5ms               10ms             
//
// The link between sender and R1 is 10 Mbps.
// The link between R1 and Reciever is a bottleneck link with 10 Kbps.

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Line Toplogy v01");

int main (int argc, char *argv [])
{
  CommandLine cmd (__FILE__);
  cmd.Parse (argc, argv);

  NodeContainer sender, receiver;
  NodeContainer router;
  sender.Create (1);
  receiver.Create (1);
  router.Create (1);

  // Create the point-to-point link helpers
  PointToPointHelper bottleneckLink;
  bottleneckLink.SetDeviceAttribute  ("DataRate", StringValue ("10Kbps"));
  bottleneckLink.SetChannelAttribute ("Delay", StringValue ("10ms"));

  PointToPointHelper edgeLink;
  edgeLink.SetDeviceAttribute  ("DataRate", StringValue ("10Mbps"));
  edgeLink.SetChannelAttribute ("Delay", StringValue ("5ms"));

  // Create NetDevice containers
  NetDeviceContainer senderEdge = edgeLink.Install (sender.Get (0), router.Get (0));
  //NetDeviceContainer r1 = bottleneckLink.Install (router.Get (0));
  NetDeviceContainer receiverEdge = bottleneckLink.Install (router.Get (0), receiver.Get (0));

  // Install Stack
  InternetStackHelper internet;
  internet.Install (sender);
  internet.Install (receiver);
  internet.Install (router);

  // Assign IP addresses
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.0.0.0", "255.255.255.0");

  ipv4.NewNetwork ();
  Ipv4InterfaceContainer is1 = ipv4.Assign (senderEdge);

  ipv4.NewNetwork ();
  Ipv4InterfaceContainer ir1 = ipv4.Assign (receiverEdge);

  // Populate routing tables
  NS_LOG_INFO ("Enable static global routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Select sender side port
  uint16_t port = 5000;

  // Install application on the sender
  //get the address of the reciever node NOT THE ROUTER!!!
  UdpClientHelper udpClient (ir1.GetAddress(1), port);
  udpClient.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  udpClient.SetAttribute ("PacketSize", UintegerValue (1024));
  // UdpClientHelper udpClient (ir1.GetAddress(1));
  ApplicationContainer sourceApps = udpClient.Install (sender.Get (0));
  sourceApps.Start (Seconds (1.0));
  sourceApps.Stop (Seconds(2.0));


  // Install application on the receiver
  //PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (receiver.Get (0));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds(5.0));


  NS_LOG_INFO ("Enable pcap tracing.");
  //
  // Do pcap tracing on all point-to-point devices on all nodes.
  //
  edgeLink.EnablePcapAll ("my_lineTopology_v01_SenderLink");
  bottleneckLink.EnablePcapAll ("my_lineTopology_v01_RecieverLink");

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}
