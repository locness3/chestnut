#ifndef DIPTOCOLOURTRANSITION_H
#define DIPTOCOLOURTRANSITION_H

#include "project/transition.h"

namespace fx
{
  class DipToColourTransition : public Transition
  {
    public:
      DipToColourTransition(ClipPtr c, ClipPtr s, const EffectMeta& em);

      virtual void setupUi() override;
      virtual void process_coords(double timecode, GLTextureCoords &, int data) override;
    private:
      EffectField* colour_field_{nullptr};
      QColor colour_;
  };
}
#endif // DIPTOCOLOURTRANSITION_H
