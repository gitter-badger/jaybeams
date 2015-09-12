#ifndef jb_itch5_compute_inside_hpp
#define jb_itch5_compute_inside_hpp

#include <jb/itch5/order_book.hpp>
#include <jb/itch5/add_order_message.hpp>
#include <jb/itch5/add_order_mpid_message.hpp>
#include <jb/itch5/order_cancel_message.hpp>
#include <jb/itch5/order_delete_message.hpp>
#include <jb/itch5/order_executed_message.hpp>
#include <jb/itch5/order_executed_price_message.hpp>
#include <jb/itch5/order_replace_message.hpp>
#include <jb/itch5/stock_directory_message.hpp>

#include <boost/functional/hash.hpp>
#include <chrono>
#include <functional>
#include <unordered_map>

namespace jb {
namespace itch5 {

/**
 * An implementation of jb::message_handler_concept to compute the inside.
 *
 * Keep a collection of all the order books, and forward the right
 * updates to them as it handles the different message types in
 * ITCH-5.0.
 */
class compute_inside {
 public:
  //@{
  /**
   * @name Type traits
   */
  /// Define the clock used to measure processing delays
  typedef std::chrono::steady_clock::time_point time_point;

  /// Callbacks
  typedef std::function<void(
      time_point, stock_t, half_quote const&, half_quote const&)> callback_type;
  //@}

  /// Initialize an empty handler
  compute_inside(callback_type const& callback);

  /// Return the current timestamp for delay measurements
  time_point now() const {
    return std::chrono::steady_clock::now();
  }


  /**
   * Pre-populate the books based on the symbol directory.
   *
   * ITCH-5.0 sends the list of expected securities to be traded on a
   * given day as a sequence of messages.  We use these messages to
   * pre-populate the map of books and avoid hash map updates during
   * the critical path.
   */
  void handle_message(
      time_point recv_ts, long msgcnt, std::size_t msgoffset,
      stock_directory_message const& msg);

  /**
   * Handle a new order.
   */
  void handle_message(
      time_point recv_ts, long msgcnt, std::size_t msgoffset,
      add_order_message const& msg);

  /**
   * Handle a new order with MPID.
   */
  void handle_message(
      time_point recv_ts, long msgcnt, std::size_t msgoffset,
      add_order_mpid_message const& msg) {
    // Delegate on the handler for add_order_message
    handle_message(
        recv_ts, msgcnt, msgoffset,
        static_cast<add_order_message const&>(msg));
  }

  /**
   * Handle am order execution.
   */
  void handle_message(
      time_point recv_ts, long msgcnt, std::size_t msgoffset,
      order_executed_message const& msg);

  /**
   * Handle am order execution.
   */
  void handle_message(
      time_point recv_ts, long msgcnt, std::size_t msgoffset,
      order_executed_price_message const& msg) {
    // Delegate on the handler for add_order_message
    handle_message(
        recv_ts, msgcnt, msgoffset,
        static_cast<order_executed_message const&>(msg));
  }
  
  /**
   * Ignore all other message types.
   *
   * We are only interested in a handful of message types, anything
   * else is captured by this template function and ignored.
   */
  template<typename message_type>
  void handle_message(
      time_point recv_ts, long msgcnt, std::size_t msgoffset,
      message_type const& msg)
  {}

  /**
   * Log any unknown message types.
   */
  void handle_unknown(
      time_point recv_ts, long msgcnt, std::size_t msgoffset,
      char const* msgbuf, std::size_t msglen);

  /**
   * A convenient container for per-order data.
   *
   * Most market data feeds resend the security identifier and side
   * with each order update, but ITCH-5.0 does not.  One needs to
   * lookup that information based on the order id.  This literal type
   * is used to keep that information around.
   */
  struct order_data {
    stock_t stock;
    buy_sell_indicator_t buy_sell_indicator;
    price4_t px;
    int qty;
  };

  /// An accessor to make testing easier
  std::size_t live_order_count() const {
    return orders_.size();
  }

 private:
  /// Store the callback ...
  callback_type callback_;

  typedef std::unordered_map<std::uint64_t, order_data> orders_by_id;
  orders_by_id orders_;

  typedef std::unordered_map<
    stock_t, order_book, boost::hash<stock_t>> books_by_security;
  books_by_security books_;
};

} // namespace itch5
} // namespace jb

#endif // jb_itch5_compute_inside_hpp
