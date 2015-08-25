#include <jb/itch5/add_order_mpid_message.hpp>
#include <jb/itch5/testing_data.hpp>

#include <boost/test/unit_test.hpp>

namespace {
// A sample message for testing
char const buf[] =
    u8"F"                 // Message Type
    JB_ITCH5_TEST_HEADER  // Common test header
    "\x00\x00\x00\x00"
    "\x00\x00\x00\x2A"    // Order Reference Number (42)
    "B"                   // Buy/Sell Indicator
    "\x00\x00\x00\x64"    // Shares (100)
    "HSART   "            // Stock
    "\x00\x12\xC6\xA4"    // Price (123.0500)
    "LOOF"                // MPID
    ;
std::size_t const bufsize = sizeof(buf) - 1;
} // anonymous namespace

/**
 * @test Verify that the jb::itch5::add_order_mpid_message decoder works
 * as expected.
 */
BOOST_AUTO_TEST_CASE(decode_add_order_mpid_message) {
  using namespace jb::itch5;
  using namespace std::chrono;

  auto expected_ts = duration_cast<nanoseconds>(
      hours(11) + minutes(32) + seconds(31) + nanoseconds(123456789L));

  auto x = decoder<true,add_order_mpid_message>::r(bufsize, buf, 0);
  BOOST_CHECK_EQUAL(
      x.header.message_type, add_order_mpid_message::message_type);
  BOOST_CHECK_EQUAL(x.header.stock_locate, 0);
  BOOST_CHECK_EQUAL(x.header.tracking_number, 1);
  BOOST_CHECK_EQUAL(x.header.timestamp.ts.count(), expected_ts.count());
  BOOST_CHECK_EQUAL(x.order_reference_number, 42ULL);
  BOOST_CHECK_EQUAL(x.shares, 100);
  BOOST_CHECK_EQUAL(x.stock, "HSART");
  BOOST_CHECK_EQUAL(x.price, price4_t(1230500));
  BOOST_CHECK_EQUAL(x.attribution, "LOOF");

  x = decoder<false,add_order_mpid_message>::r(bufsize, buf, 0);
  BOOST_CHECK_EQUAL(
      x.header.message_type, add_order_mpid_message::message_type);
  BOOST_CHECK_EQUAL(x.header.stock_locate, 0);
  BOOST_CHECK_EQUAL(x.header.tracking_number, 1);
  BOOST_CHECK_EQUAL(x.header.timestamp.ts.count(), expected_ts.count());
  BOOST_CHECK_EQUAL(x.order_reference_number, 42ULL);
  BOOST_CHECK_EQUAL(x.shares, 100);
  BOOST_CHECK_EQUAL(x.stock, "HSART");
  BOOST_CHECK_EQUAL(x.price, price4_t(1230500));
  BOOST_CHECK_EQUAL(x.attribution, "LOOF");
}

/**
 * @test Verify that jb::itch5::add_order_mpid_message iostream
 * operator works as expected.
 */
BOOST_AUTO_TEST_CASE(stream_add_order_mpid_message) {
  using namespace std::chrono;
  using namespace jb::itch5;

  auto tmp = decoder<false,add_order_mpid_message>::r(bufsize, buf, 0);
  std::ostringstream os;
  os << tmp;
  BOOST_CHECK_EQUAL(
      os.str(), "message_type=F,stock_locate=0"
      ",tracking_number=1,timestamp=113231.123456789"
      ",order_reference_number=42"
      ",buy_sell_indicator=B"
      ",shares=100"
      ",stock=HSART"
      ",price=123.0500"
      ",attribution=LOOF"
      );
}
