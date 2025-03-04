#ifndef HRUN_SRC_CONFIG_HRUN_SERVER_DEFAULT_H_
#define HRUN_SRC_CONFIG_HRUN_SERVER_DEFAULT_H_
const inline char* kHrunServerDefaultConfigStr = 
"### Runtime orchestration settings\n"
"work_orchestrator:\n"
"  # The number of worker threads to spawn\n"
"  max_workers: 4\n"
"\n"
"### Queue Manager settings\n"
"queue_manager:\n"
"  # The default depth of allocated queues\n"
"  queue_depth: 256\n"
"  # The maximum number of lanes per queue\n"
"  max_lanes: 16\n"
"  # The maximum number of queues\n"
"  max_queues: 1024\n"
"  # The shared memory allocator to use\n"
"  shm_allocator: kScalablePageAllocator\n"
"  # The name of the shared memory region to create\n"
"  shm_name: \"hrun_shm\"\n"
"  # The size of the shared memory region to allocate for general data structures\n"
"  shm_size: 0g\n"
"  # The size of the shared memory to allocate for data buffers\n"
"\n"
"### Define properties of RPCs\n"
"rpc:\n"
"  # A path to a file containing a list of server names, 1 per line. If your\n"
"  # servers are named according to a pattern (e.g., server-1, server-2, etc.),\n"
"  # prefer the `rpc_server_base_name` and `rpc_host_number_range` options. If this\n"
"  # option is not empty, it will override anything in `rpc_server_base_name`.\n"
"  host_file: \"\"\n"
"\n"
"  # Host names can be defined using the following syntax:\n"
"  # ares-comp-[0-9]-40g will convert to ares-comp-0-40g, ares-comp-1-40g, ...\n"
"  # ares-comp[00-09] will convert to ares-comp-00, ares-comp-01, ...\n"
"  host_names: [\"localhost\"]\n"
"\n"
"  # The RPC protocol. This must come from the documentation of the specific RPC\n"
"  # library in use.\n"
"  protocol: \"ofi+sockets\"\n"
"\n"
"  # RPC domain name for verbs transport. Blank for tcp.\n"
"  domain: \"\"\n"
"\n"
"  # Desired RPC port number.\n"
"  port: 8080\n"
"\n"
"  # The number of handler threads for each RPC server.\n"
"  num_threads: 4\n"
"\n"
"### Task Registry\n"
"task_registry: [\n"
"  \'hermes_mdm\',\n"
"  \'hermes_blob_mdm\',\n"
"  \'hermes_bucket_mdm\',\n"
"  \'hermes_data_op\',\n"
"  \'data_stager\',\n"
"  \'posix_bdev\',\n"
"  \'ram_bdev\'\n"
"]\n";
#endif  // HRUN_SRC_CONFIG_HRUN_SERVER_DEFAULT_H_