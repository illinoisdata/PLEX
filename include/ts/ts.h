#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

#include "ts_cht/cht.h"
#include "common.h"

#include "mmap_struct.h"

namespace ts {
// Approximates a cumulative distribution function (CDF) using spline
// interpolation.
template <class KeyType>
class TrieSpline {
 public:
  TrieSpline() = default;

  TrieSpline(fs::path root_path) : root_path_(root_path) {}

  TrieSpline(KeyType min_key, KeyType max_key,
             size_t num_keys, size_t spline_max_error,
             ts_cht::CompactHistTree<KeyType> cht,
             std::vector<ts::Coord<KeyType>> spline_points,
             fs::path root_path)
      : min_key_(min_key),
        max_key_(max_key),
        num_keys_(num_keys),
        spline_max_error_(spline_max_error),
        spline_points_(std::move(spline_points), root_path / "spline_points"),
        cht_(std::move(cht)),
        root_path_(root_path) {}

  // Returns the estimated position of `key`.
  double GetEstimatedPosition(const KeyType key) const {
    // Truncate to data boundaries.
    if (key <= min_key_) return 0;
    if (key >= max_key_) return num_keys_ - 1;

    // Find spline segment with `key` ∈ (spline[index - 1], spline[index]].
    const size_t index = GetSplineSegment(key);
    const Coord<KeyType> down = spline_points_[index - 1];
    const Coord<KeyType> up = spline_points_[index];

    // Compute slope.
    const double x_diff = up.x - down.x;
    const double y_diff = up.y - down.y;
    const double slope = y_diff / x_diff;

    // Interpolate.
    const double key_diff = key - down.x;
    return std::fma(key_diff, slope, down.y);
  }

  // Returns a search bound [begin, end) around the estimated position.
  ts::SearchBound GetSearchBound(const KeyType key) const {
    const size_t estimate = GetEstimatedPosition(key);
    const size_t begin =
        (estimate < spline_max_error_) ? 0 : (estimate - spline_max_error_);
    // `end` is exclusive.
    const size_t end = (estimate + spline_max_error_ + 2 > num_keys_)
                           ? num_keys_
                           : (estimate + spline_max_error_ + 2);
    return ts::SearchBound{begin, end};
  }

  // Returns the size in bytes.
  size_t GetSize() const {
    return sizeof(*this) + cht_.GetSize() +
           spline_points_.size() * sizeof(Coord<KeyType>);
  }

 private:
  // Returns the index of the spline point that marks the end of the spline
  // segment that contains the `key`: `key` ∈ (spline[index - 1], spline[index]]
  size_t GetSplineSegment(const KeyType key) const {
    // Narrow search range using CHT.
    const auto range = cht_.GetSearchBound(key);

    // Linear search?
    if (range.end - range.begin < 32) {
      // Do linear search over narrowed range.
      uint32_t current = range.begin;
      while (spline_points_[current].x < key) ++current;
      return current;
    }

    // Do binary search over narrowed range.
    const auto lb =
        std::lower_bound(spline_points_.begin() + range.begin,
                         spline_points_.begin() + range.end, key,
                         [](const Coord<KeyType>& coord, const KeyType key) {
                           return coord.x < key;
                         });
    return std::distance(spline_points_.begin(), lb);
  }

  KeyType min_key_;
  KeyType max_key_;
  size_t num_keys_;
  size_t spline_max_error_;

  mmap_struct::LazyVector<ts::Coord<KeyType>> spline_points_;
  ts_cht::CompactHistTree<KeyType> cht_;

  fs::path root_path_;


  fs::path make_spline_points_path() const {
    return this->root_path_ / "spline_points";
  }

  /* Serialization */

  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version __attribute__((unused))) const {
    // std::cout << "TrieSpline::save" << std::endl;
    ar << this->min_key_;
    ar << this->max_key_;
    ar << this->num_keys_;
    ar << this->spline_max_error_;
    ar << this->cht_;

    ar << this->spline_points_.size();  // data_size
  }

  template<class Archive>
  void load(Archive & ar, const unsigned int version __attribute__((unused))) {
    // std::cout << "TrieSpline::load" << std::endl;
    ar >> this->min_key_;
    ar >> this->max_key_;
    ar >> this->num_keys_;
    ar >> this->spline_max_error_;
    ar >> this->cht_;

    size_t data_size; ar >> data_size;
    this->spline_points_ = mmap_struct::LazyVector<ts::Coord<KeyType>>(this->make_spline_points_path(), data_size);
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

}  // namespace ts
