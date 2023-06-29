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

#include "apriori_prefetcher.h"
#include "hermes.h"



// TODO(candice): add for custom prefetch
#include "../adapter/mapper/abstract_mapper.h"


namespace hermes {

/** Constructor. Parse YAML schema. */
AprioriPrefetcher::AprioriPrefetcher() {
  auto &path = HERMES->server_config_.prefetcher_.apriori_schema_path_;
  auto real_path = hshm::ConfigParse::ExpandPath(path);
  auto is_mpi = HERMES->server_config_.prefetcher_.is_mpi_;
  HILOG(kDebug, "Start load apriori schema {}", real_path)

  try {
      YAML::Node yaml_conf = YAML::LoadFile(real_path);
      if (is_mpi) {
          ParseSchema(yaml_conf);
      } else {
          ParseSchema2(yaml_conf);
      }
      HILOG(kDebug, "Complete load of apriori schema {}", real_path)
  } catch (std::exception& e) {
      HELOG(kFatal, e.what())
  }
}

void ParseList(std::vector<std::string> &list, YAML::Node node) {
  list.reserve(node.size());
  for (YAML::Node sub_node : node) {
    list.emplace_back(sub_node.as<std::string>());
  }
}

/** Parse YAML schema. */
void AprioriPrefetcher::ParseSchema(YAML::Node &schema) {

  for (const auto &rank_node_pair : schema) {
    const YAML::Node& rank_node = rank_node_pair.first;
    const YAML::Node& rank_instrs = rank_node_pair.second;
    int rank = rank_node.as<int>();

    std::vector<AprioriPrefetchInstr>& instr_list = rank_info_[rank];
    for (const YAML::Node& instr_list_node : rank_instrs) {
      AprioriPrefetchInstr instr;

      const YAML::Node& op_count_range_node = instr_list_node["op_count_range"];
      instr.min_op_count_ = op_count_range_node[0].as<size_t>();
      instr.max_op_count_ = op_count_range_node[1].as<size_t>();

      const YAML::Node& prefetch_node = instr_list_node["prefetch"];
      for (const YAML::Node& instr_node : prefetch_node) {
        AprioriPromoteInstr promote;

        promote.bkt_name_ = instr_node["bucket"].as<std::string>();
        ParseList(promote.promote_, instr_node["promote_blobs"]);
        ParseList(promote.demote_, instr_node["demote_blobs"]);

        instr.promotes_.push_back(promote);
      }

      instr_list.push_back(instr);
    }
  }
  HICLOG(kDebug, PrintSchema());
}

/** check if schema is being correctly entered*/
void AprioriPrefetcher::PrintSchema() {
  for (const auto& rank_pair : rank_info_) {
    int rank = rank_pair.first;
    // const std::vector<AprioriPrefetchInstr>& instr_list = rank_pair.second;
    auto& instr_list = rank_info_[rank];

    std::cout << "Rank: " << rank << std::endl;
    for (const AprioriPrefetchInstr& instr : instr_list) {
      std::cout << "Op Range: [" << instr.min_op_count_ << ", " << instr.max_op_count_ << "]" << std::endl;

      for (const AprioriPromoteInstr& promote_instr : instr.promotes_) {
        std::cout << "Bucket: " << promote_instr.bkt_name_ << std::endl;

        std::cout << "Promote Blobs: ";
        for (const std::string& blob : promote_instr.promote_) {
          std::cout << blob << " ";
        }
        std::cout << std::endl;

        std::cout << "Demote Blobs: ";
        for (const std::string& blob : promote_instr.demote_) {
          std::cout << blob << " ";
        }
        std::cout << std::endl;
      }
    }

    std::cout << std::endl;
  }
}

/** Custom Schema code start */
/** Parsing Thread:File YAML Schema. */
void AprioriPrefetcher::ParseSchema2(const YAML::Node& schema) {
  HILOG(kDebug, "Parsing Schema type-2 \n{}", schema);

  for (const auto &rank_node_pair : schema) {
    const YAML::Node& rank_node = rank_node_pair.first;
    const YAML::Node& rank_instrs = rank_node_pair.second;
    int rank = rank_node.as<int>();
    auto& instr_list = rank_info_[rank];

    for (const YAML::Node& instr_list_node : rank_instrs) {
      const YAML::Node& bucket_node = instr_list_node["bucket"];
      const YAML::Node& prefetch_node = instr_list_node["prefetch"];

      for (const YAML::Node& prefetch_instr_node : prefetch_node) {
        AprioriPrefetchInstr instr;

        const YAML::Node& op_count_range_node = prefetch_instr_node["op_count_range"];
        if (op_count_range_node && !op_count_range_node.IsNull()) {
          instr.min_op_count_ = op_count_range_node[0].as<size_t>();
          instr.max_op_count_ = op_count_range_node[1].as<size_t>();
        }

        const YAML::Node& promote_blobs_node = prefetch_instr_node["promote_blobs"];
        const YAML::Node& demote_blobs_node = prefetch_instr_node["demote_blobs"];

        if (promote_blobs_node && !promote_blobs_node.IsNull() && demote_blobs_node && !demote_blobs_node.IsNull()) {
          for (std::size_t i = 0; i < promote_blobs_node.size(); ++i) {
            AprioriPromoteInstr promote;
            promote.bkt_name_ = bucket_node.as<std::string>();

            promote.promote_.push_back(promote_blobs_node[i].as<std::string>());
            promote.demote_.push_back(demote_blobs_node[i].as<std::string>());

            instr.promotes_.push_back(promote);
          }
        }

        instr_list.push_back(instr);
      }
    }
  }

  HICLOG(kDebug, PrintSchema2());

}

/** check if schema is being correctly entered*/
void AprioriPrefetcher::PrintSchema2() {
  for (const auto& rank_pair : rank_info_) {
    int rank = rank_pair.first;
    // const std::vector<AprioriPrefetchInstr>& instr_list = rank_pair.second;
    auto& instr_list = rank_info_[rank];

    std::cout << "Thread: " << rank << std::endl;

    for (const AprioriPrefetchInstr& instr : instr_list) {
      std::cout << "  Instruction:" << std::endl;
      // std::cout << "    Min Op Count: " << instr.min_op_count_ << std::endl;
      // std::cout << "    Max Op Count: " << instr.max_op_count_ << std::endl;
      std::cout << "Op Range: [" << instr.min_op_count_ << ", " << instr.max_op_count_ << "]" << std::endl;

      for (const AprioriPromoteInstr& promote_instr : instr.promotes_) {
        std::cout << "    Bucket: " << promote_instr.bkt_name_ << std::endl;

        std::cout << "    Promote Blobs: ";
        for (const std::string& blob_name : promote_instr.promote_) {
          std::cout << blob_name << " ";
        }
        std::cout << std::endl;

        std::cout << "    Demote Blobs: ";
        for (const std::string& blob_name : promote_instr.demote_) {
          std::cout << blob_name << " ";
        }
        std::cout << std::endl;
      }
    }
  }
}

// TODO(candice): distinguish between adaptor or native API prefetcher to use this method
template <typename StringT>
std::string get_blob_name(const StringT& blob_page) {
  
  // Retrieve the value of the environment variable
  const char* envValue = std::getenv("HERMES_PAGE_SIZE");
  std::string page_size;

  // Check if the environment variable exists
  if (envValue && *envValue != '\0') {
    // Convert the C-style string to a C++ string
    page_size = envValue;

    // // Log the value
    // HILOG(kDebug, "hermes page_size {}", page_size);
  } else {
    HILOG(kDebug, "HERMES_PAGE_SIZE environment variable not set, default to 1MiB.");
    page_size = "1048576";
  }

  // Convert blob_name
  hermes::adapter::BlobPlacement p;
  p.page_ = std::stoi(blob_page);
  p.page_size_ = std::stoi(page_size);  // Convert page_size to an integer
  std::string blob_name = p.CreateBlobName().str();

  return blob_name;
}

/** Custom Schema code end */


/** Prefetch based on YAML schema */
void AprioriPrefetcher::Prefetch(BufferOrganizer* borg, BinaryLog<IoStat>& log) {
  for (auto& rank_log : rank_info_) {
    size_t rank = rank_log.first;
    size_t num_ops = log.GetRankLogSize(static_cast<int>(rank));
    auto& instr_list = rank_info_[rank];

    // Find the instruction to execute for this rank
    auto begin = instr_list.begin();
    auto cur = begin;
    for (; cur != instr_list.end(); ++cur) {
      auto& instr = *cur;
      if (instr.min_op_count_ <= num_ops && instr.max_op_count_ <= num_ops) {
        HILOG(kDebug, "Matching operation found num_ops {}", num_ops);
        break;
      }
    }

    // First, demote blobs
    if (cur != instr_list.end()) {
      auto& instr = *cur;
      for (auto& promote_instr : instr.promotes_) {
        for (const auto& blob_page : promote_instr.demote_) {
          // get correct blob_name
          std::string blob_name = get_blob_name(blob_page);
          HILOG(kDebug, "Demoting blob {} in bucket {}", blob_name, promote_instr.bkt_name_);
          borg->GlobalOrganizeBlob(promote_instr.bkt_name_, blob_name, 0);
        }
      }

      // Next, promote blobs
      for (auto& promote_instr : instr.promotes_) {
        for (const auto& blob_page : promote_instr.promote_) {
          // get correct blob_name
          std::string blob_name = get_blob_name(blob_page);
          HILOG(kDebug, "Promoting blob {} in bucket {}", blob_name, promote_instr.bkt_name_);
          borg->GlobalOrganizeBlob(promote_instr.bkt_name_, blob_name, 1);
        }
      }

      // Erase the executed instruction from the list
      instr_list.erase(begin, cur + 1);
    }
  }
}

}  // namespace hermes
