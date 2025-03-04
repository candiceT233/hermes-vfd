/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "hrun_admin/hrun_admin.h"
#include "hrun/api/hrun_runtime.h"
#include "worch_queue_round_robin/worch_queue_round_robin.h"

namespace hrun::worch_queue_round_robin {

class Server : public TaskLib {
 public:
  u32 count_;

 public:
  void Construct(ConstructTask *task, RunContext &rctx) {
    count_ = 0;
    task->SetModuleComplete();
  }

  void Destruct(DestructTask *task, RunContext &rctx) {
    task->SetModuleComplete();
  }

  void Schedule(ScheduleTask *task, RunContext &rctx) {
    // Check if any new queues need to be scheduled
    for (MultiQueue &queue : *HRUN_QM_RUNTIME->queue_map_) {
      if (queue.id_.IsNull() || !queue.flags_.Any(QUEUE_READY)) {
        continue;
      }
      for (LaneGroup &lane_group : *queue.groups_) {
        // NOTE(llogan): Assumes a minimum of two workers, admin on worker 0.
        if (lane_group.IsLowPriority()) {
          for (u32 lane_id = lane_group.num_scheduled_; lane_id < lane_group.num_lanes_; ++lane_id) {
            // HILOG(kDebug, "Scheduling the queue {} (lane {})", queue.id_, lane_id);
            Worker &worker = HRUN_WORK_ORCHESTRATOR->workers_[0];
            worker.PollQueues({WorkEntry(lane_group.prio_, lane_id, &queue)});
          }
          lane_group.num_scheduled_ = lane_group.num_lanes_;
        } else {
          for (u32 lane_id = lane_group.num_scheduled_; lane_id < lane_group.num_lanes_; ++lane_id) {
            // HILOG(kDebug, "Scheduling the queue {} (lane {})", queue.id_, lane_id);
            u32 worker_id = (count_ % (HRUN_WORK_ORCHESTRATOR->workers_.size() - 1)) + 1;
            Worker &worker = HRUN_WORK_ORCHESTRATOR->workers_[worker_id];
            worker.PollQueues({WorkEntry(lane_group.prio_, lane_id, &queue)});
            count_ += 1;
          }
          lane_group.num_scheduled_ = lane_group.num_lanes_;
        }
      }
    }
  }

#include "worch_queue_round_robin/worch_queue_round_robin_lib_exec.h"
};

}  // namespace hrun

HRUN_TASK_CC(hrun::worch_queue_round_robin::Server, "worch_queue_round_robin");
