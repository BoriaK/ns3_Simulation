/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universita' degli Studi di Napoli Federico II
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
 * Authors:  Stefano Avallone <stavallo@unina.it>
 */

#include "ns3/log.h"
#include "DT_fifo-queue-disc_v02.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/socket.h"
#include "customTag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DT_FifoQueueDisc_v02");

NS_OBJECT_ENSURE_REGISTERED (DT_FifoQueueDisc_v02);

TypeId DT_FifoQueueDisc_v02::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DT_FifoQueueDisc_v02")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<DT_FifoQueueDisc_v02> ()
    .AddAttribute ("MaxSize",
                   "The max queue size",
                   QueueSizeValue (QueueSize ("1000p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
  ;
  return tid;
}

DT_FifoQueueDisc_v02::DT_FifoQueueDisc_v02 ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  NS_LOG_FUNCTION (this);
}

DT_FifoQueueDisc_v02::~DT_FifoQueueDisc_v02 ()
{
  NS_LOG_FUNCTION (this);
}

bool
DT_FifoQueueDisc_v02::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);
  
  int alpha_l = 1;
  int alpha_h = 2;
  int alpha;

 ///////////////////////////////////////////////////////// 
  // set a besic Packet clasification based on arbitrary Tag from recieved packet:
  // flow_priority = 0 is high priority, flow_priority = 1 is low priority
  uint8_t flow_priority = 0;
  MyTag flowPrioTag;
  if (item->GetPacket ()->PeekPacketTag (flowPrioTag))
    {
      flow_priority = flowPrioTag.GetSimpleValue();
    }
  
  if (flow_priority == 0)
    {
      alpha = alpha_h;
    }
  else
    {
      alpha = alpha_l;
    }
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
  // // set a besic Packet clasification based on packetSequence Length Tag ("flowPacketCounterTag") from recieved packet:
  // // set threshold to (arbirary) value
  // // if flowPacketCounterTag <= Threshold then flow_priority is high
  // // if flowPacketCounterTag > Threshold then flow_priority is low
  // // flow_priority = 0 is high priority, flow_priority = 1 is low priority
  // uint8_t Threshold = 10; // [packets], max number of packets per flow to be considered mouse flow
  // uint64_t packetSeqCount = 0;
  // MyTag flowPacketCounterTag;
  //   if (item->GetPacket ()->PeekPacketTag (flowPacketCounterTag))
  //   {
  //     packetSeqCount = flowPacketCounterTag.GetSimpleValue();
  //   }
    
  //   if (packetSeqCount <= Threshold)
  //     {
  //       alpha = alpha_h;
  //     }
  //   else
  //     {
  //       alpha = alpha_l;
  //     }
///////////////////////////////////////////////////////////////////////////////


  // if (GetCurrentSize () + item > GetMaxSize ())
  if ((GetCurrentSize () + item > GetQueueThreshold(alpha, alpha_l, alpha_h)) or (GetCurrentSize () + item > GetMaxSize ()))
    {
      // NS_LOG_LOGIC ("Queue full -- dropping pkt");
      NS_LOG_LOGIC ("Queue exceeds threshold -- dropping pkt");
      DropBeforeEnqueue (item, LIMIT_EXCEEDED_DROP);
      return false;
    }

  bool retval = GetInternalQueue (0)->Enqueue (item);
  
  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());
  NS_LOG_LOGIC ("Enqueue Threshold " << GetQueueThreshold (alpha, alpha_l, alpha_h));

  return retval;
}

Ptr<QueueDiscItem>
DT_FifoQueueDisc_v02::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item = GetInternalQueue (0)->Dequeue ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return item;
}

Ptr<const QueueDiscItem>
DT_FifoQueueDisc_v02::DoPeek (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<const QueueDiscItem> item = GetInternalQueue (0)->Peek ();

  if (!item)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return item;
}

bool
DT_FifoQueueDisc_v02::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("FifoQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("FifoQueueDisc needs no packet filter");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // add a DropTail queue
      AddInternalQueue (CreateObjectWithAttributes<DropTailQueue<QueueDiscItem> >
                          ("MaxSize", QueueSizeValue (GetMaxSize ())));
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("FifoQueueDisc needs 1 internal queue");
      return false;
    }

  return true;
}

void
DT_FifoQueueDisc_v02::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
