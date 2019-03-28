#ifndef TEMPORALSMOOTHEFFECT_H
#define TEMPORALSMOOTHEFFECT_H

#include "project/effect.h"

class TemporalSmoothEffect : public Effect
{
  public:
    TemporalSmoothEffect(ClipPtr c, const EffectMeta* em);
    ~TemporalSmoothEffect() override;
    virtual void process_image(double timecode, uint8_t* data, int size) override;
  private:
    EffectField* frame_length_;
    EffectField* blend_mode_;
    QVector<uint8_t*> frames_;
};

#endif // TEMPORALSMOOTHEFFECT_H
