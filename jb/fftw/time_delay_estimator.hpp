#ifndef jb_fftw_time_delay_estimator_hpp
#define jb_fftw_time_delay_estimator_hpp

#include <jb/fftw/aligned_vector.hpp>
#include <jb/fftw/plan.hpp>
#include <jb/complex_traits.hpp>

namespace jb {
namespace fftw {

/**
 * A simple time delay estimator based on cross-correlation
 */
template<
  typename timeseries_t,
  template<typename T> class vector = jb::fftw::aligned_vector
  >
class time_delay_estimator {
 public:
  typedef timeseries_t timeseries_type;
  typedef typename timeseries_type::value_type value_type;
  typedef typename jb::extract_value_type<value_type>::precision precision_type;
  typedef jb::fftw::plan<precision_type> plan;
  
  time_delay_estimator(timeseries_type const& a, timeseries_type const& b)
      : tmpa_(a.size())
      , tmpb_(b.size())
      , a2tmpa_(plan::create_forward(a, tmpa_))
      , b2tmpb_(plan::create_forward(b, tmpb_))
      , out_(a.size())
      , tmpa2out_(plan::create_backward(tmpa_, out_))
  {
    if (a.size() != b.size()) {
      throw std::invalid_argument("size mismatch in time_delay_estimator ctor");
    }
  }

  std::pair<bool, precision_type> estimate_delay(
      timeseries_type const& a, timeseries_type const& b) {
    // TODO() we should validate the size and alignment of the inputs
    // against the prototypes used in the constructor
    // First we apply the Fourier transform to both inputs ...
    a2tmpa_.execute(a, tmpa_);
    b2tmpb_.execute(b, tmpb_);
    // ... then we compute Conj(A) * B for the transformed inputs ...
    for (std::size_t i = 0; i != tmpa_.size(); ++i) {
      tmpa_[i] = std::conj(tmpa_[i]) * tmpb_[i];
    }
    // ... then we compute the inverse Fourier transform to the result ...
    tmpa2out_.execute(tmpa_, out_);
    // ... finally we compute (argmax,max) for the result ...
    precision_type max = std::numeric_limits<precision_type>::min();
    std::size_t argmax = 0;
    for (std::size_t i = 0; i != out_.size(); ++i) {
      if (max < out_[i]) {
        max = out_[i];
        argmax = i;
      }
    }
    // TODO() the tolerance should be a parameter
    // TODO() the threshold for acceptance should be configurable,
    // maybe we want the value to be close to the expected area of 'a'
    // for example ...
    if (max <= std::numeric_limits<precision_type>::epsilon()) {
      return std::make_pair(false, precision_type(0));
    }
    return std::make_pair(true, precision_type(argmax));
  }

 private:
  vector<std::complex<precision_type>> tmpa_;
  vector<std::complex<precision_type>> tmpb_;
  plan a2tmpa_;
  plan b2tmpb_;
  vector<precision_type> out_;
  plan tmpa2out_;
};

} // namespace fftw
} // namespace jb

#endif // jb_fftw_time_delay_estimator_hpp