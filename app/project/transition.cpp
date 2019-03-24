/* 
 * Olive. Olive is a free non-linear video editor for Windows, macOS, and Linux.
 * Copyright (C) 2018  {{ organization }}
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "transition.h"

#include "ui/mainwindow.h"
#include "clip.h"
#include "sequence.h"
#include "debug.h"

#include "io/clipboard.h"

#include "effects/internal/crossdissolvetransition.h"
#include "effects/internal/linearfadetransition.h"
#include "effects/internal/exponentialfadetransition.h"
#include "effects/internal/logarithmicfadetransition.h"
#include "effects/internal/cubetransition.h"

#include "ui/labelslider.h"

#include "panels/panels.h"
#include "panels/timeline.h"

#include <QMessageBox>
#include <QCoreApplication>


namespace{
  const long DEFAULT_TRANSITION_LENGTH = 30;
  const long MINIMUM_TRANSITION_LENGTH = 0;
}

Transition::Transition(ClipPtr c, ClipPtr s, const EffectMeta* em) :
  Effect(c, em),
  secondary_clip(s),
  length(DEFAULT_TRANSITION_LENGTH)
{
  length_field = add_row(tr("Length:"), false)->add_field(EffectFieldType::DOUBLE, "length");
  connect(length_field, SIGNAL(changed()), this, SLOT(set_length_from_slider()));
  length_field->set_double_default_value(DEFAULT_TRANSITION_LENGTH);
  length_field->set_double_minimum_value(MINIMUM_TRANSITION_LENGTH);

  LabelSlider* length_ui_ele = static_cast<LabelSlider*>(length_field->ui_element);
  length_ui_ele->set_display_type(SliderType::FRAMENUMBER);
  length_ui_ele->set_frame_rate(parent_clip->sequence == nullptr ? parent_clip->timeline_info.cached_fr : parent_clip->sequence->frameRate());
}

int Transition::copy(ClipPtr c, ClipPtr s) {
  return create_transition(c, s, meta, length);
}

void Transition::set_length(const long value) {
  length = value;
  length_field->set_double_value(value);
}

long Transition::get_true_length() const {
  return length;
}

long Transition::get_length() const {
  if (!secondary_clip.expired()) {
    return length * 2;
  }
  return length;
}

void Transition::set_length_from_slider() {
  set_length(length_field->get_double_value(0));
  update_ui(false);
}

TransitionPtr get_transition_from_meta(ClipPtr c, ClipPtr s, const EffectMeta* em) {
  if (!em->filename.isEmpty()) {
    // load effect from file
    return std::make_shared<Transition>(c, s, em);
  } else if (em->internal >= 0 && em->internal < TRANSITION_INTERNAL_COUNT) {
    // must be an internal effect
    switch (em->internal) {
      case TRANSITION_INTERNAL_CROSSDISSOLVE: return std::make_shared<CrossDissolveTransition>(c, s, em);
      case TRANSITION_INTERNAL_LINEARFADE: return std::make_shared<LinearFadeTransition>(c, s, em);
      case TRANSITION_INTERNAL_EXPONENTIALFADE: return std::make_shared<ExponentialFadeTransition>(c, s, em);
      case TRANSITION_INTERNAL_LOGARITHMICFADE: return std::make_shared<LogarithmicFadeTransition>(c, s, em);
      case TRANSITION_INTERNAL_CUBE: return std::make_shared<CubeTransition>(c, s, em);
      default:
        qWarning() << "Unhandled transition" << em->internal;
        break;
    }
  } else {
    qCritical() << "Invalid transition data";
    QMessageBox::critical(global::mainWindow,
                          QCoreApplication::translate("transition", "Invalid transition"),
                          QCoreApplication::translate("transition", "No candidate for transition '%1'. This transition "
                                                      "may be corrupt. Try reinstalling it or Olive.").arg(em->name)
                          );
  }
  return nullptr;
}

int create_transition(ClipPtr c, ClipPtr s, const EffectMeta* em, long length) {
  auto t = get_transition_from_meta(c, s, em);
  if (t != nullptr) {
    if (length >= 0) {
      t->set_length(length);
    }
    QVector<TransitionPtr>& transition_list = (c->sequence == nullptr) ? e_clipboard_transitions : c->sequence->transitions_;
    transition_list.append(t);
    return transition_list.size() - 1;
  }
  return -1;
}
