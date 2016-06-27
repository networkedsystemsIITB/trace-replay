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

#ifndef AP_BULK_SEND_H
#define AP_BULK_SEND_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Address;
class Socket;

class ApBulkSend : public Application
{
public:

  static TypeId GetTypeId (void);

  ApBulkSend ();

  virtual ~ApBulkSend ();

  /**
   * \brief This method sets data rate for connection
   *
   * \param DataRate data rate
   */
  void SetDataRate (DataRate);

  /**
   * \brief This method sets status for each 2 seconds slot.
   *
   * For each 2 seconds slot:
   *                        1. Run upload client in current slot on not
   *                        2. Data rate for this slot
   * \param status status of upload client for current slot
   */
  void SetStatus (std::pair<bool, uint64_t> status);

  /**
   * \brief This method sets the server address
   *
   * \param address server address
   */
  void SetPeer (Address address);

protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  bool            m_current;      //!< run in current slot or not
  bool            m_running;      //!< currently running or not
  Ptr<Socket>     m_socket;       //!< Associated socket
  Address         m_peer;         //!< Peer address
  bool            m_connected;    //!< True if connected
  uint32_t        m_sendSize;     //!< Size of data to send each time
  DataRate        m_dataRate;     //!< data rate for current slot
  EventId         m_sendEvent;    //!< send event
  std::vector<std::pair<bool, uint64_t> > m_status; //!<  status for every 2 sec. Pair of <run in this slot, data rate>

private:

  void ConnectionSucceeded (Ptr<Socket> socket);

  void ConnectionFailed (Ptr<Socket> socket);

  void CheckStatus ();

  void SendPacket ();
};
} // namespace ns3
#endif /* AP_BULK_SEND_H */
