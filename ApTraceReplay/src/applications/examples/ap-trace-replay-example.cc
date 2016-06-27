/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Indian Institute of Technology Bombay
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
 * Author: Prakash Agrawal <prakashagr@cse.iitb.ac.in>
 *         Prof. Mythili Vutukuru <mythili@cse.iitb.ac.in>
 */

#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1448));
  
  uint32_t nWifi = 1; // number of clients to simulate
  double stopTime = 150;
  std::string pcapPath = "";
  std::string traceFile = "";
  std::string apMac = "";

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi clients", nWifi);
  cmd.AddValue ("pcapPath", "Path to input pcap file", pcapPath);
  cmd.AddValue ("traceFile", "Path to input trace file", traceFile);
  cmd.AddValue ("apMac", "MAC address of ap", apMac);
  cmd.Parse (argc, argv);

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("100MBps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("3ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channelPtr = channel.Create ();

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channelPtr);
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::MinstrelWifiManager");
  wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
  
  Ssid ssid = Ssid ("ns-3-ssid");
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (2.0),
                                 "DeltaY", DoubleValue (2.0),
                                 "GridWidth", UintegerValue (2),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  mobility.Install (wifiStaNodes);

  InternetStackHelper stack;
  stack.Install (p2pNodes.Get(1));
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterface;
  p2pInterface = address.Assign (p2pDevices.Get (1));
  Ipv4InterfaceContainer p2pInterfaceAp = address.Assign (p2pDevices.Get (0));
  // global routing will not work because we are creating upload nodes inside application.
  // Therefore do static routing
  Ptr<Ipv4> ipv4Ap = p2pNodes.Get(0)->GetObject<Ipv4> ();
  Ptr<Ipv4StaticRouting> staticRoutingAP = ipv4RoutingHelper.GetStaticRouting (ipv4Ap);
  staticRoutingAP->SetDefaultRoute(p2pInterface.GetAddress(0), 1);
  Ptr<Ipv4> ipv4Server = p2pNodes.Get(1)->GetObject<Ipv4> ();
  Ptr<Ipv4StaticRouting> staticRoutingServer = ipv4RoutingHelper.GetStaticRouting (ipv4Server);
  staticRoutingServer->SetDefaultRoute(p2pInterfaceAp.GetAddress(0), 1);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer wifiInterfaces;
  wifiInterfaces = address.Assign (staDevices);
  Ipv4InterfaceContainer wifiInterfaceAp = address.Assign (apDevices); 
  for (uint32_t i=0; i<nWifi; i++)
  {
    // for each node add route to AP
    ipv4RoutingHelper.GetStaticRouting (wifiStaNodes.Get(i)->GetObject<Ipv4> ())->SetDefaultRoute(wifiInterfaceAp.GetAddress(0), 1);
  } 

  for (uint16_t i=0; i<nWifi; i++)
  {
    // Set TraceReplay on user nodes
    TraceReplayHelper application (DataRate ("25MBps"));
    application.SetPcap (pcapPath);
    application.AssignStreams (i);
    application.SetStopTime (stopTime);
    application.SetPortNumber (49153 + 200 * i);
    application.Install (wifiStaNodes.Get (i), p2pNodes.Get (1), Address (p2pInterfaces.GetAddress (1)));
  }

  // traceFile = "overcrowding.dat";
  // apMac = "00:11:74:a3:a7:20";
  traceFile = "slowtalker.dat";
  apMac = "00:11:74:90:6e:10";
  // traceFile = "other_bss.dat";
  // apMac = "00:11:74:90:6e:10";

  // Initilize ApTraceReplay application
  APTraceReplayHelper application;
  application.SetTraceFile (traceFile);
  application.SetApMacAddress (apMac);
  application.SetApChannel (channelPtr);
  application.SetApSsid (ssid);
  application.SetServer (p2pNodes.Get (1), p2pInterface.GetAddress (0));
  application.SetNumDownload (nWifi);
  application.SetStartTime (Seconds (0.0));
  application.SetStopTime (Seconds (stopTime));
  application.Install (wifiApNode.Get (0), apDevices);

  // Do not use global routing
  // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Simulator::Stop (Seconds (stopTime));
  pointToPoint.EnablePcapAll ("ApTraceReplay");
  Simulator::Run();
  Simulator::Destroy ();
  return 0;
}
