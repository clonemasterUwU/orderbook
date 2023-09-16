#include <string>
#include "OrderBookSimulator.h"

int main() {
    std::string in_file_name = "data/binary/03272019.PSX_ITCH50";
    std::string stock = "MSFT";
    OrderBookSimulator book(in_file_name,stock);
    book.start();
//  std::cout << "\x1b[38;5;220m" << "AAPL\0";
}
