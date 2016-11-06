#ifndef jb_itch5_price_levels_hpp
#define jb_itch5_price_levels_hpp

#include <jb/itch5/price_field.hpp>

#include <stdexcept>

namespace jb {
namespace itch5 {

/**
 * Compute the number of price levels between two prices.
 *
 * @throws std::bad_range if hi < lo
 */
template <typename price_field_t>
int price_levels(price_field_t lo, price_field_t hi) {
  price_field_t const unit = price_field_t(price_field_t::denom);
  price_field_t const penny = price_field_t(unit.as_integer() / 100);
  price_field_t const mill = price_field_t(penny.as_integer() / 100);

  static_assert(
      price_field_t::denom >= 10000,
      "price_levels() does not work with denom < 10000");

  if (hi < lo) {
    throw std::range_error("invalid price range in price_levels()");
  }
  if (unit <= lo) {
    // ... range is above $1.0, return the number of levels for that
    // case ...
    return (hi.as_integer() - lo.as_integer()) / penny.as_integer();
  }
  if (hi <= unit) {
    // ... range is below $1.0, return the number of levels for that
    // case ...
    return (hi.as_integer() - lo.as_integer()) / mill.as_integer();
  }
  // ... split the analysis ...
  return price_levels(lo, unit) + price_levels(unit, hi);
}

} // namespace itch5
} // namespace jb

#endif // jb_itch5_price_levels_hpp