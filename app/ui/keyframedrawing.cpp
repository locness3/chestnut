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
#include "keyframedrawing.h"

#include "project/effect.h"

constexpr int KEYFRAME_POINT_COUNT = 4;
constexpr double DARKER_KEYFRAME_MODIFIER = 0.625;


bool valid_bezier_path(const EffectKeyframe& current_key, const EffectKeyframe& prev_key, const EffectKeyframe& next_key)
{
  bool valid = true;
  if (current_key.type != KeyframeType::BEZIER) {
    qWarning() << "Keyframe for bezier path checking is not of a bezier type";
    return false;
  }

  if (prev_key.type != KeyframeType::UNKNOWN) {
    //check handles don't overlap
    double check_val = prev_key.time;
    if (prev_key.type == KeyframeType::BEZIER) {
      check_val += prev_key.post_handle_x;
    }
    valid &= (current_key.pre_handle_x + current_key.time) > check_val;
  }

  if (next_key.type != KeyframeType::UNKNOWN) {
    // check handles don't overlap
    double check_val = next_key.time;
    if (next_key.type == KeyframeType::BEZIER) {
      check_val += next_key.pre_handle_x;
    }
    valid &= (current_key.post_handle_x + current_key.time) < check_val;
  }
  return valid;
}

void draw_keyframe(QPainter &p, const KeyframeType type, int x, int y, bool darker, int r, int g, int b) {
  if (darker) {
    r *= DARKER_KEYFRAME_MODIFIER;
    g *= DARKER_KEYFRAME_MODIFIER;
    b *= DARKER_KEYFRAME_MODIFIER;
  }
  p.setPen(QColor(0, 0, 0));
  p.setBrush(QColor(r, g, b));

  switch (type) {
    case KeyframeType::LINEAR:
    {
      QPoint points[KEYFRAME_POINT_COUNT] = {QPoint(x-KEYFRAME_SIZE, y), QPoint(x, y-KEYFRAME_SIZE),
                                             QPoint(x+KEYFRAME_SIZE, y), QPoint(x, y+KEYFRAME_SIZE)};
      p.drawPolygon(points, KEYFRAME_POINT_COUNT);
    }
      break;
    case KeyframeType::BEZIER:
      p.drawEllipse(QPoint(x, y), KEYFRAME_SIZE, KEYFRAME_SIZE);
      break;
    case KeyframeType::HOLD:
      p.drawRect(QRect(x - KEYFRAME_SIZE, y - KEYFRAME_SIZE, KEYFRAME_SIZE*2, KEYFRAME_SIZE*2));
      break;
    default:
      qWarning() << "Unhandled keyframe type" << static_cast<int>(type);
      break;
  }

  p.setBrush(Qt::NoBrush);
}
