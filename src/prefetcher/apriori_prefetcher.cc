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


// TODO(candice): add debug commands

namespace hermes {

/** Constructor. Parse YAML schema. */
AprioriPrefetcher::AprioriPrefetcher() {
  auto &path = HERMES->server_config_.prefetcher_.apriori_schema_path_;
  auto real_path = hshm::ConfigParse::ExpandPath(path);
  auto is_mpi = HERMES->server_config_.prefetcher_.is_mpi_;
  HILOG(kDebug, "Start load apriori schema {}", real_path)

  try {
      YAML::Node yaml_conf = YAML::LoadFile(real_path);
      ParseSchema(yaml_conf);

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
// TODO(candice): add code for parsing thread:file schema
/** Custom Schema code end */


/** Prefetch based on YAML schema */
void AprioriPrefetcher::Prefetch(BufferOrganizer* borg, BinaryLog<IoStat>& log) {
  for (auto& rank_log : rank_info_) {
    int rank = rank_log.first;
    size_t num_ops = log.GetRankLogSize(rank);
    // std::vector<AprioriPrefetchInstr>& instr_list = rank_log.second;
    auto &instr_list = rank_info_[rank];

    auto begin = instr_list.begin();
    auto cur = begin;
    for (; cur != instr_list.end(); ++cur) {
      const AprioriPrefetchInstr& instr = *cur;
      if (instr.min_op_count_ <= num_ops && instr.max_op_count_ >= num_ops) {
        break;
      }
    }

    if (cur != instr_list.end()) {
      const AprioriPrefetchInstr& instr = *cur;
      for (const AprioriPromoteInstr& promote_instr : instr.promotes_) {
        for (const std::string& blob_name : promote_instr.demote_) {
          HILOG(kDebug, "Demoting blob {} in bucket {}", blob_name, promote_instr.bkt_name_);

          borg->GlobalOrganizeBlob(promote_instr.bkt_name_, blob_name, 0);
        }
      }

      for (const AprioriPromoteInstr& promote_instr : instr.promotes_) {
        for (const std::string& blob_name : promote_instr.promote_) {
          HILOG(kDebug, "Promoting blob {} in bucket {}", blob_name, promote_instr.bkt_name_);
          borg->GlobalOrganizeBlob(promote_instr.bkt_name_, blob_name, 1);
        }
      }
    }

    instr_list.erase(begin, cur);
  }
}

}  // namespace hermes
