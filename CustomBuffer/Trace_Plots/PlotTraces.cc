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

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
// #include "ns3/applications-module.h"
// #include "ns3/network-module.h"
// #include "ns3/internet-module.h"
// #include "ns3/point-to-point-module.h"
// #include "ns3/ipv4-global-routing-helper.h"
// #include "ns3/traffic-control-module.h"
// #include "ns3/flow-monitor-module.h"
// #include "tutorial-app.h"
// #include "custom_onoff-application.h"


using namespace ns3;

std::string dir = "./CustomBuffer/Trace_Plots/";
std::string queue_disc_type = "DT_FifoQueueDisc_v02";


int main (int argc, char *argv[])
{ 
  // command line needs to be in ./ns-3.36.1/scratch/ inorder for the script to produce gnuplot correctly///
  // system (("gnuplot " + dir + "gnuplotScriptTcHighPriorityPacketsInQueue").c_str ());
  // system (("gnuplot " + dir + "gnuplotScriptHighPriorityQueueThreshold").c_str ());
  // system (("gnuplot " + dir + "gnuplotScriptHighPriority").c_str ());
  // system (("gnuplot " + dir + "gnuplotScriptLowPriority").c_str ());
  system (("gnuplot " + dir + "gnuplotScriptTotalPacketsInQueue").c_str ());

  return 0;
}