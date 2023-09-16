#include <string>
#include "OrderBookSimulator.h"

int main(int argc, char *argv[]) {
  auto print_usage = []() {
    std::cout << "Usage: ./build/orderbooksimulator [path-to-input-file] [ticker]" << std::endl;
    exit(1);
  };
  if (argc < 3) {
    print_usage();
  }
  std::string in_file_name(argv[1]),stock(argv[2]);
  OrderBookSimulator book(in_file_name, stock);
  book.start();
}
