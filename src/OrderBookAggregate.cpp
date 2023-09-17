#include <string>
#include "OrderBookAggregate.h"

#include <sys/resource.h>

const auto stackInfinite = [] {
  const rlim_t kStackSize = 1024 * 1024 * 1024;   // min stack size = 1024 MB
  struct rlimit rl;
  int result;
  result = getrlimit(RLIMIT_STACK, &rl);
  if (result == 0) {
    if (rl.rlim_cur < kStackSize) {
      rl.rlim_cur = kStackSize;
      result = setrlimit(RLIMIT_STACK, &rl);
      if (result != 0) {
        fprintf(stderr, "setrlimit returned result = %d\n", result);
      }
    }
  }
  return 0;
}();
int main(int argc, char *argv[]) {
  auto print_usage = []() {
    std::cout << "Usage: ./build/orderbookaggregate [path-to-input-file] [depth_lvl]" << std::endl;
    exit(1);
  };
  if (argc < 3) {
    print_usage();
  }
  std::string in_file_name(argv[1]), lvl_depth(argv[2]);
  u32 depth;
  try {
    depth = std::stoi(lvl_depth);
  } catch (std::invalid_argument &e) {
    print_usage();
  }
  std::string out_file_name = in_file_name + ".csv";
  OrderBookAggregatePriceVolumeCSV books(in_file_name, out_file_name, depth);
  books.start();

}
