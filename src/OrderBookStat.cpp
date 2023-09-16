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
int main(){
  std::string in_file_name = "data/binary/03272019.PSX_ITCH50";
  OrderBookStat books(in_file_name);
  books.start();
}
