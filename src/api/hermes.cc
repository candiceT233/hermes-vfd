//
// Created by lukemartinlogan on 12/3/22.
//

#include "hermes.h"
#include "bucket.h"
#include "vbucket.h"

namespace hermes::api {

void Hermes::Init(HermesType mode,
                  std::string server_config_path,
                  std::string client_config_path) {
  mode_ = mode;
  switch (mode_) {
    case HermesType::kServer: {
      InitServer(std::move(server_config_path));
      break;
    }
    case HermesType::kClient: {
      InitClient(std::move(server_config_path),
                 std::move(client_config_path));
      break;
    }
    case HermesType::kColocated: {
      InitColocated(std::move(server_config_path),
                    std::move(client_config_path));
      break;
    }
  }
}

void Hermes::Finalize() {
  switch (mode_) {
    case HermesType::kServer: {
      FinalizeServer();
      break;
    }
    case HermesType::kClient: {
      FinalizeClient();
      break;
    }
    case HermesType::kColocated: {
      FinalizeColocated();
      break;
    }
  }
}

void Hermes::RunDaemon() {
  rpc_.RunDaemon();
}

void Hermes::StopDaemon() {
  rpc_.StopDaemon();
}

void Hermes::InitServer(std::string server_config_path) {
  LoadServerConfig(server_config_path);
  InitSharedMemory();
  mdm_.shm_init(&header_->mdm_);
  rpc_.InitServer();
  rpc_.InitClient();
}

void Hermes::InitClient(std::string server_config_path,
                        std::string client_config_path) {
  LoadServerConfig(server_config_path);
  LoadClientConfig(client_config_path);
  LoadSharedMemory();
  mdm_.shm_deserialize(&header_->mdm_);
  rpc_.InitClient();
}

void Hermes::InitColocated(std::string server_config_path,
                           std::string client_config_path) {
  LoadServerConfig(server_config_path);
  LoadClientConfig(client_config_path);
  InitSharedMemory();
  mdm_.shm_init(&header_->mdm_);
  rpc_.InitColocated();
}

void Hermes::LoadServerConfig(std::string config_path) {
  if (config_path.size() == 0) {
    config_path = GetEnvSafe(kHermesServerConf);
  }
  server_config_.LoadFromFile(config_path);
}

void Hermes::LoadClientConfig(std::string config_path) {
  if (config_path.size() == 0) {
    config_path = GetEnvSafe(kHermesClientConf);
  }
  client_config_.LoadFromFile(config_path);
}

void Hermes::InitSharedMemory() {
  // Create shared-memory allocator
  auto mem_mngr = LABSTOR_MEMORY_MANAGER;
  mem_mngr->CreateBackend(lipc::MemoryBackendType::kPosixShmMmap,
                          server_config_.shmem_name_);
  main_alloc_ =
      mem_mngr->CreateAllocator(lipc::AllocatorType::kPageAllocator,
                                server_config_.shmem_name_, main_alloc_id,
                                sizeof(HermesShmHeader),
                                lipc::MemoryManager::kDefaultSlotSize);
  header_ = main_alloc_->GetCustomHeader<HermesShmHeader>();
}

void Hermes::LoadSharedMemory() {
  // Load shared-memory allocator
  auto mem_mngr = LABSTOR_MEMORY_MANAGER;
  mem_mngr->AttachBackend(lipc::MemoryBackendType::kPosixShmMmap,
                          server_config_.shmem_name_);
  main_alloc_ = mem_mngr->GetAllocator(main_alloc_id);
  header_ = main_alloc_->GetCustomHeader<HermesShmHeader>();
}

void Hermes::FinalizeServer() {
  // NOTE(llogan): Finalize() is called internally by daemon in this case
  LABSTOR_MEMORY_MANAGER->DestroyBackend(server_config_.shmem_name_);
}

void Hermes::FinalizeClient() {
  if (client_config_.stop_daemon_) {
    StopDaemon();
  }
  rpc_.Finalize();
}

void Hermes::FinalizeColocated() {
  rpc_.Finalize();
}

std::shared_ptr<Bucket> Hermes::GetBucket(std::string name,
                                          Context ctx) {
  return std::make_shared<Bucket>(name, ctx);
}

std::shared_ptr<VBucket> Hermes::GetVBucket(std::string name,
                                            Context ctx) {
  return std::make_shared<VBucket>(name, ctx);
}

}  // namespace hermes::api