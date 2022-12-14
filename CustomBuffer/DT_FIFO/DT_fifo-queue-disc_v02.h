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

#ifndef DT_FIFO_QUEUE_DISC_V02_H
#define DT_FIFO_QUEUE_DISC_V02_H

#include "queue-disc.h"

namespace ns3 {

/**
 * \ingroup traffic-control
 *
 * Simple queue disc implementing the FIFO (First-In First-Out) policy.
 *
 */
// class DT_FifoQueueDisc_v02 : public CustomeQueueDisc {
class DT_FifoQueueDisc_v02 : public QueueDisc {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief FifoQueueDisc constructor
   *
   * Creates a queue with a depth of 1000 packets by default
   */
  DT_FifoQueueDisc_v02 ();

  virtual ~DT_FifoQueueDisc_v02();

  // Reasons for dropping packets
  static constexpr const char* LIMIT_EXCEEDED_DROP = "Queue disc limit exceeded";  //!< Packet dropped due to queue disc limit exceeded

private:
  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void);
  virtual bool CheckConfig (void);
  virtual void InitializeParams (void);
};

} // namespace ns3

#endif /* DT_FIFO_QUEUE_DISC_V02_H */
