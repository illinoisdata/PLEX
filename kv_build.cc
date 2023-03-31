/*
 * Build and save plex to file
 *
 * Examples:
    ./kv_build --keys_file=../resources/data/fb_1M_uint64 --keys_file_type=sosd --total_num_keys=1000000 --db_path=tmp/plex/fb_1M_uint64 --max_error=256
    ./kv_build --keys_file=../resources/data/fb_200M_uint64 --keys_file_type=sosd --total_num_keys=200000000 --db_path=tmp/plex/fb_200M_uint64 --max_error=256
 */

#include <iomanip>

#include "bench_utils.h"

// Modify these if running your own workload
#define KEY_TYPE uint64_t
#define VALUE_TYPE uint64_t

/*
 * Required flags:
 * --keys_file              path to the file that contains keys
 * --keys_file_type         file type of keys_file (options: binary | text | sosd)
 * --total_num_keys         total number of keys in the keys file
 * --db_path                path to save built plex
 * --max_error              PLEX's spline max error
 */
int main(int argc, char* argv[]) {
  auto flags = parse_flags(argc, argv);
  std::string keys_file_path = get_required(flags, "keys_file");
  std::string keys_file_type = get_required(flags, "keys_file_type");
  auto total_num_keys = stoi(get_required(flags, "total_num_keys"));
  std::string db_path = get_required(flags, "db_path");
  size_t max_error = stoi(get_required(flags, "max_error"));
  std::cout << "Using max_error= " << max_error << std::endl;

  // Prepare directory
  if (!fs::is_directory(db_path) || !fs::exists(db_path)) {
    fs::path db_path_p(db_path);
    fs::create_directories(db_path_p.parent_path());
    std::cout << "Created directory " << db_path_p.parent_path() << std::endl;
  }

  // Read keys from file
  auto keys = new KEY_TYPE[total_num_keys];
  if (keys_file_type == "binary") {
    load_binary_data(keys, total_num_keys, keys_file_path);
  } else if (keys_file_type == "text") {
    load_text_data(keys, total_num_keys, keys_file_path);
  } else if (keys_file_type == "sosd") {
    load_sosd_data(keys, total_num_keys, keys_file_path);
  } else {
    std::cerr << "--keys_file_type must be either 'binary' or 'text' or 'sosd'"
              << std::endl;
    return 1;
  }

  // Combine bulk loaded keys with their ranks
  auto elements = std::vector<std::pair<KEY_TYPE, VALUE_TYPE>>(total_num_keys);
  for (int i = 0; i < total_num_keys; i++) {
    if (i % (total_num_keys / 10) == 0) {
      std::cout << "idx= " << i << ": key= " << keys[i] << std::endl;
    }
    elements[i].first = keys[i];
    elements[i].second = i;
  }
  std::sort(elements.begin(), elements.end(),
            [](auto const& a, auto const& b) { return a.first < b.first; });
  delete[] keys;
  std::cout << "Loaded dataset of size " << total_num_keys << std::endl;

  {
    // Create PLEX and bulk load
    auto bulk_load_start_time = std::chrono::high_resolution_clock::now();
    util::NonOwningMultiMapTS<KEY_TYPE, VALUE_TYPE> index(elements, max_error, db_path);
    auto bulk_load_end_time = std::chrono::high_resolution_clock::now();
    auto bulk_load_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
                            bulk_load_end_time - bulk_load_start_time)
                            .count();
    std::cout << "Bulk load completed in " << bulk_load_time / 1e9 << " s" << std::endl;

    // Serialize and save to file
    index.save_to_file(); 
  }

  // Test load plex from file
  {
    util::NonOwningMultiMapTS<KEY_TYPE, VALUE_TYPE> index(db_path);
    std::cout << "Check sum_up of idx= 0, sum= " << index.sum_up(elements[0].first) << std::endl;
    std::cout << "Check sum_up of idx= 10, sum= " << index.sum_up(elements[10].first) << std::endl;
    std::cout << "Check sum_up of idx= 100, sum= " << index.sum_up(elements[100].first) << std::endl;
    std::cout << "Check sum_up of idx= 1000, sum= " << index.sum_up(elements[1000].first) << std::endl;
    std::cout << "Tested loaded from " << db_path << std::endl;
  }
}
