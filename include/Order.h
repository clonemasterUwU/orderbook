#ifndef ORDERBOOK_ORDER_H
#define ORDERBOOK_ORDER_H
#include <array>

#include "utils.h"
class Order {

private:
  side_type _side;
  std::array<i8, 5> _mp_id;
  std::array<i8, 9> _stock;
  u32 _price;
  u32 _shares;
public:
  Order() {};
  Order(side_type side, std::array<i8, 5> mp_id, std::array<i8, 9> stock, u32 price, u32 shares)
      : _side(side), _mp_id(mp_id), _stock(stock), _price(price), _shares(shares) {}

  void add_shares(u32 shares) {
    _shares += shares;
  }
  void exec_shares(u32 shares) {
    ASSERT(shares <= _shares, "subtract shares > order shares");
    _shares -= shares;
  }
  void cancel_shares(u32 shares) {
    ASSERT(shares <= _shares, "cancel shares > order shares");
    _shares -= shares;
  }
public:
  side_type getSide() const {
    return _side;
  }

  u32 getPrice() const {
    return _price;
  }

  u32 getShares() const {
    return _shares;
  }

  std::array<i8, 9> getStock() const {
    return _stock;
  }

  std::array<i8, 5> getMPId() const {
    return _mp_id;
  }
};
#endif //ORDERBOOK_ORDER_H
