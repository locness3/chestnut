#ifndef TEMPORALSMOOTHEFFECT_H
#define TEMPORALSMOOTHEFFECT_H

#include "project/effect.h"
#include "gsl/span"

using VectorSpanBytes = QVector<gsl::span<uint8_t>>;

class TemporalSmoothEffect : public Effect
{
  public:
    TemporalSmoothEffect(ClipPtr c, const EffectMeta* em);
    ~TemporalSmoothEffect() override;
    virtual void process_image(double timecode, gsl::span<uint8_t>& data) override;
  private:
    EffectField* frame_length_;
    EffectField* blend_mode_;
    VectorSpanBytes frames_;
//    QVector<uint8_t*> frames_;
};

#endif // TEMPORALSMOOTHEFFECT_H
