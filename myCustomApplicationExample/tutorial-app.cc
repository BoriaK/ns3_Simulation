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
 */

#include "tutorial-app.h"
#include "ns3/applications-module.h"
#include "customTag.h"

using namespace ns3;

TutorialApp::TutorialApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

TutorialApp::~TutorialApp ()
{
  m_socket = 0;
}

/* static */
TypeId TutorialApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("TutorialApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<TutorialApp> ()
    ;
  return tid;
}

void
TutorialApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
TutorialApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
TutorialApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

// // add custom Tag to each packet:
// class MyTag : public Tag
// {
// public:
//   /**
//    * \brief Get the type ID.
//    * \return the object TypeId
//    */
//   static TypeId GetTypeId (void);
//   virtual TypeId GetInstanceTypeId (void) const;
//   virtual uint32_t GetSerializedSize (void) const;
//   virtual void Serialize (TagBuffer i) const;
//   virtual void Deserialize (TagBuffer i);
//   virtual void Print (std::ostream &os) const;

//   // these are our accessors to our tag structure
//   /**
//    * Set the tag value
//    * \param value The tag value.
//    */
//   void SetSimpleValue (uint8_t value);
//   /**
//    * Get the tag value
//    * \return the tag value.
//    */
//   uint8_t GetSimpleValue (void) const;
// private:
//   uint8_t m_simpleValue;  //!< tag value
// };

// TypeId 
// MyTag::GetTypeId (void)
// {
//   static TypeId tid = TypeId ("ns3::MyTag")
//     .SetParent<Tag> ()
//     .AddConstructor<MyTag> ()
//     .AddAttribute ("SimpleValue",
//                    "A simple value",
//                    EmptyAttributeValue (),
//                    MakeUintegerAccessor (&MyTag::GetSimpleValue),
//                    MakeUintegerChecker<uint8_t> ())
//   ;
//   return tid;
// }
// TypeId 
// MyTag::GetInstanceTypeId (void) const
// {
//   return GetTypeId ();
// }
// uint32_t 
// MyTag::GetSerializedSize (void) const
// {
//   return 1;
// }
// void 
// MyTag::Serialize (TagBuffer i) const
// {
//   i.WriteU8 (m_simpleValue);
// }
// void 
// MyTag::Deserialize (TagBuffer i)
// {
//   m_simpleValue = i.ReadU8 ();
// }
// void 
// MyTag::Print (std::ostream &os) const
// {
//   os << "v=" << (uint32_t)m_simpleValue;
// }
// void 
// MyTag::SetSimpleValue (uint8_t value)
// {
//   m_simpleValue = value;
// }
// uint8_t 
// MyTag::GetSimpleValue (void) const
// {
//   return m_simpleValue;
// }

void
TutorialApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  // create a tag.
  MyTag flowPrioTag;
  flowPrioTag.SetSimpleValue (0x0);

  // store the tag in a packet.
  packet->AddPacketTag (flowPrioTag);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
TutorialApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &TutorialApp::SendPacket, this);
    }
}
