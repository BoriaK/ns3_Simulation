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
#include "DT2_fifo-queue-disc.h"
#include "ns3/object-factory.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/socket.h"
#include "customTag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DT2_FifoQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (DT2_FifoQueueDisc);

TypeId DT2_FifoQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DT2_FifoQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<DT2_FifoQueueDisc> ()
    .AddAttribute ("MaxSize",
                   "The max queue size",
                   QueueSizeValue (QueueSize ("1000p")),
                   MakeQueueSizeAccessor (&QueueDisc::SetMaxSize,
                                          &QueueDisc::GetMaxSize),
                   MakeQueueSizeChecker ())
  ;
  return tid;
}

DT2_FifoQueueDisc::DT2_FifoQueueDisc ()
  : QueueDisc (QueueDiscSizePolicy::SINGLE_INTERNAL_QUEUE)
{
  NS_LOG_FUNCTION (this);
}

DT2_FifoQueueDisc::~DT2_FifoQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

bool
DT2_FifoQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);
  
  int alpha_l = 1;
  int alpha_h = 2;
  int alpha;
  
  // // set a besic Packet clasification based on packet size:
  // uint32_t Packet_Size_Threshold = 60; // define a packet size [bytes] threshold to assign to different alphas.
  // // for packets < Packet_Size_Threshold, assign high priority -> alpha = alpha_h
  // // for packets >= Packet_Size_Threshold, assign low priority -> alpha = alpha_l
  // if (item->GetSize () < Packet_Size_Threshold)
  //   {
  //     alpha = alpha_h;
  //   }
  // else
  //   {
  //     alpha = alpha_l; 
  //   }
  
  // set a besic Packet clasification based on flowPriorityTag from recieved packet:
  // flow_priority = 0 is high priority, flow_priority = 1 is low priority
  // uint8_t flow_priority = 0;
  // SocketFlowPriorityTag flowPriorityTag;
  // if (item->GetPacket ()->PeekPacketTag (flowPriorityTag))
  //   {
  //     flow_priority = flowPriorityTag.GetFlowPriority ();
  //   }
  
  // if (flow_priority == 0)
  //   {
  //     alpha = alpha_h;
  //   }
  // else
  //   {
  //     alpha = alpha_l;
  //   }

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


  // If Queue::Enqueue fails, QueueDisc::DropBeforeEnqueue is called by the
  // internal queue because QueueDisc::AddInternalQueue sets the trace callback



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
DT2_FifoQueueDisc::DoDequeue (void)
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
DT2_FifoQueueDisc::DoPeek (void)
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
DT2_FifoQueueDisc::CheckConfig (void)
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
DT2_FifoQueueDisc::InitializeParams (void)
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
