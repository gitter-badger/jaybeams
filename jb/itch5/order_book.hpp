#ifndef jb_itch5_order_book_hpp
#define jb_itch5_order_book_hpp

#include <jb/itch5/price_field.hpp>
#include <jb/itch5/buy_sell_indicator.hpp>
#include <jb/log.hpp>

#include <functional>
#include <map>
#include <utility>

namespace jb {
namespace itch5 {

/// A simple represetation for price + quantity
typedef std::pair<price4_t, int> half_quote;

/**
 * Maintain the ITCH-5.0 order book for a single security.
 *
 * ITCH-5.0, like other market data feeds provide order-by-order
 * detail, that is, the feed includes a message for each order
 * received by the exchange, as well as the changes to these 
 * orders, i.e. when the execute, when their quantity (and/or price)
 * is modified, and when they are canceled..  Such feeds are sometimes
 * called Level III feeds.  Typically only orders that do not
 * immediately execute in full are included in the feed.
 *
 * There is substantial variation in the format of the messages, and
 * some variation as to whether executions are represented differently
 * than a partial cancel, and whether changes in price are allowed or
 * create new order ids.
 *
 * This class receives a stream of (unnormalized) ITCH-5.0 messages
 * for a single security, and organizes the orders in a
 * book, i.e. a  data structure where orders at the same price are
 * grouped together, and one can quickly ask:
 *
 * - What is the best bid (highest price of BUY orders) and what is
 * the total quantity available at that price?
 * - What is the best offer (lowest price of SELL orders) and what is
 * the total quantity available at that price?
 */
class order_book {
 public:
  /// Initialize an empty order book.
  order_book() {}

  /// Return the best bid price and quantity
  half_quote best_bid() const;
  /// Return the best offer price and quantity
  half_quote best_offer() const;

  /// The value used to represent an empty bid
  static half_quote empty_bid() {
    return half_quote(price4_t(0), 0);
  }
  /// The value used to represent an empty offer
  static half_quote empty_offer() {
    return half_quote(max_price_field_value<price4_t>(), 0);
  }


  /**
   * Handle a new order.
   *
   * Update the quantity at the right price level in the correct
   * side of the book.
   *
   * @param side whether the order is a buy or a sell
   * @param px the price of the order
   * @param qty the quantity of the order
   * @return true if the inside changed
   */
  bool handle_add_order(buy_sell_indicator_t side, price4_t px, int qty) {
    if (side == buy_sell_indicator_t('B')) {
      return handle_add_order(buy_, px, qty);
    }
    return handle_add_order(sell_, px, qty);

  }

  /**
   * Handle an order reduction, which includes executions, cancels and replaces.
   *
   * @param side whether the order is a buy or a sell
   * @param px the price of the order
   * @param reduced_qty the executed quantity of the order
   * @returns true if the inside changed
   */
  bool handle_order_reduced(
      buy_sell_indicator_t side, price4_t px, int reduced_qty) {
    if (side == buy_sell_indicator_t('B')) {
      return handle_order_reduced(buy_, px, reduced_qty);
    }
    return handle_order_reduced(sell_, px, reduced_qty);
  }

 private:
  /**
   * Refactor handle_add_order()
   *
   * @tparam book_side the type used to represent the buy or sell side
   * of the book
   * @param side the side of the book to update
   * @param px the price of the new order
   * @param qty the quantity of the new order
   * @returns true if the inside changed
   */
  template<typename book_side>
  bool handle_add_order(book_side& side, price4_t px, int qty) {
      auto p = side.emplace(px, 0);
      p.first->second += qty;
      return p.first == side.begin();
  }
  
  /**
   * Refactor handle_order_reduce()
   *
   * @tparam book_side the type used to represent the buy or sell side
   * of the book
   * @param side the side of the book to update
   * @param px the price of the order that was reduced
   * @param reduced_qty the quantity reduced in the order
   * @returns true if the inside changed
   */
  template<typename book_side>
  bool handle_order_reduced(book_side& side, price4_t px, int reduced_qty) {
    // Find the price level
    // TODO() we create the price level if it does not exist, that is
    // fairly inneficient ..
    auto p = side.emplace(px, 0);
    // ... reduce the quantity ...
    p.first->second -= reduced_qty;
    if (p.first->second < 0) {
      // ... this is "Not Good[tm]", somehow we missed an order or
      // processed a delete twice ...
      // TODO() need a lot more details here...
      JB_LOG(warning) << "negative quantity in order book";
    }
    // ... we may erase the element and invalidate the iterator, so
    // compute if this was at the inside or not ...
    bool r = p.first == side.begin();
    if (p.first->second <= 0) {
      side.erase(p.first);
    }
    return r;
  }
  
 private:
  typedef std::map<price4_t,int,std::greater<price4_t>> buys;
  typedef std::map<price4_t,int,std::less<price4_t>> sells;

  buys buy_;
  sells sell_;
};

} // namespace itch5
} // namespace jb

#endif // jb_itch5_order_book_hpp