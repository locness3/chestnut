#include "diptocolourtransition.h"

namespace  {
  const QColor DEFAULT_DIP_COLOUR(255,255,255,255);
}

using fx::DipToColourTransition;

DipToColourTransition::DipToColourTransition(ClipPtr c, ClipPtr s, const EffectMeta& em)
  : Transition(c, s, em),
    colour_(DEFAULT_DIP_COLOUR)
{
  setCapability(Capability::COORDS);
}

void DipToColourTransition::setupUi()
{
  if (ui_setup) {
    return;
  }
  Transition::setupUi();
  EffectRowPtr top_left = add_row(tr("Dip Colour"));
  colour_field_ = top_left->add_field(EffectFieldType::COLOR, "dipcolour");
}

void DipToColourTransition::process_coords(double progress, GLTextureCoords&, int data)
{
  colour_ = colour_field_->get_color_value(progress);


  float color[4];
  glGetFloatv(GL_CURRENT_COLOR, color);
  if (data == TA_CLOSING_TRANSITION) {
    progress = 1.0 - progress;
  }

  // TODO: set the 'background' colour

  glColor4f(color[0], color[1], color[2], color[3] * static_cast<GLfloat>(progress));
}



