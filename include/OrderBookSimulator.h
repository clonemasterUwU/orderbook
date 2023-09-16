#ifndef ORDERBOOK_INCLUDE_ORDERBOOKSIMULATOR_H
#define ORDERBOOK_INCLUDE_ORDERBOOKSIMULATOR_H

#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include "Parser.h"
#include "Order.h"
#include "OrderBook.h"

const u32 buffer_height = 13, buffer_width = 131,
    bid_x_begin = 1, bid_x_end = 51, bid_y_begin = 8, bid_y_end = 13,
    ask_x_begin = 81, ask_x_end = 131, ask_y_begin = 3, ask_y_end = 8,
    price_y_begin = 61;

class Simulator {
private:
  std::array<std::array<i8, buffer_width>, buffer_height> _buffer;
  std::array<i8, buffer_width> _internal_buffer;
  size_t write_u32(u32 x) {
    size_t begin = buffer_width;
    for (; x >= 10; x /= 10)
      _internal_buffer[--begin] = (i8) ('0' + (x % 10));
    _internal_buffer[--begin] = (i8) ('0' + x);
    return begin;
  }
  size_t write_u64(u64 x) {
    size_t begin = buffer_width;
    for (; x >= 10; x /= 10)
      _internal_buffer[--begin] = (i8) ('0' + (x % 10));
    _internal_buffer[--begin] = (i8) ('0' + x);
    return begin;
  }
  size_t write_ts(u64 timestamp) {
    size_t begin = buffer_width;
    auto sub_secs = timestamp % 1'000'000'000;
    timestamp /= 1'000'000'000;
    auto secs = timestamp % 60;
    timestamp /= 60;
    auto mins = timestamp % 60;
    timestamp /= 60;
    auto hours = timestamp;
    for (int _ = 0; _ < 9; _++)
      _internal_buffer[--begin] = (i8) ('0' + (sub_secs % 10)), sub_secs /= 10;
    _internal_buffer[--begin] = '.';
    for (int _ = 0; _ < 2; _++)
      _internal_buffer[--begin] = (i8) ('0' + (secs % 10)), secs /= 10;
    _internal_buffer[--begin] = ':';
    for (int _ = 0; _ < 2; _++)
      _internal_buffer[--begin] = (i8) ('0' + (mins % 10)), mins /= 10;
    _internal_buffer[--begin] = ':';
    for (int _ = 0; _ < 2; _++)
      _internal_buffer[--begin] = (i8) ('0' + (hours % 10)), hours /= 10;
    return begin;
  }
  size_t write_price(u32 price) {
    size_t begin = buffer_width;
    price /= 100;//stonk rounds to cent
    u32 fractional = price % 100;
    u32 whole = price / 100;
    for (int _ = 0; _ < 2; _++)
      _internal_buffer[--begin] = (i8) ('0' + (fractional % 10)), fractional /= 10;
    _internal_buffer[--begin] = '.';
    for (; whole >= 10; whole /= 10)
      _internal_buffer[--begin] = (i8) ('0' + (whole % 10));
    _internal_buffer[--begin] = (i8) ('0' + whole);
    return begin;
  }
  size_t write_lvl_ask(const_levels_ptr begin, const_levels_ptr end, u64 lvl_depth, u64 onep) {
    u32 len = std::min(lvl_depth / onep + (lvl_depth % onep == 0), (u64) ask_x_end - ask_x_begin);
    size_t res = buffer_width - len;
    std::fill(_internal_buffer.begin() + res, _internal_buffer.end(), '_');
    u64 running_sum = 0;
    for (auto p = begin; p != end; p++) {
      running_sum += p->getShares();
      _internal_buffer[_internal_buffer.size() - 1 - std::min((running_sum / onep), (u64) len - 1)] = '|';
    }
    std::reverse(_internal_buffer.begin() + res, _internal_buffer.end());
    return res;
  }

  size_t write_lvl_bid(const_levels_ptr begin, const_levels_ptr end, u64 lvl_depth, u64 onep) {
    u32 len = std::min(lvl_depth / onep + (lvl_depth % onep == 0), (u64) bid_x_end - bid_x_begin);
    size_t res = buffer_width - len;
    std::fill(_internal_buffer.begin() + res, _internal_buffer.end(), '_');
    u64 running_sum = 0;
    for (auto p = begin; p != end; p++) {
      running_sum += p->getShares();
      _internal_buffer[_internal_buffer.size() - 1 - std::min((running_sum / onep), (u64) len - 1)] = '|';
    }
    return res;
  }
  void print_buffer() {
    const char *colors[buffer_height] = {"\x1b[38;5;220m",//first 3 rows are bright yellow
                                         "\x1b[38;5;220m",
                                         "\x1b[38;5;220m",
                                         "\x1b[38;5;52m",//next 5 rows are dark red to bright red
                                         "\x1b[38;5;88m",
                                         "\x1b[38;5;124m",
                                         "\x1b[38;5;160m",
                                         "\x1b[38;5;196m",
                                         "\x1b[38;5;154m",//next 5 rows are bright green to dark green
                                         "\x1b[38;5;118m",
                                         "\x1b[38;5;112m",
                                         "\x1b[38;5;84m",
                                         "\x1b[38;5;76m"
    };
    std::cout << "\x1B[2J\x1B[H";
    for (size_t row = 0; row < buffer_height; row++) {
      std::cout << colors[row];
      for (auto &ch : _buffer[row])
        std::cout << ch;
      std::cout << "\n";
    }
    std::cout << std::flush;
  }
public:
  Simulator() {};

  Simulator(std::array<i8, 9> ticker) {
    for (auto &row : _buffer)
      std::fill(row.begin(), row.end(), ' ');
    std::copy(ticker.begin(), ticker.end() - 1, _buffer.begin()->data());

  }
  void update(u64 timestamp,
              const std::map<u32, u64, std::greater<>> &agg_bid, const std::map<u32, u64> &agg_ask,
              const std::map<u32, levels, std::greater<>> &lvl_bid, const std::map<u32, levels> &lvl_ask) {
    auto timestamp_begin = write_ts(timestamp);
    std::copy(_internal_buffer.begin() + timestamp_begin, _internal_buffer.end(), (_buffer.begin() + 1)->data());
    u64 total_volume = [&agg_bid, &agg_ask]() {
      u64 vol = 0;
      auto bp = agg_bid.begin();
      for (int _ = 0; _ < 5 && bp != agg_bid.end(); _++, bp++)
        vol += bp->second;
      auto ap = agg_ask.begin();
      for (int _ = 0; _ < 5 && ap != agg_ask.end(); _++, ap++)
        vol += ap->second;
      return vol;
    }();
    for (size_t row = ask_y_begin; row < bid_y_end; row++)
      std::fill(_buffer[row].begin(), _buffer[row].end(), ' ');
    if (total_volume) {
      u64 onep = total_volume / 100;
      auto abp = agg_bid.begin();
      auto lbp = lvl_bid.begin();
      auto aap = agg_ask.begin();
      auto lap = lvl_ask.begin();
      for (size_t ask_row = ask_y_end - 1; ask_row >= ask_y_begin; ask_row--) {
        if (aap != agg_ask.end()) {
          ASSERT(aap->first == lap->first, "internal invariant failed: mismatch ask price");
          auto price_begin = write_price(aap->first);
          std::copy(_internal_buffer.begin() + price_begin, _internal_buffer.end(), _buffer[ask_row].begin() + price_y_begin);
          auto lvl_vol_begin = write_u64(aap->second);
          std::copy(_internal_buffer.begin() + lvl_vol_begin, _internal_buffer.end(),
                    _buffer[ask_row].begin() + ask_x_begin - (_internal_buffer.size() - lvl_vol_begin));
          auto lvl_begin = write_lvl_ask(lap->second.begin(), lap->second.end(), aap->second, onep);
          auto len = _internal_buffer.size() - lvl_begin;
          std::copy(_internal_buffer.begin() + lvl_begin, _internal_buffer.end(), _buffer[ask_row].begin() + ask_x_begin);
          std::fill(_buffer[ask_row].begin() + ask_x_begin + len, _buffer[ask_row].begin() + ask_x_end, ' ');
          aap++, lap++;
        } else {
          break;
        }
      }
      for (size_t bid_row = bid_y_begin; bid_row < bid_y_end; bid_row++) {
        if (abp != agg_bid.end()) {
          ASSERT(abp->first == lbp->first, "internal invariant failed: mismatch bid price");
          auto price_begin = write_price(abp->first);
          std::copy(_internal_buffer.begin() + price_begin, _internal_buffer.end(), _buffer[bid_row].begin() + price_y_begin);
          auto lvl_vol_begin = write_u64(abp->second);
          std::copy(_internal_buffer.begin() + lvl_vol_begin, _internal_buffer.end(), _buffer[bid_row].begin() + bid_x_end);
          auto lvl_begin = write_lvl_bid(lbp->second.begin(), lbp->second.end(), abp->second, onep);
          auto len = _internal_buffer.size() - lvl_begin;
          std::copy(_internal_buffer.begin() + lvl_begin, _internal_buffer.end(), _buffer[bid_row].begin() + bid_x_end - len);
          std::fill(_buffer[bid_row].begin() + bid_x_begin, _buffer[bid_row].begin() + bid_x_end - len, ' ');
          abp++, lbp++;
        } else {
          break;
        }
      }
    }
    print_buffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
};
class OrderBookSimulator {
private:
  Parser _parser;
  BookWithLevel _level_book;
  BookAggregate _aggregate_book;
  u16 _stock_locate;
  std::array<i8, 9> _stock;
  std::unordered_map<u64, std::pair<Order, levels_ptr>> _pool;
  Simulator _logger;
public:
  OrderBookSimulator(const std::string &in_file_name, const std::string &stock) : _parser(in_file_name), _stock_locate(0), _stock() {
    ASSERT(stock.size() < 9, "ticker size < 9");
    std::fill(_stock.begin(), _stock.end() - 1, ' ');
    std::copy(stock.begin(), stock.end(), _stock.begin());
    _logger = Simulator(_stock);
  }
  void start() {
    while (_parser.good()) {
      message_type next_message = _parser.next_message();
      if (!_parser.protocol_invariant()) {
        std::cerr << "corrupted input file" << std::endl;
        break;
      }
      if (!_parser.good()) {
        break;
      }
      next_message | match{//
          [](const SystemEventMessage &message) {},
          [&](const StockDirectoryMessage &message) {
            if (message._stock == _stock)
              _stock_locate = message._stock_locate;
          },
          [](const StockTradingActionMessage &message) {},
          [](const RegSHOShortSalePriceTestRestrictedIndicatorMessage &message) {},
          [](const MarketParticipantPosition &message) {},
          [](const MarketWideCircuitBreakerDeclineLevelMessage &message) {},
          [](const MarketWideCircuitBreakerStatusMessage &message) {},
          [](const QuotingPeriodUpdateMessage &message) {},
          [](const LULDAuctionCollarMessage &message) {},
          [](const OperationalHaltMessage &message) {},
          [&](const AddOrderNoMPIDAttribution &message) {
            if (message._stock_locate == _stock_locate) {
              Order add_order(message._buy_sell_indicator == 'B' ? BUY : SELL, std::array<i8, 5>{}, message._stock, message._price, message._shares);
              _aggregate_book.add_order(add_order);
              auto order_ptr = _level_book.add_order(add_order);
              ASSERT(!_pool.count(message._order_reference_number), "order existed in the pool");
              _pool[message._order_reference_number] = {add_order, order_ptr};
              _logger.update(message._timestamp, _aggregate_book.get_bid(), _aggregate_book.get_ask(), _level_book.get_bid(), _level_book.get_ask());
            }
          },
          [&](const AddOrderMPIDAttribution &message) {
            if (message._stock_locate == _stock_locate) {
              Order add_order(message._buy_sell_indicator == 'B' ? BUY : SELL, message._mp_id, message._stock, message._price, message._shares);
              _aggregate_book.add_order(add_order);
              auto order_ptr = _level_book.add_order(add_order);
              ASSERT(!_pool.count(message._order_reference_number), "order existed in the pool");
              _pool[message._order_reference_number] = {add_order, order_ptr};
              _logger.update(message._timestamp, _aggregate_book.get_bid(), _aggregate_book.get_ask(), _level_book.get_bid(), _level_book.get_ask());
            }
          },
          [&](const OrderExecutedMessage &message) {
            if (message._stock_locate == _stock_locate) {
              ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
              auto &[order, order_ptr] = _pool[message._order_reference_number];
              order.cancel_shares(message._executed_shares);
              bool del = !order.getShares();
              if (del) {
                _level_book.exec_order<true>(order.getPrice(), order.getSide(), message._executed_shares, order_ptr);
                _aggregate_book.exec_order<true>(order.getPrice(), order.getSide(), message._executed_shares);
                _pool.erase(message._order_reference_number);
              } else {
                _level_book.exec_order<false>(order.getPrice(), order.getSide(), message._executed_shares, order_ptr);
                _aggregate_book.exec_order<false>(order.getPrice(), order.getSide(), message._executed_shares);
              }
              _logger.update(message._timestamp, _aggregate_book.get_bid(), _aggregate_book.get_ask(), _level_book.get_bid(), _level_book.get_ask());
            }
          },
          [&](const OrderExecutedWithPriceMessage &message) {
            if (message._stock_locate == _stock_locate) {
              ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
              auto &[order, order_ptr] = _pool[message._order_reference_number];
              order.cancel_shares(message._executed_shares);
              bool del = !order.getShares();
              if (del) {
                _level_book.exec_order<true>(order.getPrice(), order.getSide(), message._executed_shares, order_ptr);
                _aggregate_book.exec_order<true>(order.getPrice(), order.getSide(), message._executed_shares);
                _pool.erase(message._order_reference_number);
              } else {
                _level_book.exec_order<false>(order.getPrice(), order.getSide(), message._executed_shares, order_ptr);
                _aggregate_book.exec_order<false>(order.getPrice(), order.getSide(), message._executed_shares);
              }
              _logger.update(message._timestamp, _aggregate_book.get_bid(), _aggregate_book.get_ask(), _level_book.get_bid(), _level_book.get_ask());
            }
          },
          [&](const OrderCancelMessage &message) {
            if (message._stock_locate == _stock_locate) {
              ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
              auto &[order, order_ptr] = _pool[message._order_reference_number];
              order.cancel_shares(message._cancelled_shares);
              bool del = !order.getShares();
              if (del) {
                _level_book.cancel_shares<true>(order.getPrice(), order.getSide(), message._cancelled_shares, order_ptr);
                _aggregate_book.cancel_shares<true>(order.getPrice(), order.getSide(), message._cancelled_shares);
                _pool.erase(message._order_reference_number);
              } else {
                _level_book.cancel_shares<false>(order.getPrice(), order.getSide(), message._cancelled_shares, order_ptr);
                _aggregate_book.cancel_shares<false>(order.getPrice(), order.getSide(), message._cancelled_shares);
              }
              _logger.update(message._timestamp, _aggregate_book.get_bid(), _aggregate_book.get_ask(), _level_book.get_bid(), _level_book.get_ask());
            }
          },
          [&](const OrderDeleteMessage &message) {
            if (message._stock_locate == _stock_locate) {
              ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
              auto &[order, order_ptr] = _pool[message._order_reference_number];
              u32 shares_left = order.getShares();
              _level_book.cancel_shares<true>(order.getPrice(), order.getSide(), shares_left, order_ptr);
              _aggregate_book.cancel_shares<true>(order.getPrice(), order.getSide(), shares_left);
              _pool.erase(message._order_reference_number);
              _logger.update(message._timestamp, _aggregate_book.get_bid(), _aggregate_book.get_ask(), _level_book.get_bid(), _level_book.get_ask());
            }
          },
          [&](const OrderReplaceMessage &message) {
            if (message._stock_locate == _stock_locate) {
              ASSERT(_pool.count(message._original_order_reference_number), "order should exist in the pool");
              auto &[prev_order, prev_order_ptr] = _pool[message._original_order_reference_number];
              Order new_order{prev_order.getSide(), prev_order.getMPId(), prev_order.getStock(), message._price, message._shares};
              u32 shares_left = prev_order.getShares();
              _level_book.cancel_shares<true>(prev_order.getPrice(), prev_order.getSide(), shares_left, prev_order_ptr);
              _aggregate_book.cancel_shares<true>(prev_order.getPrice(), prev_order.getSide(), shares_left);
              _aggregate_book.add_order(new_order);
              auto order_ptr = _level_book.add_order(new_order);
              ASSERT(!_pool.count(message._new_order_reference_number), "order existed in the pool");
              _pool[message._new_order_reference_number] = {new_order, order_ptr};
              _logger.update(message._timestamp, _aggregate_book.get_bid(), _aggregate_book.get_ask(), _level_book.get_bid(), _level_book.get_ask());
            }
          },
          [](const TradeNonCrossMessage &message) {},
          [](const TradeCrossMessage &message) {},
          [](const NetOrderImbalanceIndicatorMessage &message) {},
          [](const RetailPriceImprovementIndicatorMessage &message) {},
          [](const DirectListingWithCapitalRaisePriceDiscoveryMessage &message) {},
          [](auto whatever) { //
            __builtin_unreachable();
          } //
      };
    }
  }
};
#endif //ORDERBOOK_INCLUDE_ORDERBOOKSIMULATOR_H
