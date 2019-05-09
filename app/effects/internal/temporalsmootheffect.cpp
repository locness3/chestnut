#include "temporalsmootheffect.h"
#include <omp.h>
#include <cmath>

#include "gsl/span"

TemporalSmoothEffect::TemporalSmoothEffect(ClipPtr c, const EffectMeta& em) : Effect(std::move(c), em)
{  
  setCapability(Capability::IMAGE);

}

TemporalSmoothEffect::~TemporalSmoothEffect()
{
  while (!frames_.empty()) {
    delete[] frames_.front().data();
    frames_.pop_front();
  }
}

uint8_t average(const int ix, const VectorSpanBytes& data)
{
  auto val = 0;
  for (const auto& d: data) {
    val += d[ix];
  }
  // average pixel in all frames
  return static_cast<uint8_t>(val / data.size());
}

uint8_t median(const int ix, const VectorSpanBytes& data)
{
  const auto sz = data.size();
  gsl::span<uint8_t> vals;
  auto lim = sz > 10 ? 10 : sz;
  for (auto i = 0; i < lim; ++i) {
    vals[i] = data[i].at(ix);
  }
  std::sort(vals.begin(), vals.end());
  uint8_t val;
  if (sz % 2 == 0) {
    // sum pixels "around" the middle and avg
    uint16_t tmp = vals.at(sz/2);
    tmp += vals.at(lround(static_cast<double>(sz)/2));
    val = static_cast<uint8_t>(lround(static_cast<double>(tmp) / 2));
  } else {
    // use middle val
    val = vals.at(sz/2);
  }
  return val;
}

uint8_t maxVal(const int ix, const VectorSpanBytes& data)
{
  uint8_t val = 0;
  for (const auto& d : data) {
    if (d[ix] > val) {
      val = d[ix];
    }
  }

  return val;
}

uint8_t minVal(const int ix, const VectorSpanBytes& data)
{
  uint8_t val = 255;
  for (const auto& d : data) {
    if (d[ix] < val) {
      val = d[ix];
    }
  }

  return val;
}

void TemporalSmoothEffect::process_image(double timecode, gsl::span<uint8_t>& data)
{
  if (frame_length_ == nullptr || blend_mode_ == nullptr) {
    qCritical() << "Ui Elements null";
    return;
  }
  const auto length = lround(frame_length_->get_double_value(timecode));

  while (frames_.size() > length - 1) { // -1 as new frame is about to be added
    delete[] frames_.front().data();
    frames_.pop_front();
  }
  // data needs to be copied as is deleted outside of class
  const auto tmp = new uint8_t[static_cast<uint32_t>(data.size())];
  memcpy(tmp, data.data(), static_cast<size_t>(data.size()));
  frames_.push_back(gsl::span<uint8_t>(tmp, data.size()));

  if (!is_enabled()) {
    return;
  }


  const auto mode_index = blend_mode_->get_combo_index(timecode);
#pragma omp parallel for
  for (auto i=0L; i < data.size(); ++i) {
    uint8_t val;
    switch (mode_index) {
      case 0:
        val = average(i, frames_);
        break;
      case 1:
        val = median(i, frames_);
        break;
      case 2:
        val = maxVal(i, frames_);
        break;
      case 3:
        val = minVal(i, frames_);
        break;
      default:
        val = data.at(i);
        break;
    }
    data[i] = val;
  }
}

void TemporalSmoothEffect::setupUi()
{
  if (ui_setup) {
    return;
  }
  Effect::setupUi();

  frame_length_ = add_row(tr("Frame Length"))->add_field(EffectFieldType::DOUBLE, "length");
  frame_length_->set_double_minimum_value(1.0);
  frame_length_->set_double_maximum_value(10.0);
  frame_length_->set_double_default_value(3.0);

  blend_mode_ = add_row(tr("Blend Mode"))->add_field(EffectFieldType::COMBO, "mode");
  blend_mode_->add_combo_item("Average", 0);
  blend_mode_->add_combo_item("Median", 0);
  blend_mode_->add_combo_item("Max", 0);
  blend_mode_->add_combo_item("Min", 0);
  blend_mode_->setDefaultValue(0);
}

