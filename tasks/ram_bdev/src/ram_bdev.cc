//
// Created by lukemartinlogan on 6/29/23.
//

#include "hrun_admin/hrun_admin.h"
#include "hrun/api/hrun_runtime.h"
#include "ram_bdev/ram_bdev.h"
#include "hermes/slab_allocator.h"

namespace hermes::ram_bdev {

class Server : public TaskLib, public bdev::Server {
 public:
  SlabAllocator alloc_;
  char *mem_ptr_;

 public:
  void Construct(ConstructTask *task, RunContext &rctx) {
    DeviceInfo &dev_info = task->info_;
    rem_cap_ = dev_info.capacity_;
    alloc_.Init(id_, dev_info.capacity_, dev_info.slab_sizes_);
    mem_ptr_ = (char*)malloc(dev_info.capacity_);
    // score_hist_.Resize(10);
    HILOG(kDebug, "Created {} at {} of size {}",
          dev_info.dev_name_, dev_info.mount_point_, dev_info.capacity_);
    task->SetModuleComplete();
  }

  void Destruct(DestructTask *task, RunContext &rctx) {
    free(mem_ptr_);
    task->SetModuleComplete();
  }

  void Allocate(AllocateTask *task, RunContext &rctx) {
    HILOG(kDebug, "Allocating {} bytes (RAM)", task->size_);
    alloc_.Allocate(task->size_, *task->buffers_, task->alloc_size_);
    rem_cap_ -= task->alloc_size_;
    // score_hist_.Increment(task->score_);
    HILOG(kDebug, "Allocated {} bytes (RAM)", task->alloc_size_);
    task->SetModuleComplete();
  }

  void Free(FreeTask *task, RunContext &rctx) {
    rem_cap_ += alloc_.Free(task->buffers_);
    // score_hist_.Decrement(task->score_);
    task->SetModuleComplete();
  }

  void Write(WriteTask *task, RunContext &rctx) {
    HILOG(kDebug, "Writing {} bytes to RAM", task->size_);
    memcpy(mem_ptr_ + task->disk_off_, task->buf_, task->size_);
    task->SetModuleComplete();
  }

  void Read(ReadTask *task, RunContext &rctx) {
    HILOG(kDebug, "Reading {} bytes from RAM", task->size_);
    memcpy(task->buf_, mem_ptr_ + task->disk_off_, task->size_);
    task->SetModuleComplete();
  }
 public:
#include "bdev/bdev_lib_exec.h"
};

}  // namespace hrun

HRUN_TASK_CC(hermes::ram_bdev::Server, "ram_bdev");
