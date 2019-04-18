#ifndef DIPTOCOLOURTRANSITION_H
#define DIPTOCOLOURTRANSITION_H

#include "project/transition.h"

namespace fx
{
  class DipToColourTransition : public Transition
  {
    public:
      DipToColourTransition(ClipPtr c, ClipPtr s, const EffectMeta& em);
  };
}
#endif // DIPTOCOLOURTRANSITION_H
