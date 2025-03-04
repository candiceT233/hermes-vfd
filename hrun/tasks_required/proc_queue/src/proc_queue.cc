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
#include "proc_queue/proc_queue.h"

namespace hrun::proc_queue {

class Server : public TaskLib {
 public:
  Server() = default;

  void Construct(ConstructTask *task, RunContext &rctx) {
    task->SetModuleComplete();
  }

  void Destruct(DestructTask *task, RunContext &rctx) {
    task->SetModuleComplete();
  }

  void Push(PushTask *task, RunContext &rctx) {
    switch (task->phase_) {
      case PushTaskPhase::kSchedule: {
        task->sub_run_.shm_ = task->sub_cli_.shm_;
        task->sub_run_.ptr_ = HRUN_CLIENT->GetPrivatePointer<Task>(task->sub_cli_.shm_);
        Task *&ptr = task->sub_run_.ptr_;
//        HILOG(kDebug, "Scheduling task {} on state {} tid {}",
//              ptr->task_node_, ptr->task_state_, GetLinuxTid());
        if (ptr->IsFireAndForget()) {
          ptr->UnsetFireAndForget();
        }
        MultiQueue *real_queue = HRUN_CLIENT->GetQueue(QueueId(ptr->task_state_));
        real_queue->Emplace(ptr->prio_, ptr->lane_hash_, task->sub_run_.shm_);
        task->phase_ = PushTaskPhase::kWaitSchedule;
      }
      case PushTaskPhase::kWaitSchedule: {
        Task *&ptr = task->sub_run_.ptr_;
        if (!ptr->IsComplete()) {
          return;
        }
        // TODO(llogan): handle fire & forget tasks gracefully
        task->SetModuleComplete();
      }
    }
  }

 public:
#include "proc_queue/proc_queue_lib_exec.h"
};

}  // namespace hrun::proc_queue

HRUN_TASK_CC(hrun::proc_queue::Server, "proc_queue");
