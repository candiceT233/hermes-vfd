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

#ifndef HRUN_INCLUDE_HRUN_WORK_ORCHESTRATOR_WORK_ORCHESTRATOR_H_
#define HRUN_INCLUDE_HRUN_WORK_ORCHESTRATOR_WORK_ORCHESTRATOR_H_

#include "hrun/hrun_types.h"
#include "hrun/api/hrun_runtime.h"
#include "hrun/queue_manager/queue_manager_runtime.h"
#include "worker.h"
#include "hrun/network/rpc_thallium.h"
#include <thread>

namespace hrun {

class WorkOrchestrator {
 public:
  ServerConfig *config_;  /**< The server configuration */
  std::vector<Worker> workers_;  /**< Workers execute tasks */
  std::atomic<bool> stop_runtime_;  /**< Begin killing the runtime */
  std::atomic<bool> kill_requested_;  /**< Kill flushing threads eventually */
  ABT_xstream xstream_;

 public:
  /** Default constructor */
  WorkOrchestrator() = default;

  /** Destructor */
  ~WorkOrchestrator() = default;

  /** Create thread pool */
  void ServerInit(ServerConfig *config, QueueManager &qm) {
    config_ = config;

    // Initialize argobots
    ABT_init(0, nullptr);

    // Create argobots xstream
    int ret = ABT_xstream_create(ABT_SCHED_NULL, &xstream_);
    if (ret != ABT_SUCCESS) {
      HELOG(kFatal, "Could not create argobots xstream");
    }

    // Spawn workers on the stream
    size_t num_workers = config_->wo_.max_workers_;
    workers_.reserve(num_workers);
    for (u32 worker_id = 0; worker_id < num_workers; ++worker_id) {
      workers_.emplace_back(worker_id, xstream_);
    }
    stop_runtime_ = false;
    kill_requested_ = false;

    // Schedule admin queue on worker 0
    MultiQueue *admin_queue = qm.GetQueue(qm.admin_queue_);
    LaneGroup *admin_group = &admin_queue->GetGroup(0);
    for (u32 lane_id = 0; lane_id < admin_group->num_lanes_; ++lane_id) {
      Worker &worker = workers_[0];
      worker.PollQueues({WorkEntry(0, lane_id, admin_queue)});
    }
    admin_group->num_scheduled_ = admin_group->num_lanes_;

    HILOG(kInfo, "Started {} workers", num_workers);
  }

  /** Get worker with this id */
  Worker& GetWorker(u32 worker_id) {
    return workers_[worker_id];
  }

  /** Get the number of workers */
  size_t GetNumWorkers() {
    return workers_.size();
  }

  /** Begin finalizing the runtime */
  void FinalizeRuntime() {
    stop_runtime_.store(true);
  }

  /** Finalize thread pool */
  void Join() {
    kill_requested_.store(true);
    for (Worker &worker : workers_) {
      worker.thread_->join();
      ABT_xstream_join(xstream_);
      ABT_xstream_free(&xstream_);
    }
  }

  /** Whether threads should still be executing */
  bool IsAlive() {
    return !kill_requested_.load();
  }

  /** Whether runtime should still be executing */
  bool IsRuntimeAlive() {
    return !stop_runtime_.load();
  }

  /** Spawn an argobots thread */
  template<typename FUNC, typename TaskT>
  ABT_thread SpawnAsyncThread(FUNC &&func, TaskT *data) {
    ABT_thread tl_thread;
    int ret = ABT_thread_create_on_xstream(xstream_,
                                           func, (void*) data,
                                           ABT_THREAD_ATTR_NULL,
                                           &tl_thread);
    if (ret != ABT_SUCCESS) {
      HELOG(kFatal, "Couldn't spawn worker");
    }
    return tl_thread;
  }

  /** Wait for argobots thread */
  void JoinAsyncThread(ABT_thread tl_thread) {
    ABT_thread_join(tl_thread);
  }
};

}  // namespace hrun

#define HRUN_WORK_ORCHESTRATOR \
  (&HRUN_RUNTIME->work_orchestrator_)

#endif  // HRUN_INCLUDE_HRUN_WORK_ORCHESTRATOR_WORK_ORCHESTRATOR_H_
