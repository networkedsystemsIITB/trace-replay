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

#ifndef AP_TRACE_REPLAY_HELPER_H
#define AP_TRACE_REPLAY_HELPER_H

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/assert.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <map>

namespace ns3 {


/**
 * \brief APTraceReplayHelper is a helper to parse MAC trace file,
 * create upload clients and install upload application over them,
 * calculate number of upload clients neede to match upload traffic in
 * trace file and setup ApBlocker application in AP.
 * The ApBlocker replays wasted air-time and blockes the AP for
 * time which is calculated by MAC trace file.
 *
 */
class APTraceReplayHelper
{
public:

  APTraceReplayHelper (void);
  ~APTraceReplayHelper (void);


  /**
   * \brief This method sets the input trace file
   *
   * \param fileName path to input trace file
   */
  void SetTraceFile (std::string fileName);

  /**
   * \brief This method sets the mac address of AP in trace file
   *
   * \param macADD mac address of AP in trace file
   */  
  void SetApMacAddress (std::string macAdd);

  /**
   * \brief This method sets the start time for upload clients
   *
   * \param time start time of the application
   */ 
  void SetStartTime (Time time);

  /**
   * \brief This method sets the stop time for upload clients
   *
   * \param time stop time of the application
   */ 
  void SetStopTime (Time time);

  /**
   * \brief This method sets the number of upload clients that are initially created.
   *
   * The number of upload clients that are uploading data are calculated at run time.
   * This function is just to create nodes on which we can install upload applications.
   *
   * \param num Number of nodes to create
   */ 
  void SetNumUpload (uint32_t num);

  /**
   * \brief This method sets the number of download connections that user is simulating
   *
   * \param num Number of download connections that user is simulating
   */ 
  void SetNumDownload (uint32_t num);

  /**
   * \brief This method sets the channel on which all upload nodes tranfer data.
   * 
   * This channel must be same as the original AP channel on which all applications are installed.
   *
   * \param channel pointer to AP channel
   */ 
  void SetApChannel (Ptr<YansWifiChannel> channel);

  /**
   * \brief This method sets the SSID of access point
   *
   * \param ssid SSID of access point
   */ 
  void SetApSsid (Ssid ssid);

  /**
   * \brief This method sets the node pointer and ip address for server
   *
   * \param remoteNode pointer to server node
   * \param remoteAdd IP address of server
   */ 
  void SetServer (Ptr<Node> remoteNode, Ipv4Address remoteAdd);

  /**
   * \brief This method creates the upload nodes, parese the trace file and
   * installs the upload applications. It also installs ApBlocker on AP.
   *
   * \param apNode Pointer to AP node
   * \param apDevices Pointer to Ap device, use to install ApBlocker
   */ 
  void Install (Ptr<Node> apNode, NetDeviceContainer apDevices);

private:
  std::string           m_traceFile;  //!< path to input trace file
  std::string           m_apMacAdd;   //!< mac address of access point
  Time                  m_startTime;  //!< Start time of application
  Time                  m_stopTime;   //!< Stop time of application
  Ptr<Node>             m_remoteNode; //!< Pointer to Server node for upload clients
  Ipv4Address           m_remoteAdd;  //!< Ip address of server node for upload nodes
  Ptr<YansWifiChannel>  m_channel;    //!< WiFi channel to which upload clients will connect to
  uint32_t              m_numUpload;  //!< Number of upload clients connected to upload channel
  uint32_t              m_numDownload;//!< Number of download connections user is simulating
  Ssid                  m_ssid;       //!< SSID of access point to which upload clients will connect to
};

} // namespace ns3
#endif /* TRACE_REPLAY_HELPER_H */
