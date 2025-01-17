#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>

#include "include/rs/multi_map.h"
#include "include/ts/builder.h"
#include "include/ts/ts.h"

using namespace std;

namespace ts_manual_tuning {

struct TSConfig {
  size_t spline_max_error;
  size_t num_bins;
  size_t tree_max_error;
};

TSConfig GetTuning(const string& data_filename,
                                   uint32_t size_scale) {
  assert(size_scale >= 1 && size_scale <= 10);

  string dataset = data_filename;

  // Cut the prefix of the filename
  size_t pos = dataset.find_last_of('/');
  if (pos != string::npos) {
    dataset.erase(dataset.begin(), dataset.begin() + pos + 1);
  }

  using Configs = const vector<TSConfig>;
  auto infty = std::numeric_limits<unsigned>::max();

  if (dataset == "books_200M_uint64") {
    Configs configs = {{380, 1u << 12, infty}, {170, 1u << 16, infty}, {110, 1u << 16, infty}, {50, 1u << 20, infty}, {30, 1u << 20, infty}, {20, 1u << 22, infty}, {10, 1u << 22, infty}, {3, 1u << 24, infty}, {3, 1u << 26, infty}, {2, 1u << 28, infty}};
    return configs[10 - size_scale];
  } else if (dataset == "fb_200M_uint64") {
    Configs configs = {{1024, 1024, 16}, {1024, 1024, 16}, {1024, 512, 8},
                {256, 512, 8},    {128, 512, 8},    {16, 128, 16},
                {16, 1024, 16},   {8, 1024, 16},    {4, 256, 16},
                {2, 256, 16}};
    return configs[10 - size_scale];
  } else if (dataset == "osm_cellids_200M_uint64") {
    Configs	configs = {{160, 1u << 20, infty}, {160, 1u << 20, infty}, {160, 1u << 20, infty}, {160, 1u << 20, infty}, {80, 1u << 20, infty}, {40, 1u << 24, infty}, {20, 1u << 24, infty}, {8, 1u << 26, infty}, {3, 1u << 26, infty}, {2, 1u << 28, infty}};
    return configs[10 - size_scale];
  } else if (dataset == "wiki_ts_200M_uint64") {
	  Configs configs = {{100, 1u << 14, infty}, {100, 1u << 14, infty}, {60, 1u << 16, infty}, {20, 1u << 18, infty}, {20, 1u << 20, infty}, {9, 1u << 20, infty}, {5, 1u << 20, infty}, {3, 1u << 22, infty}, {2, 1u << 26, infty}, {1, 1u << 26, infty}};
    return configs[10 - size_scale];
  }

  cerr << "No tuning config for this dataset" << endl;
  throw;
}
};

namespace rs_manual_tuning {

// Returns <num_radix_bits, max_error>
pair<uint64_t, uint64_t> GetTuning(const string& data_filename,
                                   uint32_t size_scale) {
  assert(size_scale >= 1 && size_scale <= 10);

  string dataset = data_filename;

  // Cut the prefix of the filename
  size_t pos = dataset.find_last_of('/');
  if (pos != string::npos) {
    dataset.erase(dataset.begin(), dataset.begin() + pos + 1);
  }

  using Configs = const vector<pair<size_t, size_t>>;

  if (dataset == "normal_200M_uint32") {
    Configs configs = {{10, 6}, {15, 1}, {16, 1}, {18, 1}, {20, 1},
                       {21, 1}, {24, 1}, {25, 1}, {26, 1}, {26, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "normal_200M_uint64") {
    Configs configs = {{14, 2}, {16, 1}, {16, 1}, {20, 1}, {22, 1},
                       {24, 1}, {26, 1}, {26, 1}, {28, 1}, {28, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "lognormal_200M_uint32") {
    Configs configs = {{12, 20}, {16, 3}, {16, 2}, {18, 1}, {20, 1},
                       {22, 1},  {24, 1}, {24, 1}, {26, 1}, {28, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "lognormal_200M_uint64") {
    Configs configs = {{12, 3}, {18, 1}, {18, 1}, {20, 1}, {22, 1},
                       {24, 1}, {26, 1}, {26, 1}, {28, 1}, {28, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "uniform_dense_200M_uint32") {
    Configs configs = {{4, 2},  {16, 2}, {18, 1}, {20, 1}, {20, 1},
                       {22, 2}, {24, 1}, {26, 3}, {26, 3}, {28, 2}};
    return configs[10 - size_scale];
  }

  if (dataset == "uniform_dense_200M_uint64") {
    Configs configs = {{4, 2},  {16, 1}, {16, 1}, {20, 1}, {22, 1},
                       {24, 1}, {24, 1}, {26, 1}, {28, 1}, {28, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "uniform_dense_200M_uint64") {
    Configs configs = {{4, 2},  {16, 1}, {16, 1}, {20, 1}, {22, 1},
                       {24, 1}, {24, 1}, {26, 1}, {28, 1}, {28, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "uniform_sparse_200M_uint32") {
    Configs configs = {{12, 220}, {14, 100}, {14, 80}, {16, 30}, {18, 20},
                       {20, 10},  {20, 8},   {20, 5},  {24, 3},  {26, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "uniform_sparse_200M_uint64") {
    Configs configs = {{12, 150}, {14, 70}, {16, 50}, {18, 20}, {20, 20},
                       {20, 9},   {20, 5},  {24, 3},  {26, 2},  {28, 1}};
    return configs[10 - size_scale];
  }

  // Books (or amazon in the paper)
  if (dataset == "books_200M_uint32") {
    Configs configs = {{14, 250}, {14, 250}, {16, 190}, {18, 80}, {18, 50},
                       {22, 20},  {22, 9},   {22, 8},   {24, 3},  {28, 2}};
    return configs[10 - size_scale];
  }

  if (dataset == "books_200M_uint64") {
    Configs configs = {{12, 380}, {16, 170}, {16, 110}, {20, 50}, {20, 30},
                       {22, 20},  {22, 10},  {24, 3},   {26, 3},  {28, 2}};
    return configs[10 - size_scale];
  }

  if (dataset == "books_400M_uint64") {
    Configs configs = {{16, 220}, {16, 220}, {18, 160}, {20, 60}, {20, 40},
                       {22, 20},  {22, 7},   {26, 3},   {28, 2},  {28, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "books_600M_uint64") {
    Configs configs = {{18, 330}, {18, 330}, {18, 190}, {20, 70}, {22, 50},
                       {22, 20},  {24, 7},   {26, 3},   {28, 2},  {28, 1}};
    return configs[10 - size_scale];
  }

  if (dataset == "books_800M_uint64") {
    Configs configs = {{18, 320}, {18, 320}, {18, 200}, {22, 80}, {22, 60},
                       {22, 20},  {24, 9},   {26, 3},   {28, 3},  {28, 3}};
    return configs[10 - size_scale];
  }

  // Facebook
  if (dataset == "fb_200M_uint64") {
    Configs configs = {{8, 128}, {8, 140}, {8, 140}, {8, 140}, {10, 90},
                       {22, 90}, {24, 70}, {26, 80}, {26, 7},  {28, 80}};
    return configs[10 - size_scale];
  }

  // OSM
  if (dataset == "osm_cellids_200M_uint64") {
    Configs configs = {{20, 160}, {20, 160}, {20, 160}, {20, 160}, {20, 80},
                       {24, 40},  {24, 20},  {26, 8},   {26, 3},   {28, 2}};
    return configs[10 - size_scale];
  }

  if (dataset == "osm_cellids_400M_uint64") {
    Configs configs = {{20, 190}, {20, 190}, {20, 190}, {20, 190}, {22, 80},
                       {24, 20},  {26, 20},  {26, 10},  {28, 6},   {28, 2}};
    return configs[10 - size_scale];
  }

  if (dataset == "osm_cellids_600M_uint64") {
    Configs configs = {{20, 190}, {20, 190}, {20, 190}, {22, 180}, {22, 100},
                       {24, 20},  {26, 20},  {28, 7},   {28, 5},   {28, 2}};
    return configs[10 - size_scale];
  }

  if (dataset == "osm_cellids_800M_uint64") {
    Configs configs = {{22, 190}, {22, 190}, {22, 190}, {22, 190}, {24, 190},
                       {26, 30},  {26, 20},  {28, 7},   {28, 5},   {28, 1}};
    return configs[10 - size_scale];
  }

  // Wiki
  if (dataset == "wiki_ts_200M_uint64") {
    Configs configs = {{14, 100}, {14, 100}, {16, 60}, {18, 20}, {20, 20},
                       {20, 9},   {20, 5},   {22, 3},  {26, 2},  {26, 1}};
    return configs[10 - size_scale];
  }

  cerr << "No tuning config for this dataset" << endl;
  throw;
}
}  // namespace rs_manual_tuning

namespace util {

// Loads values from binary file into vector.
template <typename T>
static vector<T> load_data(const string& filename, bool print = true) {
  vector<T> data;
  ifstream in(filename, ios::binary);
  if (!in.is_open()) {
    cerr << "unable to open " << filename << endl;
    exit(EXIT_FAILURE);
  }
  // Read size.
  uint64_t size;
  in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
  data.resize(size);
  // Read values.
  in.read(reinterpret_cast<char*>(data.data()), size * sizeof(T));

  return data;
}

// Generates deterministic values for keys.
template <class KeyType>
static vector<pair<KeyType, uint64_t>> add_values(const vector<KeyType>& keys) {
  vector<pair<KeyType, uint64_t>> result;
  result.reserve(keys.size());

  for (uint64_t i = 0; i < keys.size(); ++i) {
    pair<KeyType, uint64_t> row;
    row.first = keys[i];
    row.second = i;

    result.push_back(row);
  }
  return result;
}

// }  // namespace util

// namespace {

template <class KeyType, class ValueType>
class NonOwningMultiMapRS {
 public:
  using element_type = pair<KeyType, ValueType>;

  NonOwningMultiMapRS(const vector<element_type>& elements,
                    size_t num_radix_bits = 18, size_t max_error = 32)
      : data_(elements) {
    assert(elements.size() > 0);

    // Create spline builder.
    const auto min_key = data_.front().first;
    const auto max_key = data_.back().first;
    rs::Builder<KeyType> rsb(min_key, max_key, num_radix_bits, max_error);

    // Build the radix spline.
    for (const auto& iter : data_) {
      rsb.AddKey(iter.first);
    }
    rs_ = rsb.Finalize();
  }

  typename vector<element_type>::const_iterator lower_bound(KeyType key) const {
    rs::SearchBound bound = rs_.GetSearchBound(key);
    return ::lower_bound(data_.begin() + bound.begin, data_.begin() + bound.end,
                         key, [](const element_type& lhs, const KeyType& rhs) {
                           return lhs.first < rhs;
                         });
  }

  uint64_t sum_up(KeyType key) const {
    uint64_t result = 0;
    auto iter = lower_bound(key);
    while (iter != data_.end() && iter->first == key) {
      result += iter->second;
      ++iter;
    }
    return result;
  }

  size_t GetSizeInByte() const { return rs_.GetSize(); }

 private:
  const vector<element_type>& data_;
  rs::RadixSpline<KeyType> rs_;
};

template <class KeyType, class ValueType>
class NonOwningMultiMapTS {
 public:
  using element_type = pair<KeyType, ValueType>;

  NonOwningMultiMapTS(const vector<element_type>& elements, size_t max_error, fs::path root_path)
      : data_(elements, root_path / "data"), root_path_(root_path) {
    assert(elements.size() > 0);

    // Create spline builder.
    const auto min_key = data_.front().first;
    const auto max_key = data_.back().first;
    ts::Builder<KeyType> tsb(min_key, max_key, max_error, root_path);

    // Build TS.
    for (const auto& iter : data_) tsb.AddKey(iter.first);
    ts_ = tsb.Finalize();
  }

  typename mmap_struct::LazyVector<element_type>::Iterator lower_bound(KeyType key) const {
    ts::SearchBound bound = ts_.GetSearchBound(key);
    // std::cout << "bound: begin= " << bound.begin << ", end= " << bound.end << std::endl;
    return ::lower_bound(data_.begin() + bound.begin, data_.begin() + bound.end,
                         key, [](const element_type& lhs, const KeyType& rhs) {
                          //  std::cout << "lhs= " << lhs.first << std::endl;
                          //  std::cout << "rhs= " << rhs << std::endl;
                           return lhs.first < rhs;
                         });
  }

  uint64_t sum_up(KeyType key) const {
    uint64_t result = 0;
    auto iter = lower_bound(key);
    while (iter != data_.end() && iter->first == key) {
      result += iter->second;
      ++iter;
    }
    return result;
  }

  size_t GetSizeInByte() const { return ts_.GetSize(); }

  /* Save-load */

  // Save to file
  void save_to_file() const {
    fs::path meta_path = this->make_meta_path();
    std::ofstream ofs(meta_path);
    boost::archive::binary_oarchive oa(ofs);
    oa << (*this);
    std::cout << "Saved NonOwningMultiMapTS to " << meta_path << std::endl; 
  }

  // Load under path
  NonOwningMultiMapTS(fs::path root_path) : ts_(root_path), root_path_(root_path) {
    fs::path meta_path = this->make_meta_path();
    std::ifstream ifs(meta_path);
    boost::archive::binary_iarchive ia(ifs);
    ia >> (*this);
    std::cout << "Loaded NonOwningMultiMapTS from " << meta_path << std::endl;
  }

 private:
  mmap_struct::LazyVector<element_type> data_;
  ts::TrieSpline<KeyType> ts_;
  fs::path root_path_;

  fs::path make_meta_path() const {
    return this->root_path_ / "meta";
  }

  fs::path make_data_path() const {
    return this->root_path_ / "data";
  }
  

  /* Serialization */

  friend class boost::serialization::access;
  template<class Archive>
  void save(Archive & ar, const unsigned int version __attribute__((unused))) const {
    // std::cout << "NonOwningMultiMapTS::save" << std::endl;
    ar << this->data_.size();  // data_size
    ar << this->ts_;
  }

  template<class Archive>
  void load(Archive & ar, const unsigned int version __attribute__((unused))) {
    // std::cout << "NonOwningMultiMapTS::load" << std::endl;
    size_t data_size; ar >> data_size;
    this->data_ = mmap_struct::LazyVector<element_type>(this->make_data_path(), data_size);
    ar >> this->ts_;
  }
  BOOST_SERIALIZATION_SPLIT_MEMBER()
};

template <class KeyType>
struct Lookup {
  KeyType key;
  uint64_t value;
};

template <class KeyType>
void RunRS(const string& data_file, const string lookup_file) {
  // Load data
  vector<KeyType> keys = util::load_data<KeyType>(data_file);
  vector<pair<KeyType, uint64_t>> elements = util::add_values(keys);
  vector<Lookup<KeyType>> lookups =
      util::load_data<Lookup<KeyType>>(lookup_file);

  cout << "index,data_file,spline,radix,size(MB),build(s),lookup" << std::endl;
  for (uint32_t size_config = 1; size_config <= 10; ++size_config) {
    // Get the config for tuning
    auto tuning = rs_manual_tuning::GetTuning(data_file, size_config);

    // Build RS
    auto build_begin = chrono::high_resolution_clock::now();
    NonOwningMultiMapRS<KeyType, uint64_t> map(elements, tuning.first,
                                             tuning.second);
    auto build_end = chrono::high_resolution_clock::now();
    uint64_t build_ns =
        chrono::duration_cast<chrono::nanoseconds>(build_end - build_begin)
            .count();

    // Run queries
    auto lookup_begin = chrono::high_resolution_clock::now();
    for (const Lookup<KeyType>& lookup_iter : lookups) {
      uint64_t sum = map.sum_up(lookup_iter.key);
      if (sum != lookup_iter.value) {
        cerr << "wrong result!" << endl;
        throw "error";
      }
    }
    auto lookup_end = chrono::high_resolution_clock::now();
    uint64_t lookup_ns =
        chrono::duration_cast<chrono::nanoseconds>(lookup_end - lookup_begin)
            .count();

    cout << "RS," << data_file << "," << tuning.second << "," << tuning.first << ","
       << static_cast<double>(map.GetSizeInByte()) / 1000 / 1000 << ","
       << static_cast<double>(build_ns) / 1000 / 1000 / 1000 << ","
       << lookup_ns / lookups.size() << endl;
  }
}

template <class KeyType>
void RunTS(const string& data_file, const string lookup_file) {
  // Load data
  vector<KeyType> keys = util::load_data<KeyType>(data_file);
  vector<pair<KeyType, uint64_t>> elements = util::add_values(keys);
  vector<Lookup<KeyType>> lookups =
      util::load_data<Lookup<KeyType>>(lookup_file);

  cout << "index,data_file,spline,radix,size(MB),build(s),lookup" << std::endl;
  for (uint32_t size_config = 1; size_config <= 10; ++size_config) {
    // Get the config for tuning
    auto tuning = ts_manual_tuning::GetTuning(data_file, size_config);

    // Build TS
    auto build_begin = chrono::high_resolution_clock::now();
    NonOwningMultiMapTS<KeyType, uint64_t> map(elements, tuning.spline_max_error, "/tmp/plex_ts/");
    auto build_end = chrono::high_resolution_clock::now();
    uint64_t build_ns =
        chrono::duration_cast<chrono::nanoseconds>(build_end - build_begin)
            .count();

    // Run queries
    auto lookup_begin = chrono::high_resolution_clock::now();
    for (const Lookup<KeyType>& lookup_iter : lookups) {
      uint64_t sum = map.sum_up(lookup_iter.key);
      if (sum != lookup_iter.value) {
        cerr << "wrong result!" << endl;
        throw "error";
      }
    }
    auto lookup_end = chrono::high_resolution_clock::now();
    uint64_t lookup_ns =
        chrono::duration_cast<chrono::nanoseconds>(lookup_end - lookup_begin)
            .count();

    cout << "TS," << data_file << "," << tuning.spline_max_error << "," << 0 << ","
       << static_cast<double>(map.GetSizeInByte()) / 1000 / 1000 << ","
       << static_cast<double>(build_ns) / 1000 / 1000 / 1000 << ","
       << lookup_ns / lookups.size() << endl;
  }
}

template <class KeyType>
void CustomRunTS(const string& data_file, const string lookup_file, const uint32_t max_error) {
  // Load data
  std::cerr << "Load data.." << std::endl;
  vector<KeyType> keys = util::load_data<KeyType>(data_file);
  vector<pair<KeyType, uint64_t>> elements = util::add_values(keys);
  vector<Lookup<KeyType>> lookups =
      util::load_data<Lookup<KeyType>>(lookup_file);

  // Build index
  std::cerr << "Build index.." << std::endl;
  auto build_begin = chrono::high_resolution_clock::now();
  NonOwningMultiMapTS<KeyType, uint64_t> map(elements, max_error, "/tmp/plex_customts/");
  auto build_end = chrono::high_resolution_clock::now();
  uint64_t build_ns =
      chrono::duration_cast<chrono::nanoseconds>(build_end - build_begin)
          .count();

  // Run queries
  std::cerr << "Run queries.." << std::endl;
  vector<uint64_t> lookup_ns;
  for (uint32_t i = 0; i < 3; i++) {
    auto lookup_begin = chrono::high_resolution_clock::now();
    for (const Lookup<KeyType>& lookup_iter : lookups) {
      uint64_t sum = map.sum_up(lookup_iter.key);
      if (sum != lookup_iter.value) {
        cerr << "wrong result!" << endl;
        throw "error";
      }
    }
    auto lookup_end = chrono::high_resolution_clock::now();
    uint64_t run_lookup_ns =
        chrono::duration_cast<chrono::nanoseconds>(lookup_end - lookup_begin)
            .count();
    lookup_ns.push_back(run_lookup_ns / lookups.size());
  }
  sort(lookup_ns.begin(), lookup_ns.end());

  cout << "TS" << "," << data_file << "," << max_error << ","
       << static_cast<double>(map.GetSizeInByte()) / 1000 / 1000 << ","
       << static_cast<double>(build_ns) / 1000 / 1000 / 1000 << ","
       << lookup_ns[1] << endl;
}

}  // namespace util


std::map<std::string, std::string> parse_flags(int argc, char** argv) {
  std::map<std::string, std::string> flags;
  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    size_t equals = arg.find("=");
    size_t dash = arg.find("--");
    if (dash != 0) {
      std::cout << "Bad flag '" << argv[i] << "'. Expected --key=value"
                << std::endl;
      continue;
    }
    std::string key = arg.substr(2, equals - 2);
    std::string val;
    if (equals == std::string::npos) {
      val = "";
      std::cout << "found flag " << key << std::endl;
    } else {
      val = arg.substr(equals + 1);
      std::cout << "found flag " << key << " = " << val << std::endl;
    }
    flags[key] = val;
  }
  return flags;
}

std::string get_with_default(const std::map<std::string, std::string>& m,
                             const std::string& key,
                             const std::string& defval) {
  auto it = m.find(key);
  if (it == m.end()) {
    return defval;
  }
  return it->second;
}

std::string get_required(const std::map<std::string, std::string>& m,
                         const std::string& key) {
  auto it = m.find(key);
  if (it == m.end()) {
    std::cout << "Required flag --" << key << " was not found" << std::endl;
    throw std::invalid_argument("Missing required flag.");
  }
  return it->second;
}

bool get_boolean_flag(const std::map<std::string, std::string>& m,
                      const std::string& key) {
  return m.find(key) != m.end();
}

std::vector<std::string> get_comma_separated(
    std::map<std::string, std::string>& m, const std::string& key) {
  std::vector<std::string> vals;
  auto it = m.find(key);
  if (it == m.end()) {
    return vals;
  }
  std::istringstream s(m[key]);
  std::string val;
  while (std::getline(s, val, ',')) {
    vals.push_back(val);
    std::cout << "parsed csv val " << val << std::endl;
  }
  return vals;
}

template <class T>
bool load_binary_data(T data[], int length, const std::string& file_path) {
  std::ifstream is(file_path.c_str(), std::ios::binary | std::ios::in);
  if (!is.is_open()) {
    return false;
  }
  is.read(reinterpret_cast<char*>(data), std::streamsize(length * sizeof(T)));
  is.close();
  return true;
}

template <class T>
bool load_text_data(T array[], int length, const std::string& file_path) {
  std::ifstream is(file_path.c_str());
  if (!is.is_open()) {
    return false;
  }
  int i = 0;
  std::string str;
  while (std::getline(is, str) && i < length) {
    std::istringstream ss(str);
    ss >> array[i];
    i++;
  }
  is.close();
  return true;
}

template <class T>
bool load_sosd_data(T data[], int length, const std::string& file_path) {
  std::ifstream is(file_path.c_str(), std::ios::binary | std::ios::in);
  if (!is.is_open()) {
    return false;
  }
  {
    // Read and discard first 8 bytes of SOSD blob (number of key).
    char num_keys[8];
    is.read(&num_keys[0], std::streamsize(sizeof(uint64_t)));
  }
  is.read(reinterpret_cast<char*>(data), std::streamsize(length * sizeof(T)));
  is.close();
  return true;
}
