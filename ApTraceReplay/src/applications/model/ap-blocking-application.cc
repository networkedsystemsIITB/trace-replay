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

 #include "ap-blocking-application.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("apBlockingApplication");
NS_OBJECT_ENSURE_REGISTERED (apBlockingApplication);

TypeId apBlockingApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::apBlockingApplication")
    .SetParent<Application> ()
    .SetGroupName ("Applications")
    .AddConstructor<apBlockingApplication> ()
  ;
  return tid;
}

apBlockingApplication::apBlockingApplication ()
{
  NS_LOG_FUNCTION (this);
  m_apBlocked = false;
  m_IterationCount = -1;

}

apBlockingApplication::~apBlockingApplication ()
{
  NS_LOG_FUNCTION (this);
}

void apBlockingApplication::setup(std::vector<double> offTimeVec, uint32_t Rval){ 
  // call from startApplication
  NS_LOG_FUNCTION (this);
  m_RVal = Rval;
  m_offTimeVec = offTimeVec;
  m_offTimeVecIt = m_offTimeVec.begin();
}

void apBlockingApplication::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  m_offTime = *(m_offTimeVecIt++);
  srand(m_RVal);
  flipCoin();
}

void apBlockingApplication::flipCoin(){

  m_IterationCount++;
  if(m_IterationCount == 20000){
    m_IterationCount = 0; // this is the last slot for this 2 seconds
    // If vector reaches end than roll it over again and start from begining of vector
    if(m_offTimeVecIt == m_offTimeVec.end()) m_offTimeVecIt = m_offTimeVec.begin();
    m_offTime = *(m_offTimeVecIt++);
  }

  if(m_offTime < 0.0001){
    // No need to do anything, Call the same function after 100uSec
    Time tnext (Seconds(0.001));
    Simulator::Schedule (tnext, &apBlockingApplication::flipCoin, this);  
    return;  
  }

  // Flip coin
  if(rand()%2 == 1 || m_offTime > (20000 - m_IterationCount - 1)*0.0001){
    blockAp();
  }
  else{
    // Call the same function after 100uSec
    Time tnext (Seconds(0.0001));
    Simulator::Schedule (tnext, &apBlockingApplication::flipCoin, this);
  }

}

void apBlockingApplication::blockAp(){
  
  Config::Set("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/EnergyDetectionThreshold"
              , DoubleValue(100));
  if(m_offTime > 0.001){
    m_offTime -= 0.001;
  }
    Time tnext (Seconds(0.001));
    Simulator::Schedule (tnext, &apBlockingApplication::unblockAp, this);
    return;
}

void apBlockingApplication::unblockAp(){

  Config::Set("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/EnergyDetectionThreshold"
              , DoubleValue(-96.0)); 
  flipCoin();
}

}
