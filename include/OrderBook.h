#ifndef ORDERBOOK_ORDERBOOK_H
#define ORDERBOOK_ORDERBOOK_H

#include <map>
#include <list>

using levels = std::list<Order>;
using levels_ptr = levels::iterator;
using const_levels_ptr = levels::const_iterator;
class BookWithLevel {
  std::map<u32, levels> _asks;
  std::map<u32, levels, std::greater<>> _bids;
public:
  levels_ptr add_order(Order o) {
    if (o.getSide() == BUY) {
      auto &level = _bids[o.getPrice()];
      auto ret = level.insert(level.end(), o);
      ASSERT(check_invariant(), "add buy order > best ask");
      return ret;
    } else {
      auto &level = _asks[o.getPrice()];
      auto ret = level.insert(level.end(), o);
      ASSERT(check_invariant(), "add sell order < best bid");
      return ret;
    }
  }

  template<bool del>
  void exec_order(u32 price, side_type side, u32 shares, levels_ptr ptr) {
    if (del) {
      auto &level = side == BUY ? _bids[price] : _asks[price];
      level.erase(ptr);
      if (level.empty()) {
        if (side == BUY)
          _bids.erase(price);
        else
          _asks.erase(price);
      }
    } else {
      ptr->exec_shares(shares);
    }
  }

  template<bool del>
  void cancel_shares(u32 price, side_type side, u32 shares, levels_ptr ptr) {
    if (del) {
      auto &level = side == BUY ? _bids[price] : _asks[price];
      level.erase(ptr);
      if (level.empty()) {
        if (side == BUY)
          _bids.erase(price);
        else
          _asks.erase(price);
      }
    } else {
      ptr->cancel_shares(shares);
    }
  }
  [[nodiscard]] bool check_invariant() const {
    return _asks.empty() || _bids.empty() || _asks.begin()->first > _bids.begin()->first;
  }

  [[nodiscard]] const std::map<u32, levels, std::greater<>> &get_bid() const {
    return _bids;
  }
  [[nodiscard]] const std::map<u32, levels> &get_ask() const {
    return _asks;
  }
};
class BookAggregate {
  std::map<u32, u64> _asks;
  std::map<u32, u64, std::greater<>> _bids;
public:
  void add_order(Order o) {
    if (o.getSide() == BUY) {
      _bids[o.getPrice()] += o.getShares();
      if (!check_invariant()) {
        ASSERT(false, "add buy order > best ask");
      }
    } else {
      _asks[o.getPrice()] += o.getShares();
      if (!check_invariant()) {
        ASSERT(false, "add sell order < best bid");
      }
    }
  }

  template<bool del>
  void exec_order(u32 price, side_type side, u32 shares) {
    if (side == BUY) {
      _bids[price] -= shares;
    } else {
      _asks[price] -= shares;
    }
    if (del) {
      if (side == BUY) {
        if (!_bids[price])
          _bids.erase(price);
      } else {
        if (!_asks[price])
          _asks.erase(price);
      }
    }
  }

  template<bool del>
  void cancel_shares(u32 price, side_type side, u32 shares) {
    if (side == BUY) {
      _bids[price] -= shares;
    } else {
      _asks[price] -= shares;
    }
    if (del) {
      if (side == BUY) {
        if (!_bids[price])
          _bids.erase(price);
      } else {
        if (!_asks[price])
          _asks.erase(price);
      }
    }
  }

  [[nodiscard]] bool check_invariant() const {
    return _asks.empty() || _bids.empty() || _asks.begin()->first > _bids.begin()->first;
  }

  [[nodiscard]] const std::map<u32, u64, std::greater<>>& get_bid() const {
    return _bids;
  }
  [[nodiscard]] const std::map<u32, u64>& get_ask() const {
    return _asks;
  }
};
#endif //ORDERBOOK_ORDERBOOK_H
