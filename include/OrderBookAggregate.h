#ifndef ORDERBOOK_ORDERBOOKAGGREGATE_H
#define ORDERBOOK_ORDERBOOKAGGREGATE_H

#include <unordered_map>

#include "Parser.h"
#include "Message.h"
#include "Order.h"
#include "OrderBook.h"

#include <iomanip>
class PriceVolumeCSVLogger {
private:
  std::ofstream _out_file;
  u16 _depth;
public:
  PriceVolumeCSVLogger(const std::string &out_file_name, u16 depth) : _out_file(out_file_name), _depth(depth) {
    if (!_out_file.is_open()) {
      std::cerr << "Unable to open " << out_file_name << std::endl;
      ASSERT(_depth > 0 && _depth < 10, "You probably don't want 0 or >10 depth layer");
    }
  }
  bool good() const {
    return _out_file.good();
  }

  void write_header() {
    _out_file << "ticker,timestamp";
    for (u16 i = 1; i <= _depth; i++)
      _out_file << ",bid_price_" << i << ",bid_vol_" << i;
    for (u16 i = 1; i <= _depth; i++)
      _out_file << ",ask_price_" << i << ",ask_vol_" << i;
    _out_file << "\n";
  }

  void write_row(std::array<i8, 9> ticker, u64 timestamp, const std::map<u32, u64, std::greater<>> &agg_bid, const std::map<u32, u64> &agg_ask) {
    auto sub_secs = timestamp % 1'000'000'000;
    timestamp /= 1'000'000'000;
    auto secs = timestamp % 60;
    timestamp /= 60;
    auto mins = timestamp % 60;
    timestamp /= 60;
    auto hours = timestamp;
    _out_file << std::setw(2) << std::setfill('0') << hours << ":"
              << std::setw(2) << std::setfill('0') << mins << ":"
              << std::setw(2) << std::setfill('0') << secs << "."
              << std::setw(9) << std::setfill('0') << sub_secs << ",";
    for (size_t i = 0; i < 8; i++) {
      if (ticker[i] != ' ') {
        _out_file << ticker[i];
      } else {
        break;
      }
    }
    auto bp = agg_bid.begin();
    for (u16 i = _depth; i--;) {
      if (bp != agg_bid.end()) {
        auto [bid_price, bid_vol] = *bp;
        auto bid_price_fractional = bid_price % 10000 / 100;//stock round to cent
        auto bid_price_decimal = bid_price / 10000;
        _out_file << "," << bid_price_decimal << "." << std::setw(2) << std::setfill('0') << bid_price_fractional << "," << bid_vol;
        bp++;
      } else {
        _out_file << ",,";
      }
    }
    auto ap = agg_ask.begin();
    for (u16 i = _depth; i--;) {
      if (ap != agg_ask.end()) {
        auto [ask_price, ask_vol] = *ap;
        auto ask_price_fractional = ask_price % 10000 / 100;//stock round to cent
        auto ask_price_decimal = ask_price / 10000;
        _out_file << "," << ask_price_decimal << "." << std::setw(2) << std::setfill('0') << ask_price_fractional << "," << ask_vol;
        ap++;
      } else {
        _out_file << ",,";
      }
    }
    _out_file << "\n";
  }
};

class OrderBookAggregatePriceVolumeCSV {
private:
  Parser _parser;
  PriceVolumeCSVLogger _logger;
  std::array<BookAggregate, 1 << 16> _books_aggregate;
  std::unordered_map<u64, Order> _pool;
  std::array<std::array<i8, 9>, 1 << 16> _locate_to_stock;
public:
  OrderBookAggregatePriceVolumeCSV(const std::string &in_file_name, const std::string &out_file_name, u16 depth) :
      _parser(in_file_name), _logger(out_file_name, depth) {}
  void start() {
    if (!_parser.good() || !_logger.good())
      return;
    _logger.write_header();
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
            auto &book_aggregate = _books_aggregate[message._stock_locate];
            Order add_order(message._buy_sell_indicator == 'B' ? BUY : SELL, std::array<i8, 5>{}, message._stock, message._price, message._shares);
            book_aggregate.add_order(add_order);
            ASSERT(!_pool.count(message._order_reference_number), "order existed in the pool");
            _pool[message._order_reference_number] = add_order;
            _logger.write_row(message._stock, message._timestamp, book_aggregate.get_bid(), book_aggregate.get_ask());
          },
          [&](const AddOrderMPIDAttribution &message) {
            auto &book_aggregate = _books_aggregate[message._stock_locate];
            Order add_order(message._buy_sell_indicator == 'B' ? BUY : SELL, message._mp_id, message._stock, message._price, message._shares);
            book_aggregate.add_order(add_order);
            ASSERT(!_pool.count(message._order_reference_number), "order existed in the pool");
            _pool[message._order_reference_number] = add_order;
            _logger.write_row(message._stock, message._timestamp, book_aggregate.get_bid(), book_aggregate.get_ask());
          },
          [&](const OrderExecutedMessage &message) {
            ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
            auto &order = _pool[message._order_reference_number];
            order.exec_shares(message._executed_shares);
            bool del = !order.getShares();
            auto &book_aggregate = _books_aggregate[message._stock_locate];
            if (del) {
              book_aggregate.exec_order<true>(order.getPrice(), order.getSide(), message._executed_shares);
              _pool.erase(message._order_reference_number);
            } else {
              book_aggregate.exec_order<false>(order.getPrice(), order.getSide(), message._executed_shares);
            }
            _logger.write_row(_locate_to_stock[message._stock_locate], message._timestamp, book_aggregate.get_bid(), book_aggregate.get_ask());
          },
          [&](const OrderExecutedWithPriceMessage &message) {
            ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
            auto &order = _pool[message._order_reference_number];
            order.exec_shares(message._executed_shares);
            bool del = !order.getShares();
            auto &book_aggregate = _books_aggregate[message._stock_locate];
            if (del) {
              book_aggregate.exec_order<true>(order.getPrice(), order.getSide(), message._executed_shares);
              _pool.erase(message._order_reference_number);
            } else {
              book_aggregate.exec_order<false>(order.getPrice(), order.getSide(), message._executed_shares);
            }
            _logger.write_row(_locate_to_stock[message._stock_locate], message._timestamp, book_aggregate.get_bid(), book_aggregate.get_ask());
            //TODO accounting should change this to use the new price
          },
          [&](const OrderCancelMessage &message) {
            ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
            auto &order = _pool[message._order_reference_number];
            order.cancel_shares(message._cancelled_shares);
            bool del = !order.getShares();
            auto &book_aggregate = _books_aggregate[message._stock_locate];
            if (del) {
              book_aggregate.cancel_shares<true>(order.getPrice(), order.getSide(), message._cancelled_shares);
              _pool.erase(message._order_reference_number);
            } else {
              book_aggregate.cancel_shares<false>(order.getPrice(), order.getSide(), message._cancelled_shares);
            }
            _logger.write_row(_locate_to_stock[message._stock_locate], message._timestamp, book_aggregate.get_bid(), book_aggregate.get_ask());
          },
          [&](const OrderDeleteMessage &message) {
            ASSERT(_pool.count(message._order_reference_number), "order should exist in the pool");
            auto &order = _pool[message._order_reference_number];
            u32 shares_left = order.getShares();
            auto &book_aggregate = _books_aggregate[message._stock_locate];
            book_aggregate.cancel_shares<true>(order.getPrice(), order.getSide(), shares_left);
            _pool.erase(message._order_reference_number);
            _logger.write_row(_locate_to_stock[message._stock_locate], message._timestamp, book_aggregate.get_bid(), book_aggregate.get_ask());

          },
          [&](const OrderReplaceMessage &message) {
            ASSERT(_pool.count(message._original_order_reference_number), "order should exist in the pool");
            auto &prev_order = _pool[message._original_order_reference_number];
            Order new_order{prev_order.getSide(), prev_order.getMPId(), prev_order.getStock(), message._price, message._shares};
            u32 shares_left = prev_order.getShares();
            auto &book_aggregate = _books_aggregate[message._stock_locate];
            book_aggregate.cancel_shares<true>(prev_order.getPrice(), prev_order.getSide(), shares_left);
            book_aggregate.add_order(new_order);
            ASSERT(!_pool.count(message._new_order_reference_number), "order existed in the pool");
            _pool[message._new_order_reference_number] = new_order;
            _logger.write_row(new_order.getStock(), message._timestamp, book_aggregate.get_bid(), book_aggregate.get_ask());
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

#endif //ORDERBOOK_ORDERBOOKAGGREGATE_H
