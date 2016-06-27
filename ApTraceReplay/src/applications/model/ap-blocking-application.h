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
 */
#ifndef AP_BLOCKING_APPLICATION_H
#define AP_BLOCKING_APPLICATION_H

#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <cstdlib>
#include <ctime>
#include <map>

namespace ns3 {

/* \brief ApBlockingApplication is an application to replay wasted air-time from MAC layer trace.
* For each 2 seconds slot, it blocks the AP for amount of time which was wasted during experiment
* due to non-wifi devices or other BSS nearby.
* The wasted air-time is replayed in slots of 100us. For each 100us, it randomly decides whether or
* not to block the AP, so that the total time blocked matches calculated wasted air-time.
*
*/
class apBlockingApplication : public Application
{
public:

  static TypeId GetTypeId (void);

  apBlockingApplication ();
  virtual ~apBlockingApplication ();

  /**
   * Helper function used to set the application.
   *
   * \param offTimeVec vector containing off time for every 2 seconds slot
   * \param RVal random number
   */
  void setup(std::vector<double> offTimeVec, uint32_t RVal);

private:
  std::vector<double>           m_offTimeVec;    //!< vector containing off time for every 2 seconds slot
  uint32_t                      m_RVal;          //!< Random number
  std::vector<double>::iterator m_offTimeVecIt;  //!< pointer to vector containing off times
  bool                          m_apBlocked;     //!< AP blocked or unblocked
  uint32_t                      m_IterationCount;//!< 2 seconds slot is divided into 2000 slots
  double                        m_offTime;       //!< off time in current slot

  virtual void StartApplication (void);

  /**
   * For each 100us slot, this function decides whether or not to block the AP.
   *
   */
  void flipCoin();

  /**
   * This function blocks the AP by increasing the EDM of AP
   *
   */
  void blockAp();

  /**
   * This function unblocks the AP by changing the EDM of AP back to original value
   *
   */
  void unblockAp();
};

} // namespace ns3
#endif /* AP_BLOCKING_APPLICATION_H */