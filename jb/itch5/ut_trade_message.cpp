#include <jb/itch5/trade_message.hpp>
#include <jb/itch5/testing_data.hpp>

#include <boost/test/unit_test.hpp>

namespace {
// A sample message for testing
char const buf[] =
    u8"P"                 // Message Type
    JB_ITCH5_TEST_HEADER  // Common test header
    "\x00\x00\x00\x00"
    "\x00\x00\x10\x92"    // Order Reference Number (4242)
    "B"                   // Buy/Sell Indicator
    "\x00\x00\x00\x64"    // Shares (100)
    "HSART   "            // Stock
    "\x00\x12\xC6\xA4"    // Price (123.0500)
    "\x00\x00\x00\x00"
    "\x00\x23\xB6\xF8"    // Match Number (2340600)
    ;
std::size_t const bufsize = sizeof(buf) - 1;
} // anonymous namespace

/**
 * @test Verify that the jb::itch5::trade_message decoder works
 * as expected.
 */
BOOST_AUTO_TEST_CASE(decode_trade_message) {
  using namespace jb::itch5;
  using namespace std::chrono;

  auto expected_ts = duration_cast<nanoseconds>(
      hours(11) + minutes(32) + seconds(31) + nanoseconds(123456789L));

  auto x = decoder<true,trade_message>::r(bufsize, buf, 0);
  BOOST_CHECK_EQUAL(
      x.header.message_type, trade_message::message_type);
  BOOST_CHECK_EQUAL(x.header.stock_locate, 0);
  BOOST_CHECK_EQUAL(x.header.tracking_number, 1);
  BOOST_CHECK_EQUAL(x.header.timestamp.ts.count(), expected_ts.count());
  BOOST_CHECK_EQUAL(x.order_reference_number, 4242ULL);
  BOOST_CHECK_EQUAL(x.buy_sell_indicator, buy_sell_indicator_t(u'B'));
  BOOST_CHECK_EQUAL(x.shares, 100);
  BOOST_CHECK_EQUAL(x.stock, "HSART");
  BOOST_CHECK_EQUAL(x.price, price4_t(1230500));
  BOOST_CHECK_EQUAL(x.match_number, 2340600ULL);

  x = decoder<false,trade_message>::r(bufsize, buf, 0);
  BOOST_CHECK_EQUAL(
      x.header.message_type, trade_message::message_type);
  BOOST_CHECK_EQUAL(x.header.stock_locate, 0);
  BOOST_CHECK_EQUAL(x.header.tracking_number, 1);
  BOOST_CHECK_EQUAL(x.header.timestamp.ts.count(), expected_ts.count());
  BOOST_CHECK_EQUAL(x.order_reference_number, 4242ULL);
  BOOST_CHECK_EQUAL(x.buy_sell_indicator, buy_sell_indicator_t(u'B'));
  BOOST_CHECK_EQUAL(x.shares, 100);
  BOOST_CHECK_EQUAL(x.stock, "HSART");
  BOOST_CHECK_EQUAL(x.price, price4_t(1230500));
  BOOST_CHECK_EQUAL(x.match_number, 2340600ULL);
}

/**
 * @test Verify that jb::itch5::trade_message iostream
 * operator works as expected.
 */
BOOST_AUTO_TEST_CASE(stream_trade_message) {
  using namespace std::chrono;
  using namespace jb::itch5;

  auto tmp = decoder<false,trade_message>::r(bufsize, buf, 0);
  std::ostringstream os;
  os << tmp;
  BOOST_CHECK_EQUAL(
      os.str(), "message_type=P,stock_locate=0"
      ",tracking_number=1,timestamp=113231.123456789"
      ",order_reference_number=4242"
      ",buy_sell_indicator=B"
      ",shares=100"
      ",stock=HSART"
      ",price=123.0500"
      ",match_number=2340600"
      );
}