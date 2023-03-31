#pragma once

#include <algorithm>
#include <cassert>
#include <optional>
#include <queue>
#include <vector>

#include "common.h"

#include "../mmap_struct.h"

namespace ts_cht {

template <class KeyType>
class CompactHistTree {
 public:
  CompactHistTree() = default;

  CompactHistTree(bool single_layer, KeyType min_key, KeyType max_key, size_t num_keys,
                  size_t num_bins, size_t log_num_bins, size_t max_error,
                  size_t shift, std::vector<unsigned> table)
      : single_layer_(single_layer),
        min_key_(min_key),
        max_key_(max_key),
        num_keys_(num_keys),
        num_bins_(num_bins),
        log_num_bins_(log_num_bins),
        max_error_(max_error),
        shift_(shift),
        table_(std::move(table)) {}

  // Returns a search bound [`begin`, `end`) around the estimated position.
  SearchBound GetSearchBound(const KeyType key) const {
    if (!single_layer_) {
      const size_t begin = Lookup(key);
      // `end` is exclusive.
      const size_t end = (begin + max_error_ + 1 > num_keys_)
                            ? num_keys_
                            : (begin + max_error_ + 1);
      return SearchBound{begin, end};
    } else {
      const KeyType prefix = (key - min_key_) >> shift_;
      assert(prefix + 1 < table_.size());
      const uint32_t begin = table_[prefix];
      const uint32_t end = table_[prefix + 1];
      return SearchBound{begin, end};
    }
  }

  // Returns the size in bytes.
  size_t GetSize() const {
    return sizeof(*this) + table_.size() * sizeof(unsigned);
  }

 private:
  static constexpr unsigned Leaf = (1u << 31);
  static constexpr unsigned Mask = Leaf - 1;

  // Lookup `key` in tree
  size_t Lookup(KeyType key) const {
    key -= min_key_;
    auto width = shift_;
    size_t next = 0;
    do {
      // Get the bin
      KeyType bin = key >> width;
      next = table_[(next << log_num_bins_) + bin];

      // Is it a leaf?
      if (next & Leaf) return next & Mask;

      // Prepare for the next level
      key -= bin << width;
      width -= log_num_bins_;
    } while (true);
  }

  bool single_layer_;
  KeyType min_key_;
  KeyType max_key_;
  size_t num_keys_;
  size_t num_bins_;
  size_t log_num_bins_;
  size_t max_error_;
  size_t shift_;
  
  std::vector<unsigned> table_;


  /* Serialization */

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version __attribute__((unused))) {
    // std::cout << "In CompactHistTree::serialize" << std::endl;
    ar & this->single_layer_;
    ar & this->min_key_;
    ar & this->max_key_;
    ar & this->num_keys_;
    ar & this->num_bins_;
    ar & this->log_num_bins_;
    ar & this->max_error_;
    ar & this->shift_;
    size_t table_size = this->table_.size();
    ar & table_size;
    // std::cout << "table_size= " << table_size << std::endl;
    this->table_.resize(table_size);
    for (size_t idx = 0; idx < table_size; ++idx) {
      ar & this->table_[idx];
    }
  }
};

}  // namespace cht