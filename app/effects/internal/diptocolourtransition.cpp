#include "diptocolourtransition.h"

namespace  {
  const QColor DIP_COLOUR(255,255,255,255);
}

using fx::DipToColourTransition;

DipToColourTransition::DipToColourTransition(ClipPtr c, ClipPtr s, const EffectMeta& em) : Transition(c, s, em)
{

}

