#include "bench_utils.h"

using namespace std;

int main(int argc, char** argv) {
  if ((argc != 4) && (argc != 5)) {
    std::cout << "usage: " << argv[0] << " <data_file> <lookup_file> <index(rs|ts)> [max_error]" << endl;
    exit(-1);
  }

  const string data_file = argv[1];
  const string lookup_file = argv[2];
  const string index_type = argv[3];
  const uint32_t max_error = (argc == 5) ? atoi(argv[4]) : 0;

  if (data_file.find("32") != string::npos) {
    if (index_type == "rs")
      util::RunRS<uint32_t>(data_file, lookup_file);
    else if ((index_type == "ts") && (!max_error))
      util::RunTS<uint32_t>(data_file, lookup_file);
    else
      util::CustomRunTS<uint32_t>(data_file, lookup_file, max_error);
  } else {
    if (index_type == "rs")
      util::RunRS<uint64_t>(data_file, lookup_file);
    else if ((index_type == "ts") && (!max_error))
      util::RunTS<uint64_t>(data_file, lookup_file);
    else
      util::CustomRunTS<uint64_t>(data_file, lookup_file, max_error);
  }
  return 0;
}