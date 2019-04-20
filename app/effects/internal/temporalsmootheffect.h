#ifndef TEMPORALSMOOTHEFFECT_H
#define TEMPORALSMOOTHEFFECT_H

#include "project/effect.h"
#include "gsl/span"

using VectorSpanBytes = QVector<gsl::span<uint8_t>>;

class TemporalSmoothEffect : public Effect
{
  public:
    TemporalSmoothEffect(ClipPtr c, const EffectMeta& em);
    ~TemporalSmoothEffect() override;

    TemporalSmoothEffect(const TemporalSmoothEffect&) = delete;
    TemporalSmoothEffect(const TemporalSmoothEffect&&) = delete;
    TemporalSmoothEffect& operator=(const TemporalSmoothEffect&) = delete;
    TemporalSmoothEffect& operator=(const TemporalSmoothEffect&&) = delete;

    virtual void process_image(double timecode, gsl::span<uint8_t>& data) override;
    virtual void setupUi() override;
  private:
    EffectField* frame_length_;
    EffectField* blend_mode_;
    VectorSpanBytes frames_;
};

#endif // TEMPORALSMOOTHEFFECT_H
