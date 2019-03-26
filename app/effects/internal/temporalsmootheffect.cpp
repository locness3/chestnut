#include "temporalsmootheffect.h"
#include <omp.h>

TemporalSmoothEffect::TemporalSmoothEffect(ClipPtr c, const EffectMeta* em) : Effect(c, em)
{
  enable_image = true;
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

void TemporalSmoothEffect::process_image(double timecode, uint8_t* data, int size)
{
  const auto length = static_cast<int>(frame_length_->get_double_value(timecode) + 0.5);

  while (frames_.size() > length - 1) { // -1 as new frame is about to be added
    delete[] frames_.front();
    frames_.pop_front();
  }
  // data needs to be copied as is deleted outside of class
  const auto tmp = new uint8_t[static_cast<uint32_t>(size)];
  memcpy(tmp, data, static_cast<size_t>(size));
  frames_.push_back(tmp);

  if (!is_enabled()) {
    return;
  }

#pragma omp parallel for
  for (auto i=0; i < size; ++i) {
    auto val = 0;
    for (auto frame: frames_) {
      val += frame[i];
    }
    // average pixel in all frames
    data[i] = static_cast<uint8_t>(val / frames_.size());
  }
}

