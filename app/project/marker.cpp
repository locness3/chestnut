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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "marker.h"

constexpr const char* const START_ELEM = "marker";
constexpr const char* const ATTR_FRAME = "frame";
constexpr const char* const ATTR_NAME = "name";
constexpr const char* const ATTR_COMMENT = "comment";
constexpr const char* const ATTR_DURATION = "duration";
constexpr const char* const ATTR_COLOR = "color";

void Marker::load(const QXmlStreamReader& stream)
{
  for (const auto& attr : stream.attributes()) {
    if (attr.name() == ATTR_FRAME) {
      frame = attr.value().toLong();
    } else if (attr.name() == ATTR_NAME) {
      name = attr.value().toString();
    } else if (attr.name() == ATTR_COMMENT) {
      comment_ = attr.value().toString();
    } else if (attr.name() == ATTR_DURATION) {
      duration_ = attr.value().toLong();
    } else if (attr.name() == ATTR_COLOR) {
      color_ = QColor(static_cast<QRgb>(attr.value().toUInt()));
    }
  }
}

bool Marker::save(QXmlStreamWriter& stream) const
{
  stream.writeStartElement(START_ELEM);
  stream.writeAttribute(ATTR_FRAME, QString::number(frame));
  stream.writeAttribute(ATTR_NAME, name);
  stream.writeAttribute(ATTR_COMMENT, comment_);
  stream.writeAttribute(ATTR_DURATION, QString::number(duration_));
  stream.writeAttribute(ATTR_COLOR, QString::number(color_.rgb()));
  stream.writeEndElement();
  return true;
}
