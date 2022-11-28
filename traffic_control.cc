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
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//

// please do not use this syntax just add ns3:: 
// using namespace ns3;

NS_LOG_COMPONENT_DEFINE("trafficControlExample");

int main(int argc, char* argv[])
{

    ns3::CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    ns3::Time::SetResolution(ns3::Time::NS);
    ns3::LogComponentEnable("UdpEchoClientApplication", ns3::LOG_LEVEL_INFO);
    ns3::LogComponentEnable("UdpEchoServerApplication", ns3::LOG_LEVEL_INFO);

    ns3::NodeContainer nodes;
    nodes.Create(2);

    ns3::PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", ns3::StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", ns3::StringValue("2ms"));

    ns3::NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    ns3::InternetStackHelper stack;
    stack.Install(nodes);

    ns3::Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    ns3::Ipv4InterfaceContainer interfaces = address.Assign(devices);

    ns3::UdpEchoServerHelper echoServer(9);

    ns3::ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
    serverApps.Start(ns3::Seconds(1.0));
    serverApps.Stop(ns3::Seconds(10.0));

    ns3::UdpEchoClientHelper echoClient(interfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", ns3::UintegerValue(1));
    echoClient.SetAttribute("Interval", ns3::TimeValue(ns3::Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", ns3::UintegerValue(1024));

    ns3::ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(ns3::Seconds(2.0));
    clientApps.Stop(ns3::Seconds(10.0));

    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
    return 0;
}
