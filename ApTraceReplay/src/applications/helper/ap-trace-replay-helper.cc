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

#include "ap-trace-replay-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("APTraceReplayHelper");

APTraceReplayHelper::APTraceReplayHelper ()
{
  NS_LOG_FUNCTION (this);
  m_traceFile = "";
  m_apMacAdd = "";
  m_startTime = Seconds (0);
  m_stopTime = Seconds (0);
  // Number of upload nodes created is set 10 by default. Can be changed by user
  m_numUpload = 10;
  // Number of download connections simulated by user is set 1 by default. Can be changed by user
  m_numDownload = 1;
}

APTraceReplayHelper::~APTraceReplayHelper ()
{
  NS_LOG_FUNCTION (this);
}

void
APTraceReplayHelper::SetTraceFile (std::string fileName)
{
  NS_LOG_FUNCTION (this);
  m_traceFile = fileName;
}

void
APTraceReplayHelper::SetApMacAddress (std::string macAdd)
{
  NS_LOG_FUNCTION (this);
  m_apMacAdd = macAdd;
}

void
APTraceReplayHelper::SetNumUpload (uint32_t num)
{
  NS_LOG_FUNCTION (this);
  m_numUpload = num;
}

void
APTraceReplayHelper::SetNumDownload (uint32_t num)
{
  NS_LOG_FUNCTION (this);
  m_numDownload = num;
}

void
APTraceReplayHelper::SetStartTime (Time time)
{
  NS_LOG_FUNCTION (this);
  m_startTime = time;
}

void
APTraceReplayHelper::SetStopTime (Time time)
{
  NS_LOG_FUNCTION (this);
  m_stopTime = time;
}

void
APTraceReplayHelper::SetApChannel (Ptr<YansWifiChannel> channel)
{
  NS_LOG_FUNCTION (this); 
  m_channel = channel;
}

void
APTraceReplayHelper::SetApSsid (Ssid ssid)
{
  NS_LOG_FUNCTION (this); 
  m_ssid = ssid;
}

void
APTraceReplayHelper::SetServer (Ptr<Node> remoteNode, Ipv4Address remoteAdd)
{
  NS_LOG_FUNCTION (this); 
  m_remoteNode = remoteNode;
  m_remoteAdd = remoteAdd;
}

void
APTraceReplayHelper::Install (Ptr<Node> apNode, NetDeviceContainer apDevices)
{
  NS_LOG_FUNCTION (this);

  // Convert trace file to custom formatted trace file
  if (!std::ifstream (m_traceFile.c_str ()))
    {
      std::cerr << "Invalid trace file.\n";
      exit (1);
    }

  // change permissions to execute scipts
  std::string command = "sudo chmod -R +x witals/";
  system (command.c_str ());  

  // run the scripts to generate trace file in output folder inside witals directory from '.dat' file 
  command = "sudo sh witals/all_in_one_total.sh output " + m_apMacAdd + " ../" + m_traceFile;  
  system (command.c_str ());  

  // calculate wasted air-time from trace file for each 2 seconds slot
  command = "witals/apTraceReplay.py witals/output/" + m_traceFile + ".txt " + m_apMacAdd + " > apTraceFile";
  system (command.c_str ());

  // slow talker intensity, overcrowding number and upload traffic intensity are calculated in witals/output/diagnosis_2sec for each 2 seconds slot
  std::string fileName = "witals/output/diagnosis_2sec";
  std::ifstream infile;
  infile.open (fileName.c_str ());
  std::vector<double> STArray;
  std::vector<double> OCArray;
  std::vector<double> UpArray;
  if (infile.is_open ())
    {
      double stIntensity;
      double ocIntensity;
      double upIntensity; 
      // read diagnosis_2sec line by line and store values in arrays
      while (infile >> stIntensity >> ocIntensity >> upIntensity)
      {
        STArray.push_back (stIntensity);       
        OCArray.push_back (ocIntensity);
        UpArray.push_back (upIntensity);
      }
    }

  infile.close ();

  std::string fname = "apTraceFile"; // apTraceFile contains wasted air-time
  std::ifstream in (fname.c_str ());
  std::vector<double> apOffTimeVec;
  if (!in.is_open ())
  {
    std::cout << "Error opening apTraceFile file.\n";
    exit (1);
  }
  uint32_t n;
  in >> n;
  for(uint32_t i=0; i<n; i++)
  {
    double offTime;
    in >> offTime;
    apOffTimeVec.push_back(offTime); // store wasted air-time in array
  }

  // Set Upload clients
  {
    NodeContainer uploadNodes;
    // create upload nodes
    uploadNodes.Create (m_numUpload);

    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    // use channel provided by user to connect to same frequency
    phy.SetChannel (m_channel);
    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::MinstrelWifiManager");
    wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
    
    NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (m_ssid),
                 "ActiveProbing", BooleanValue (false));

    NetDeviceContainer uploadDevices;
    uploadDevices = wifi.Install (phy, mac, uploadNodes);

    MobilityHelper mobility;

    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                   "MinX", DoubleValue (0.0),
                                   "MinY", DoubleValue (0.0),
                                   "DeltaX", DoubleValue (2.0),
                                   "DeltaY", DoubleValue (2.0),
                                   "GridWidth", UintegerValue (2),
                                   "LayoutType", StringValue ("RowFirst"));

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (uploadNodes);

    InternetStackHelper stack;
    stack.Install (uploadNodes);

    Ipv4AddressHelper address;
    Ipv4StaticRoutingHelper ipv4RoutingHelper;

    address.SetBase ("10.1.10.0", "255.255.255.0");
    Ipv4InterfaceContainer uploadInterfaces;
    address.Assign (uploadDevices);
    Ipv4InterfaceContainer uploadInterfaceAp = address.Assign (apDevices); 
    for (uint32_t i=0; i<m_numUpload; i++)
    {
      // Global routing does not work in this case as we are creating nodes inside application
      // Use static routing and add route to AP
      ipv4RoutingHelper.GetStaticRouting (uploadNodes.Get(i)->GetObject<Ipv4> ())->SetDefaultRoute(uploadInterfaceAp.GetAddress(0), 1);
    }

    std::vector<Ptr<ApBulkSend> > apBulkSendArray;

    for (uint32_t i=0; i<m_numUpload; i++)
    {
      // Install ap-bulk send connections
      Address remoteAdd = InetSocketAddress (m_remoteAdd, 80+i);

      // Install Sink at server for each connection
      PacketSinkHelper sink ("ns3::TcpSocketFactory", remoteAdd);
      ApplicationContainer sinkApp = sink.Install(m_remoteNode);
      sinkApp.Start (m_startTime);
      sinkApp.Stop (m_stopTime);

      // Install ap-bulk-send on each upload nodes
      Ptr<ApBulkSend> source = CreateObject<ApBulkSend> ();
      source->SetPeer (remoteAdd);
      source->SetStartTime (m_startTime);
      source->SetStopTime (m_stopTime);
      // source->SetStatus (std::make_pair(true, DataRate("1Mbps")));
      // source->SetStatus (std::make_pair(false, DataRate("1Mbps")));
      uploadNodes.Get (i)->AddApplication (source);  
      apBulkSendArray.push_back (source); 
    }

    for (uint32_t i=0; i<UpArray.size (); i++)
    {
      double intensity = UpArray[i]; // Upload traffic intensity in this 2 seconds slot
      uint32_t numUploadConn = 0;
      // application throughput of AP is 21Mbps
      // remove wasted air-time from it apOffTimeVec[i] for each seconds
      uint64_t dataRateAp = 21000000 * (1-apOffTimeVec[i]/2);
      uint64_t dataRateDown = (dataRateAp * (1-intensity/100));
      uint64_t dataRateUp = (dataRateAp-dataRateDown);

      // Ignore if upload traffic intensity is less than 5%
      if (intensity > 5)
      {
        numUploadConn++;
        // throughput per upload connection should be less than the average throughput per connection
        while (dataRateUp/numUploadConn > dataRateDown/m_numDownload)
        {
          numUploadConn++; // throughput per upload connection is greater than the average throughput per connection. Increase upload connection
        }
        for (uint32_t j=0; j<numUploadConn; j++)
        {
          // for each upload connection throughput will decrease by 3% due to increase in contention
          dataRateUp = dataRateUp * 0.97;
        }
      }
      NS_ASSERT_MSG (numUploadConn <= m_numUpload, "Required number of upload connections is less than provided upload connections!");
      
      for (uint32_t j=0; j<numUploadConn; j++)
      {
        // Out of m_numUpload upload connections, numUploadConn will be running in this 2 seconds slot
        apBulkSendArray[j]->SetStatus (std::make_pair(true, dataRateUp/numUploadConn));
      }
      for (uint32_t j=numUploadConn; j<m_numUpload; j++)
      {
        // Rest of upload connections will be silent in this 2 seconds slot
        apBulkSendArray[j]->SetStatus (std::make_pair(false, 1));
      }
    }
  }

  // Setup ap blocking
  Ptr<apBlockingApplication> apBlocker = CreateObject<apBlockingApplication> ();
  apBlocker->SetStartTime (m_startTime);
  apBlocker->SetStopTime (m_stopTime);
  apBlocker->setup(apOffTimeVec, 0);
  apNode->AddApplication(apBlocker);
}

} // namespace ns3
