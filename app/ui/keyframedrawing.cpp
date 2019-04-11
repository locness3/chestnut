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
