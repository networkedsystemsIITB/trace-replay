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

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ap-bulk-send.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ApBulkSend");

NS_OBJECT_ENSURE_REGISTERED (ApBulkSend);

TypeId
ApBulkSend::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ApBulkSend")
    .SetParent<Application> ()
    .SetGroupName("Applications") 
    .AddConstructor<ApBulkSend> ()
    .AddAttribute ("SendSize", "The amount of data to send each time.",
                   UintegerValue (512),
                   MakeUintegerAccessor (&ApBulkSend::m_sendSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("DataRate", "The data rate of node.",
                   DataRateValue (DataRate ("5000kb/s")),
                   MakeDataRateAccessor (&ApBulkSend::m_dataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&ApBulkSend::m_peer),
                   MakeAddressChecker ())
  ;
  return tid;
}


ApBulkSend::ApBulkSend ()
  : m_socket (0),
    m_connected (false)
{
  NS_LOG_FUNCTION (this);
  m_current = false; 
  m_running = false;
  m_sendSize = 512;
  m_statusI = 0;
}

ApBulkSend::~ApBulkSend ()
{
  NS_LOG_FUNCTION (this);
}

void
ApBulkSend::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void ApBulkSend::StartApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), TcpSocketFactory::GetTypeId ());
      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind6 ();
        }
      else if (InetSocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind ();
        }
      m_socket->Connect (m_peer);
      m_socket->SetConnectCallback (
        MakeCallback (&ApBulkSend::ConnectionSucceeded, this),
        MakeCallback (&ApBulkSend::ConnectionFailed, this));
    }
}

void ApBulkSend::StopApplication (void) // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_connected = false;
    }
  else
    {
      NS_LOG_WARN ("BulkSendApplication found null socket to close in StopApplication");
    }
}

void ApBulkSend::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("ApBulkSend Connection succeeded");
  m_connected = true;
  CheckStatus ();
}

void ApBulkSend::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("ApBulkSend, Connection Failed");
}

void ApBulkSend::SetStatus (std::pair<bool, uint64_t> status)
{
  NS_LOG_FUNCTION (this);
  // set status for each 2 seconds slot
  m_status.push_back (status);
}

void ApBulkSend::SetDataRate (DataRate dataRate)
{
  m_dataRate = dataRate;
}

void ApBulkSend::SetPeer (Address address)
{
  m_peer = address;
}

void ApBulkSend::CheckStatus ()
{

  Time tNext (Seconds (2));
  // call check status after every 2 seconds slot
  Simulator::Schedule (tNext, &ApBulkSend::CheckStatus, this);

  std::pair<bool, uint64_t> status = m_status[(m_statusI++)%m_status.size ()];;
  m_current = status.first; // run in current slot or not
  m_dataRate = DataRate (status.second); // data rate for current slot

  // no need to do anything if alredy running
  if (m_current && !m_running)
  {  
    // start upload connections
    m_running = true;
    Time tNext (Seconds (m_sendSize*8/static_cast<double> (m_dataRate.GetBitRate ())));
    Simulator::Schedule (tNext, &ApBulkSend::SendPacket, this);
  }
  else if (!m_current && m_running)
  {
    // stop uploading data
    m_running = false;
    // Clear buffer at the end of slot by cancelling all pending send events
    Simulator::Cancel (m_sendEvent);
  }
}

void ApBulkSend::SendPacket ()
{
  if (m_connected && m_running)
  {
    // send packet only when running
    Ptr<Packet> packet = Create<Packet> (m_sendSize);
    m_socket->Send (packet);
    Time tNext (Seconds (m_sendSize*8/static_cast<double> (m_dataRate.GetBitRate ())));
    m_sendEvent = Simulator::Schedule (tNext, &ApBulkSend::SendPacket, this);
  }
}

} // Namespace ns3