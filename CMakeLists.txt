cmake_minimum_required(VERSION 3.24)
project(orderbook)

set(CMAKE_CXX_STANDARD 20)


include_directories(include)

add_executable(orderbookstat src/OrderBookStat.cpp)
target_compile_options(orderbookstat PRIVATE -g -fsanitize=address -fsanitize=undefined -O2)
target_link_options(orderbookstat PRIVATE -g -fsanitize=address -fsanitize=undefined)

add_executable(orderbookaggregate src/OrderBookAggregate.cpp)
target_compile_options(orderbookaggregate PRIVATE -g -fsanitize=address -fsanitize=undefined -O2)
target_link_options(orderbookaggregate PRIVATE -g -fsanitize=address -fsanitize=undefined)

add_executable(orderbooksimulator src/OrderBookSimulator.cpp)
target_compile_options(orderbooksimulator PRIVATE -g -fsanitize=address -fsanitize=undefined -O2)
target_link_options(orderbooksimulator PRIVATE -g -fsanitize=address -fsanitize=undefined)
