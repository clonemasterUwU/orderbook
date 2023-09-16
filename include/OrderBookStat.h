#ifndef ORDERBOOK_INCLUDE_ORDERBOOKSTAT_H
#define ORDERBOOK_INCLUDE_ORDERBOOKSTAT_H
#include <unordered_map>

#include "Parser.h"
#include "Message.h"
#include "Order.h"
#include "OrderBook.h"

#include <iomanip>

class Stat {
private:
  u64 _cnt;
  i64 _mean;
  u64 _std;
  i64 _k;
public:
  Stat() : _cnt(0), _mean(0), _std(0) {};
  void add_x(u32 x, u64 freq = 1) {
    if (_cnt == 0) {
      _k = x;
    }
    i64 kx = ((i64) x - _k);
    _cnt += freq;
    _mean += kx * freq;
    _std += kx * kx * freq;
  }
  std::tuple<u64, double, double> get_stat() {
    if (_cnt) {
      return {_cnt, (double) _mean / _cnt + _k, (_std - (double) _mean / _cnt * _mean) / (_cnt - 1)};
    } else {
      return {0, 0.0, 0.0};
    }
  }
};

class OrderBookStat {
private:
  Parser _parser;
  std::unordered_map<u64, Order> _pool;
  std::array<std::array<i8, 9>, 1 << 16> _locate_to_stock;
  std::array<Stat, 1 << 16> add_stats, executed_stats, cancel_or_delete_stats, executed_price_stats;
  //print to std output TODO:change
  void print_result() {
    std::cout
        << "| Ticker | add cnt | add mean | add std | exec cnt | exec mean | exec std | cancel cnt | cancel mean | cancel std | exec shares | exec price mean | exec price std |\n";
    for (size_t i = 0; i < 1 << 16; i++) {
      if (!_locate_to_stock[i][0])
        continue;

      std::cout << "|";
      for (size_t j = 0; j < 8; j++) {
        std::cout << _locate_to_stock[i][j];
      }
      auto [add_cnt, add_mean, add_std] = add_stats[i].get_stat();
      std::cout << "|" << std::setw(9) << add_cnt << "|" << std::setw(10) << std::fixed << std::setprecision(2) << add_mean << "|" << std::setw(9) << std::fixed
                << std::setprecision(2) << add_std;
      auto [exec_cnt, exec_mean, exec_std] = executed_stats[i].get_stat();
      std::cout << "|" << std::setw(10) << exec_cnt << "|" << std::setw(11) << std::fixed << std::setprecision(2) << exec_mean << "|" << std::setw(10)
                << std::fixed << std::setprecision(2) << exec_std;
      auto [cancel_cnt, cancel_mean, cancel_std] = cancel_or_delete_stats[i].get_stat();
      std::cout << "|" << std::setw(12) << cancel_cnt << "|" << std::setw(13) << std::fixed << std::setprecision(2) << cancel_mean << "|" << std::setw(12)
                << std::fixed << std::setprecision(2) << cancel_std;
      auto [exec_price_cnt, exec_price_mean, exec_price_std] = executed_price_stats[i].get_stat();
      std::cout << "|" << std::setw(12) << exec_price_cnt << "|" << std::setw(17) << std::fixed << std::setprecision(2) << exec_price_mean / 1e4 << "|"
                << std::setw(12) << std::fixed << std::setprecision(2) << exec_price_std / 1e8;
      std::cout << "|\n";
    }
  }
public:
  OrderBookStat(const std::string &in_file_name) : _parser(in_file_name) {}
  void start() {
    if (!_parser.good())
      return;
    while (_parser.good()) {
      message_type next_message = _parser.next_message();
      if (!_parser.protocol_invariant()) {
        std::cerr << "corrupted input file" << std::endl;
        return;
      }
      if (!_parser.good()) {
        break;
      }
      next_message | match{//
          [](const SystemEventMessage &message) {},
          [&](const StockDirectoryMessage &message) {
            _locate_to_stock[message._stock_locate] = message._stock;
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
            Order add_order(message._buy_sell_indicator == 'B' ? BUY : SELL, std::array<i8, 5>{}, message._stock, message._price, message._shares);
            _pool[message._order_reference_number] = add_order;
            add_stats[message._stock_locate].add_x(message._shares);
          },
          [&](const AddOrderMPIDAttribution &message) {
            Order add_order(message._buy_sell_indicator == 'B' ? BUY : SELL, message._mp_id, message._stock, message._price, message._shares);
            _pool[message._order_reference_number] = add_order;
            add_stats[message._stock_locate].add_x(message._shares);
          },
          [&](const OrderExecutedMessage &message) {
            ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
            auto &order = _pool[message._order_reference_number];
            order.exec_shares(message._executed_shares);
            executed_stats[message._stock_locate].add_x(message._executed_shares);
            executed_price_stats[message._stock_locate].add_x(order.getPrice(), message._executed_shares);
            bool del = !order.getShares();
            if (del) {
              _pool.erase(message._order_reference_number);
            }
          },
          [&](const OrderExecutedWithPriceMessage &message) {
            ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
            auto &order = _pool[message._order_reference_number];
            order.cancel_shares(message._executed_shares);
            bool del = !order.getShares();
            executed_stats[message._stock_locate].add_x(message._executed_shares);
            executed_price_stats[message._stock_locate].add_x(message._execution_price, message._executed_shares);
            if (del) {
              _pool.erase(message._order_reference_number);
            }
            //TODO accounting should change this to use the new price
          },
          [&](const OrderCancelMessage &message) {
            ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
            auto &order = _pool[message._order_reference_number];
            order.cancel_shares(message._cancelled_shares);
            cancel_or_delete_stats[message._stock_locate].add_x(message._cancelled_shares);
            bool del = !order.getShares();
            if (del) {
              _pool.erase(message._order_reference_number);
            }
          },
          [&](const OrderDeleteMessage &message) {
            ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
            auto &order = _pool[message._order_reference_number];
            u32 shares_left = order.getShares();
            cancel_or_delete_stats[message._stock_locate].add_x(shares_left);
            _pool.erase(message._order_reference_number);
          },
          [&](const OrderReplaceMessage &message) {
            ASSERT(_pool.count(message._original_order_reference_number), "order should exist in the pool");
            auto &prev_order = _pool[message._original_order_reference_number];
            Order new_order{prev_order.getSide(), prev_order.getMPId(), prev_order.getStock(), message._price, message._shares};
            u32 shares_left = prev_order.getShares();
            cancel_or_delete_stats[message._stock_locate].add_x(shares_left);
            ASSERT(!_pool.count(message._new_order_reference_number), "order existed in the pool");
            _pool[message._new_order_reference_number] = new_order;
            add_stats[message._stock_locate].add_x(message._shares);
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
    print_result();
  }
};

#endif //ORDERBOOK_INCLUDE_ORDERBOOKSTAT_H
