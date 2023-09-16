#ifndef ORDERBOOK_PARSER_H
#define ORDERBOOK_PARSER_H

#include <fstream>
#include <iostream>
#include <array>

#include "utils.h"
#include "Message.h"

class Parser {
private:
  std::ifstream _in_file;
  bool _good;
  std::array<i8, 64> _buffer;

public:
  Parser(const std::string &file_name) : _in_file(file_name, std::ios::in | std::ios::binary) {
    if (_in_file.is_open()) {
      _good = true;
    } else {
      std::cerr << "Unable to open " << file_name << std::endl;
      _good = false;
    }
  }

  bool protocol_invariant() const {
    return _good;
  }

  message_type next_message() {

    if (!_in_file.read(_buffer.data(), 3)) {
      return std::monostate{};
    }
    auto message_size = parse_u16(_buffer.data());
    i8 message_id = _buffer[2];
    if (message_id == 'S') {
      _in_file.read(_buffer.data() + 1, 11);
      ASSERT(message_size - 1 == 11, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      i8 event_code = _buffer[11];
      return SystemEventMessage{tracking_number, timestamp, event_code};
    } else if (message_id == 'R') {
      _in_file.read(_buffer.data() + 1, 38);
      ASSERT(message_size - 1 == 38, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      std::array<i8, 9> stock{};
      std::copy(_buffer.data() + 11, _buffer.data() + 19, stock.data());
      i8 market_category = _buffer[19];
      i8 financial_status_indicator = _buffer[20];
      u32 round_lot_size = parse_u32(_buffer.data() + 21);
      i8 round_lots_only = _buffer[25];
      i8 issue_classification = _buffer[26];
      std::array<i8, 2> issue_subtype{_buffer[27], _buffer[28]};
      i8 authenticity = _buffer[29];
      i8 short_sale_threshold_indicator = _buffer[30];
      i8 ipo_flag = _buffer[31];
      i8 luld_reference_tier = _buffer[32];
      i8 etp_flag = _buffer[33];
      u32 etp_leverage_factor = parse_u32(_buffer.data() + 34);
      i8 inverse_indicator = _buffer[38];
      return StockDirectoryMessage{stock_locate, tracking_number, timestamp, stock, market_category, financial_status_indicator, round_lot_size,
                                   round_lots_only, issue_classification, issue_subtype, authenticity, short_sale_threshold_indicator, ipo_flag,
                                   luld_reference_tier, etp_flag, etp_leverage_factor, inverse_indicator};
    }//TODO
    else if (message_id == 'H') {
      _in_file.read(_buffer.data() + 1, 24);
      ASSERT(message_size - 1 == 24, "size");
      return StockTradingActionMessage();
    } else if (message_id == 'Y') {
      _in_file.read(_buffer.data() + 1, 19);
      ASSERT(message_size - 1 == 19, "size");
      return RegSHOShortSalePriceTestRestrictedIndicatorMessage();
    } else if (message_id == 'L') {
      _in_file.read(_buffer.data() + 1, 25);
      ASSERT(message_size - 1 == 25, "size");
      return MarketParticipantPosition();
    } else if (message_id == 'V') {
      _in_file.read(_buffer.data() + 1, 34);
      ASSERT(message_size - 1 == 34, "size");
      return MarketWideCircuitBreakerDeclineLevelMessage();
    } else if (message_id == 'W') {
      _in_file.read(_buffer.data() + 1, 11);
      ASSERT(message_size - 1 == 11, "size");
      return MarketWideCircuitBreakerStatusMessage();
    } else if (message_id == 'K') {
      _in_file.read(_buffer.data() + 1, 27);
      ASSERT(message_size - 1 == 27, "size");
      return QuotingPeriodUpdateMessage();
    } else if (message_id == 'J') {
      _in_file.read(_buffer.data() + 1, 34);
      ASSERT(message_size - 1 == 34, "size");
      return LULDAuctionCollarMessage();
    } else if (message_id == 'h') {
      _in_file.read(_buffer.data() + 1, 20);
      ASSERT(message_size - 1 == 20, "size");
      return OperationalHaltMessage();
    } else if (message_id == 'A') {
      _in_file.read(_buffer.data() + 1, 35);
      ASSERT(message_size - 1 == 35, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      u64 order_reference_number = parse_u64(_buffer.data() + 11);
      i8 buy_sell_indicator = _buffer[19];
      u32 shares = parse_u32(_buffer.data() + 20);
      std::array<i8, 9> stock{};
      std::copy(_buffer.data() + 24, _buffer.data() + 32, stock.data());
      u32 price = parse_u32(_buffer.data() + 32);
      return AddOrderNoMPIDAttribution{stock_locate, tracking_number, timestamp, order_reference_number, buy_sell_indicator, shares, stock, price};
    } else if (message_id == 'F') {
      _in_file.read(_buffer.data() + 1, 39);
      ASSERT(message_size - 1 == 39, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      u64 order_reference_number = parse_u64(_buffer.data() + 11);
      i8 buy_sell_indicator = _buffer[19];
      u32 shares = parse_u32(_buffer.data() + 20);
      std::array<i8, 9> stock{};
      std::copy(_buffer.data() + 24, _buffer.data() + 32, stock.data());
      u32 price = parse_u32(_buffer.data() + 32);
      std::array<i8, 5> mp_id{};
      std::copy(_buffer.data() + 36, _buffer.data() + 40, mp_id.data());
      return AddOrderMPIDAttribution{stock_locate, tracking_number, timestamp, order_reference_number, buy_sell_indicator, shares, stock, price, mp_id};
    } else if (message_id == 'E') {
      _in_file.read(_buffer.data() + 1, 30);
      ASSERT(message_size - 1 == 30, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      u64 order_reference_number = parse_u64(_buffer.data() + 11);
      u32 executed_shares = parse_u32(_buffer.data() + 19);
      u64 match_number = parse_u64(_buffer.data() + 23);
      return OrderExecutedMessage{stock_locate, tracking_number, timestamp, order_reference_number, executed_shares, match_number};
    } else if (message_id == 'C') {
      _in_file.read(_buffer.data() + 1, 35);
      ASSERT(message_size - 1 == 35, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      u64 order_reference_number = parse_u64(_buffer.data() + 11);
      u32 executed_shares = parse_u32(_buffer.data() + 19);
      u64 match_number = parse_u64(_buffer.data() + 23);
      i8 printable = _buffer[31];
      u32 execution_price = parse_u32(_buffer.data() + 32);
      return OrderExecutedWithPriceMessage{stock_locate, tracking_number, timestamp, order_reference_number, executed_shares, match_number, printable,
                                           execution_price};
    } else if (message_id == 'X') {
      _in_file.read(_buffer.data() + 1, 22);
      ASSERT(message_size - 1 == 22, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      u64 order_reference_number = parse_u64(_buffer.data() + 11);
      u32 cancelled_shares = parse_u32(_buffer.data() + 19);
      return OrderCancelMessage{stock_locate, tracking_number, timestamp, order_reference_number, cancelled_shares};
    } else if (message_id == 'D') {
      _in_file.read(_buffer.data() + 1, 18);
      ASSERT(message_size - 1 == 18, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      u64 order_reference_number = parse_u64(_buffer.data() + 11);
      return OrderDeleteMessage{stock_locate, tracking_number, timestamp, order_reference_number};
    } else if (message_id == 'U') {
      _in_file.read(_buffer.data() + 1, 34);
      ASSERT(message_size - 1 == 34, "size");
      u16 stock_locate = parse_u16(_buffer.data() + 1);
      u16 tracking_number = parse_u16(_buffer.data() + 3);
      u64 timestamp = parse_timestamp(_buffer.data() + 5);
      u64 original_order_reference_number = parse_u64(_buffer.data() + 11);
      u64 new_order_reference_number = parse_u64(_buffer.data() + 19);
      u32 shares = parse_u32(_buffer.data() + 27);
      u32 price = parse_u32(_buffer.data() + 31);
      return OrderReplaceMessage{stock_locate, tracking_number, timestamp, original_order_reference_number, new_order_reference_number, shares, price};
    } //TODO
    else if (message_id == 'P') {
      _in_file.read(_buffer.data() + 1, 43);
      ASSERT(message_size - 1 == 43, "size");
      return TradeNonCrossMessage();
    } else if (message_id == 'Q') {
      _in_file.read(_buffer.data() + 1, 39);
      ASSERT(message_size - 1 == 39, "size");
      return TradeCrossMessage();
    } else if (message_id == 'B') {
      _in_file.read(_buffer.data() + 1, 18);
      ASSERT(message_size - 1 == 18, "size");
      return BrokenTradeMessage();
    } else if (message_id == 'I') {
      _in_file.read(_buffer.data() + 1, 49);
      ASSERT(message_size - 1 == 49, "size");
      return NetOrderImbalanceIndicatorMessage();
    } else if (message_id == 'N') {
      _in_file.read(_buffer.data() + 1, 19);
      ASSERT(message_size - 1 == 19, "size");
      return RetailPriceImprovementIndicatorMessage();
    } else if (message_id == 'O') {
      _in_file.read(_buffer.data() + 1, 47);
      ASSERT(message_size - 1 == 47, "size");
      return DirectListingWithCapitalRaisePriceDiscoveryMessage();
    } else {
      _good = false;
      return std::monostate{};
    }
  }

  bool good() const {
    return _in_file.good();
  }
};

#endif //ORDERBOOK_PARSER_H
