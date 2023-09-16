#ifndef ORDERBOOK_MESSAGE_H
#define ORDERBOOK_MESSAGE_H

#include <array>
#include <variant>
#include "utils.h"

struct SystemEventMessage
{
    u16 _tracking_number;
    u64 _timestamp;
    i8 _event_code;
};

struct StockDirectoryMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 9> _stock;
    i8 _market_category;
    i8 _financial_status_indicator;
    u32 _round_lot_size;
    i8 _round_lots_only;
    i8 _issue_classification;
    std::array<i8, 2> _issue_subtype;
    i8 _authenticity;
    i8 _short_sale_threshold_indicator;
    i8 _ipo_flag;
    i8 _luld_reference_price_tier;
    i8 _etp_flag;
    u32 _etp_leverage_factor;
    i8 _inverse_indicator;
};

struct StockTradingActionMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 9> _stock;
    i8 _trading_state;
    i8 _reserved;
    std::array<i8, 4> _reason;
};

struct RegSHOShortSalePriceTestRestrictedIndicatorMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 9> _stock;
    i8 _reg_sho_action;
};

struct MarketParticipantPosition
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 5> _mp_id;
    std::array<i8, 9> _stock;
    i8 _primary_market_maker;
    i8 _market_maker_mode;
    i8 _market_participant_state;
};

struct MarketWideCircuitBreakerDeclineLevelMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _level_1;
    u64 _level_2;
    u64 _level_3;
};

struct MarketWideCircuitBreakerStatusMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    i8 _breach_level;
};

struct QuotingPeriodUpdateMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 9> _stock;
    u32 _ipo_quotation_release_time;
    i8 ipo_quotation_release_qualifier;
    u32 _ipo_price;
};

struct LULDAuctionCollarMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 9> _stock;
    u32 _auction_collar_reference_price;
    u32 _upper_auction_collar_price;
    u32 _lower_auction_collar_price;
    u32 _auction_collar_extension;
};

struct OperationalHaltMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 9> _stock;
    i8 _market_code;
    i8 _operational_halt_action;
};

struct AddOrderNoMPIDAttribution
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _order_reference_number;
    i8 _buy_sell_indicator;
    u32 _shares;
    std::array<i8, 9> _stock;
    u32 _price;

    //    AddOrderNoMPIDAttribution(u16 stock_locate, u16 tracking_number, u64 timestamp, u64 order_reference_number, u8 buy_sell_indicator, u32 shares
    //                              , std::array<u8, 9> stock, u32 price)
    //            :
    //            _stock_locate(stock_locate)
    //            , _tracking_number(tracking_number)
    //            , _timestamp(timestamp)
    //            , _order_reference_number(order_reference_number)
    //            , _buy_sell_indicator(buy_sell_indicator)
    //            , _shares(shares)
    //            , _stock(stock)
    //            , _price(price)
    //    {};
};

struct AddOrderMPIDAttribution
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _order_reference_number;
    i8 _buy_sell_indicator;
    u32 _shares;
    std::array<i8, 9> _stock;
    u32 _price;
    std::array<i8, 5> _mp_id;
    //    AddOrderMPIDAttribution(u16 stock_locate, u16 tracking_number, u64 timestamp, u64 order_reference_number, u8 buy_sell_indicator, u32 shares
    //    , std::array<u8, 9> stock, u32 price,std::array<u8,5> mp_id)
    //    :
    //    _stock_locate(stock_locate)
    //    , _tracking_number(tracking_number)
    //    , _timestamp(timestamp)
    //    , _order_reference_number(order_reference_number)
    //    , _buy_sell_indicator(buy_sell_indicator)
    //    , _shares(shares)
    //    , _stock(stock)
    //    , _price(price)
    //    , _mp_id(mp_id)
    //    {};
};

struct OrderExecutedMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _order_reference_number;
    u32 _executed_shares;
    u64 _match_number;
};

struct OrderExecutedWithPriceMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _order_reference_number;
    u32 _executed_shares;
    u64 _match_number;
    i8 _printable;
    u32 _execution_price;
};

struct OrderCancelMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _order_reference_number;
    u32 _cancelled_shares;
};

struct OrderDeleteMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _order_reference_number;
};

struct OrderReplaceMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _original_order_reference_number;
    u64 _new_order_reference_number;
    u32 _shares;
    u32 _price;
};

struct TradeNonCrossMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _order_reference_number;
    i8 _buy_sell_indicator;
    u32 _shares;
    std::array<i8, 9> _stock;
    u64 _match_number;
};

struct TradeCrossMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u32 _shares;
    std::array<i8, 9> _stock;
    u32 _cross_price;
    u64 _match_number;
    i8 _cross_type;
};

struct BrokenTradeMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _match_number;
};

struct NetOrderImbalanceIndicatorMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    u64 _paired_shares;
    u64 _imbalance_shares;
    i8 _imbalance_direction;
    std::array<i8, 9> _stock;
    u32 _far_price;
    u32 _near_price;
    u32 _current_reference_price;
    i8 _cross_type;
    i8 _price_variation_indicator;
};

struct RetailPriceImprovementIndicatorMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 9> _stock;
    i8 _interest_flag;
};

struct DirectListingWithCapitalRaisePriceDiscoveryMessage
{
    u16 _stock_locate;
    u16 _tracking_number;
    u64 _timestamp;
    std::array<i8, 9> _stock;
    i8 _open_eligibility_status;
    u32 _minimum_allowable_price;
    u32 _maximum_allowable_price;
    u64 _near_execution_time;
    u32 _lower_price_range_collar;
    u32 _upper_price_range_collar;
};

using message_type = std::variant<SystemEventMessage, StockDirectoryMessage, StockTradingActionMessage, RegSHOShortSalePriceTestRestrictedIndicatorMessage, MarketParticipantPosition, MarketWideCircuitBreakerDeclineLevelMessage, MarketWideCircuitBreakerStatusMessage, QuotingPeriodUpdateMessage, LULDAuctionCollarMessage, OperationalHaltMessage, AddOrderNoMPIDAttribution, AddOrderMPIDAttribution, OrderExecutedMessage, OrderExecutedWithPriceMessage, OrderCancelMessage, OrderDeleteMessage, OrderReplaceMessage, TradeNonCrossMessage, TradeCrossMessage, BrokenTradeMessage, NetOrderImbalanceIndicatorMessage, RetailPriceImprovementIndicatorMessage, DirectListingWithCapitalRaisePriceDiscoveryMessage, std::monostate>;
//enum message_enum
//{
//    SystemEventMessage,
//    StockDirectoryMessage,
//    StockTradingActionMessage,
//    RegSHOShortSalePriceTestRestrictedIndicatorMessage,
//    MarketParticipantPosition,
//    MarketWideCircuitBreakerDeclineLevelMessage,
//    MarketWideCircuitBreakerStatusMessage,
//    QuotingPeriodUpdateMessage,
//    LULDAuctionCollarMessage,
//    OperationalHaltMessage,
//    AddOrderNoMPIDAttribution,
//    AddOrderMPIDAttribution,
//    OrderExecutedMessage,
//    OrderExecutedWithPriceMessage,
//    OrderCancelMessage,
//    OrderDeleteMessage,
//    OrderReplaceMessage,
//    TradeNonCrossMessage,
//    TradeCrossMessage,
//    BrokenTradeMessage,
//    NetOrderImbalanceIndicatorMessage,
//    RetailPriceImprovementIndicatorMessage,
//    DirectListingWithCapitalRaisePriceDiscoveryMessage
//};
#endif //ORDERBOOK_MESSAGE_H
