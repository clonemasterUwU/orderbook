#ifndef ORDERBOOK_UTILS_H
#define ORDERBOOK_UTILS_H

#include <cstdint>
#include <iostream>
#include <variant>
#include <bit>
#include <ranges>
#include <algorithm>
using i8 = char;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i64 = int64_t;

template<std::integral T>
constexpr T byteswap(T value) noexcept {
  static_assert(std::has_unique_object_representations_v<T>,
                "T may not have padding bits");
  auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
  std::ranges::reverse(value_representation);
  return std::bit_cast<T>(value_representation);
}

u16 parse_u16(const i8 *buffer) {
  return (u16) std::bit_cast<u8>(buffer[0]) << 8 | (u16) std::bit_cast<u8>(buffer[1]);
}

u32 parse_u32(const i8 *buffer) {
  return (u32) std::bit_cast<u8>(buffer[0]) << 24 | (u32) std::bit_cast<u8>(buffer[1]) << 16 |
      (u32) std::bit_cast<u8>(buffer[2]) << 8 | (u32) std::bit_cast<u8>(buffer[3]);
}

u64 parse_u64(const i8 *buffer) {
  return (u64) std::bit_cast<u8>(buffer[0]) << 56 | (u64) std::bit_cast<u8>(buffer[1]) << 48 |
      (u64) std::bit_cast<u8>(buffer[2]) << 40 | (u64) std::bit_cast<u8>(buffer[3]) << 32 |
      (u64) std::bit_cast<u8>(buffer[4]) << 24 | (u64) std::bit_cast<u8>(buffer[5]) << 16 |
      (u64) std::bit_cast<u8>(buffer[6]) << 8 | (u64) std::bit_cast<u8>(buffer[7]);
}

//timestamp is 6-byte wide
u64 parse_timestamp(const i8 *buffer) {
  return (u64) std::bit_cast<u8>(buffer[0]) << 40 | (u64) std::bit_cast<u8>(buffer[1]) << 32 |
      (u64) std::bit_cast<u8>(buffer[2]) << 24 | (u64) std::bit_cast<u8>(buffer[3]) << 16 |
      (u64) std::bit_cast<u8>(buffer[4]) << 8 | (u64) std::bit_cast<u8>(buffer[5]);
}

enum side_type {
  BUY,
  SELL
};

template<size_t N>
struct array_u8_hasher {
  std::size_t operator()(std::array<i8, N> const &arr) const {
    std::size_t seed = arr.size();
    for (auto &i : arr) {
      seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
  }
};

template<typename... Fs>
struct match : Fs ... {
  using Fs::operator()...;

};
template<class... Ts> match(Ts...) -> match<Ts...>;

template<typename... Ts, typename... Fs>
constexpr decltype(auto) operator|(std::variant<Ts...> const &v, match<Fs...> const &match) {
  return std::visit(match, v);
}

#ifndef NDEBUG
//debug assert, C++23 std::stack_trace waiting room
#define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << (message) << std::endl; \
            std::abort(); \
        } \
    } while (false)
#else
#define ASSERT(condition, message) do { } while (false)
#endif

#endif //ORDERBOOK_UTILS_H
