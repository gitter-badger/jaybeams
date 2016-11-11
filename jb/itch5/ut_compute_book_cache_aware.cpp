#include <jb/itch5/compute_book_cache_aware.hpp>
#include <jb/as_hhmmss.hpp>

#include <skye/mock_function.hpp>
#include <boost/test/unit_test.hpp>

using namespace jb::itch5;

namespace {

buy_sell_indicator_t const BUY(u'B');
buy_sell_indicator_t const SELL(u'S');

/// Create a simple timestamp
timestamp create_timestamp() {
  return timestamp{std::chrono::nanoseconds(0)};
}

/// Create a stock_directory_message for testing
stock_directory_message create_stock_directory(char const* symbol) {
  return stock_directory_message{
      {stock_directory_message::message_type, 0, 0, create_timestamp()},
      stock_t(symbol),
      market_category_t(u'Q'),
      financial_status_indicator_t(u'N'),
      100,
      roundlots_only_t('N'),
      issue_classification_t(u'C'),
      issue_subtype_t("C"),
      authenticity_t(u'P'),
      short_sale_threshold_indicator_t(u' '),
      ipo_flag_t(u'N'),
      luld_reference_price_tier_t(u' '),
      etp_flag_t(u'N'),
      0,
      inverse_indicator_t(u'N')};
}

} // anonymous namespace

/**
 * @test Verify that jb::itch5::compute_book_cache_aware works as expected.
 */
BOOST_AUTO_TEST_CASE(compute_book_cache_aware_simple) {
  // We are going to use a mock function to handle the callback
  // because it is easy to test what values they got ...
  skye::mock_function<void(
      compute_book_cache_aware::time_point, stock_t, tick_t, int)>
      callback;

  // ... create a callback that holds a reference to the mock
  // function, because the handler keeps the callback by value.  Also,
  // ignore the header, because it is tedious to test for it ...
  auto cb = [&callback](
      compute_book_cache_aware::time_point recv_ts, stock_t const& stock,
      tick_t tick,
      int price_levels) { callback(recv_ts, stock, tick, price_levels); };

  // ... create the object under testing ...
  compute_book_cache_aware tested(cb);

  // ... we do not expect any callbacks ...
  callback.check_called().never();

  // ... send a couple of stock directory messages, do not much care
  // about their contents other than the symbol ...
  compute_book_cache_aware::time_point now = tested.now();
  long msgcnt = 0;
  tested.handle_message(now, ++msgcnt, 0, create_stock_directory("HSART"));
  tested.handle_message(now, ++msgcnt, 0, create_stock_directory("FOO"));
  tested.handle_message(now, ++msgcnt, 0, create_stock_directory("BAR"));
  // ... duplicates should not create a problem ...
  tested.handle_message(now, ++msgcnt, 0, create_stock_directory("HSART"));
  callback.check_called().never();

  // ... handle a new order ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          2,
                                          BUY,
                                          100,
                                          stock_t("HSART"),
                                          price4_t(100000)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // new price on the book

  // ... handle a new order on the opposite side of the book ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          3,
                                          SELL,
                                          100,
                                          stock_t("HSART"),
                                          price4_t(100100)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // new price on the book

  // ... handle a new order with an mpid ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      add_order_mpid_message{
          add_order_message{
              {add_order_mpid_message::message_type, 0, 0, create_timestamp()},
              4,
              SELL,
              500,
              stock_t("HSART"),
              price4_t(100000)},
          mpid_t("LOOF")});
  // ... updates the book just like a regular order ...
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 1, 0);

  // ... handle a partial execution ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_executed_message{
          {add_order_mpid_message::message_type, 0, 0, create_timestamp()},
          4,
          100,
          123456});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // still 400 remaining on that price

  // ... handle a full execution ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_executed_message{
          {add_order_mpid_message::message_type, 0, 0, create_timestamp()},
          3,
          100,
          123457});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0, 0);
  BOOST_CHECK_EQUAL(tested.live_order_count(), 2);

  // ... handle a partial execution with price ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_executed_price_message{
          order_executed_message{{order_executed_price_message::message_type, 0,
                                  0, create_timestamp()},
                                 4,
                                 100,
                                 123456},
          printable_t{u'Y'}, price4_t(100150)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // still 300 remaining on that price
  BOOST_CHECK_EQUAL(tested.live_order_count(), 2);

  // ... create yet another order ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          5,
                                          BUY,
                                          1000,
                                          stock_t("HSART"),
                                          price4_t(100100)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 1, 0);
  BOOST_CHECK_EQUAL(tested.live_order_count(), 3);

  // ... partially cancel the order ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_cancel_message{
          {order_cancel_message::message_type, 0, 0, create_timestamp()},
          5,
          200});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // still 800 remaining on that price

  // ... fully cancel the order ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_delete_message{
          {order_delete_message::message_type, 0, 0, create_timestamp()}, 5});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 1, 0);

  // test the tail now
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          70,
                                          SELL,
                                          300,
                                          stock_t("HSART"),
                                          price4_t(900000)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // new price on the book

  // ... handle a full execution ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_executed_message{
          {add_order_mpid_message::message_type, 0, 0, create_timestamp()},
          4,
          300,
          123457});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 8000, 1);

  // ... handle a new order, new price ... buy side now
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          6,
                                          BUY,
                                          100,
                                          stock_t("HSART"),
                                          price4_t(1000000)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 9000,
      1); // new price on the book
}

/**
 * @test Verify that jb::itch5::compute_book_cache_aware works as expected for
 * replace.
 *
 * Order replaces have several scenarios, the previous test was
 * getting too big.
 */
BOOST_AUTO_TEST_CASE(compute_book_cache_aware_replace) {
  // We are going to use a mock function to handle the callback
  // because it is easy to test what values they got ...
  skye::mock_function<void(
      compute_book_cache_aware::time_point, stock_t, tick_t, int)>
      callback;

  // ... create a callback that holds a reference to the mock
  // function, because the handler keeps the callback by value.  Also,
  // ignore the header, because it is tedious to test for it ...
  auto cb = [&callback](
      compute_book_cache_aware::time_point recv_ts, stock_t const& stock,
      tick_t tick,
      int price_levels) { callback(recv_ts, stock, tick, price_levels); };
  // ... create the object under testing ...
  compute_book_cache_aware tested(cb);

  // ... setup the book with orders on both sides ...
  auto now = tested.now();
  int msgcnt = 0;
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          1,
                                          BUY,
                                          500,
                                          stock_t("HSART"),
                                          price4_t(100000)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // new price
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          2,
                                          SELL,
                                          500,
                                          stock_t("HSART"),
                                          price4_t(100500)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // new price

  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          10,
                                          BUY,
                                          500,
                                          stock_t("HSART"),
                                          price4_t(99900)});
  callback.check_called().at_least(2).with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // new price
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          20,
                                          SELL,
                                          500,
                                          stock_t("HSART"),
                                          price4_t(100600)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 0,
      0); // new price

  // ... handle a replace message that improves the price ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_replace_message{
          {order_replace_message::message_type, 0, 0, create_timestamp()},
          1,
          3,
          600,
          price4_t(100100)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 3,
      0); // 1 down + 2 up

  // ... handle a replace message that improves the price ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_replace_message{
          {order_replace_message::message_type, 0, 0, create_timestamp()},
          2,
          30,
          600,
          price4_t(100400)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 3,
      0); // 1 down + 2 up

  // ... handle a replace that changes the qty ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_replace_message{
          {order_replace_message::message_type, 0, 0, create_timestamp()},
          30,
          40,
          300,
          price4_t(100400)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 4,
      0); // 2 down to 100600 and back

  // ... handle a replace that changes lowers the best price ...
  now = tested.now();
  tested.handle_message(
      now, ++msgcnt, 0,
      order_replace_message{
          {order_replace_message::message_type, 0, 0, create_timestamp()},
          3,
          9,
          400,
          price4_t(99800)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("HSART"), 2,
      0); // new price (99900)
}

/**
 * @test Improve code coverage for edge cases.
 */
BOOST_AUTO_TEST_CASE(compute_book_cache_aware_edge_cases) {
  // We are going to use a mock function to handle the callback
  // because it is easy to test what values they got ...
  skye::mock_function<void(
      compute_book_cache_aware::time_point, stock_t, tick_t, int)>
      callback;

  // ... create a callback that holds a reference to the mock
  // function, because the handler keeps the callback by value.  Also,
  // ignore the header, because it is tedious to test for it ...
  auto cb = [&callback](
      compute_book_cache_aware::time_point recv_ts, stock_t const& stock,
      tick_t tick,
      int price_levels) { callback(recv_ts, stock, tick, price_levels); };
  // ... create the object under testing ...
  compute_book_cache_aware tested(cb);

  // ... force an execution on a non-existing order ...
  // an exception is triggered
  auto now = tested.now();
  int msgcnt = 0;
  tested.handle_message(
      now, ++msgcnt, 0,
      order_executed_message{
          {add_order_mpid_message::message_type, 0, 0, create_timestamp()},
          1000,
          100,
          123456});
  callback.check_called().never();

  // ... improve code coverage for unknown messages ...
  now = tested.now();
  char const unknownbuf[] = "foobarbaz";
  tested.handle_unknown(
      now, jb::itch5::unknown_message(
               ++msgcnt, 0, sizeof(unknownbuf) - 1, unknownbuf));
  callback.check_called().never();

  // ... a completely new symbol might be slow, but should work ...
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          2000,
                                          BUY,
                                          500,
                                          stock_t("CRAZY"),
                                          price4_t(150000)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("CRAZY"), 0,
      0); // new symbol, new price
  // sell side now with different symbol....
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          10,
                                          SELL,
                                          1000,
                                          stock_t("DIFFSYM"),
                                          price4_t(180000)});
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("DIFFSYM"), 0,
      0); // new symbol, new price

  // ... a duplicate order id should result in no changes ...
  // add_order_mesage with same id=1, DIFFSYM this time
  tested.handle_message(
      now, ++msgcnt, 0, add_order_message{{add_order_message::message_type, 0,
                                           0, create_timestamp()},
                                          2000,
                                          BUY,
                                          700,
                                          stock_t("DIFFSYM"),
                                          price4_t(160000)});
  // no *new* callback is expected ....
  // ... therefore verifies it gets the first add_order_message
  // value still on the (memory) logs of calls (CRAZY)
  callback.check_called().once().with(
      compute_book_cache_aware::time_point(now), stock_t("CRAZY"), 0,
      0); // previous logged callback
}
