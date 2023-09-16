#include <string>
#include "OrderBookStat.h"

#include <sys/resource.h>

const auto stackInfinite = []{
  const rlim_t kStackSize = 1024 * 1024 * 1024;   // min stack size = 1024 MB
  struct rlimit rl;
  int result;
  result = getrlimit(RLIMIT_STACK, &rl);
  if (result == 0)
  {
    if (rl.rlim_cur < kStackSize)
    {
      rl.rlim_cur = kStackSize;
      result = setrlimit(RLIMIT_STACK, &rl);
      if (result != 0)
      {
        fprintf(stderr, "setrlimit returned result = %d\n", result);
      }
    }
  }
  return 0;
}();
int main(int argc,char* argv[]){
  auto print_usage = []() {
    std::cout << "Usage: ./build/orderbookstat [path-to-input-file]" << std::endl;
    exit(1);
  };
  if (argc < 2) {
    print_usage();
  }
  std::string in_file_name(argv[1]);
  OrderBookStat books(in_file_name);
  books.start();
}
