/*
 * Simple benchmark that runs a mixture of point lookups and inserts on plex.
 *
 * Examples:
    ./kv_benchmark --key_path=../resources/keyset/fb_1M_uint64_ks_0 --target_db_path=tmp/plex/fb_1M_uint64 --out_path=tmp/out.txt
    ./kv_benchmark --key_path=../resources/keyset/fb_200M_uint64_ks_0 --target_db_path=tmp/plex/fb_200M_uint64 --out_path=tmp/out.txt
 */

#include "bench_utils.h"


// Modify these if running your own workload
#define KEY_TYPE uint64_t
#define VALUE_TYPE uint64_t  // to store rank

double report_t(size_t t_idx, size_t &count_milestone, size_t &last_count_milestone, long long &last_elapsed, std::chrono::time_point<std::chrono::high_resolution_clock> start_t) {
    const double freq_mul = 1.1;
    auto curr_time = std::chrono::high_resolution_clock::now();
    long long time_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(curr_time - start_t).count();
    std::cout
        << "t = "<< time_elapsed << " ns: "
        << t_idx + 1 << " counts, tot "
        << (time_elapsed) / (t_idx + 1.0) << "/op, seg "
        << (time_elapsed - last_elapsed) / (t_idx + 1.0 - last_count_milestone) << "/op"
        << std::endl;
    last_elapsed = time_elapsed;
    last_count_milestone = count_milestone;
    count_milestone = ceil(((double) count_milestone) * freq_mul);  // next milestone to print
    return time_elapsed;
}

/*
 * Required flags:
 * --target_db_path         path to the saved plex
 * --key_path               path to keyset file
 * --out_path               path to save benchmark results
 */
int main(int argc, char* argv[]) {
  auto flags = parse_flags(argc, argv);
  std::string key_path = get_required(flags, "key_path");
  std::string target_db_path = get_required(flags, "target_db_path");
  std::string out_path = get_required(flags, "out_path");
  std::string num_samples_str = get_with_default(flags, "num_samples", "0");  // number of queries
  size_t num_samples = 0;
  std::stringstream(num_samples_str) >> num_samples;

  // Load keyset
  std::vector<uint64_t> queries;
  std::vector<uint64_t> expected_ans;
  size_t count_wrong = 0;
  {
      std::ifstream query_words_in(key_path);
      std::string line;
      while (std::getline(query_words_in, line)) {
          std::istringstream input;
          input.str(line);

          std::string key;
          std::string exp;
          input >> key;
          input >> exp;

          queries.push_back(std::stoull(key));
          expected_ans.push_back(std::stoull(exp));
      }   
  }
  if (num_samples == 0) {
      num_samples = queries.size();
  }
  std::cout << "queries.size()= " << queries.size() << "num_samples= " << num_samples << std::endl;

  // variables for milestone
  size_t last_count_milestone = 0;
  size_t count_milestone = 1;
  long long last_elapsed = 0;
  std::vector<double> timestamps;

  // start timer
  auto start_t = std::chrono::high_resolution_clock::now();

  // Load plex from file
  util::NonOwningMultiMapTS<KEY_TYPE, VALUE_TYPE> index(target_db_path);

  // Issue queries and check answers
  for (size_t t_idx = 0; t_idx < num_samples; t_idx++) {
    // Query key and answer
    uint64_t key = queries[t_idx];
    uint64_t answer = expected_ans[t_idx];  

    // Search
    auto it = index.lower_bound(key);

    // Check with answer
    if (it->second != answer) {
      ++count_wrong;
      // printf("ERROR: incorrect rank: %lu (rcv_key= %lu), expected: %lu (key= %lu)\n", it->second, it->first, answer, key);
    }

    // Step milestone
    if (t_idx + 1 == count_milestone || t_idx + 1 == num_samples) {
      timestamps.push_back(report_t(t_idx, count_milestone, last_count_milestone, last_elapsed, start_t));    
    }
  }
  if (count_wrong > 0) {
    std::cout << "ERROR: there are " << count_wrong << " incorrect ranks" << std::endl;
  }

  // Write result to file
  {
    std::cout << "Writing timestamps to file " << out_path << std::endl;
    std::ofstream file_out;
    file_out.open(out_path, std::ios_base::app);
    for (const auto& timestamp : timestamps) {
        file_out << (long long) timestamp << ",";
    }
    file_out << std::endl;
    file_out.close();   
  }
}
