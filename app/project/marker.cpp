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
constexpr const char* const ELEM_FRAME = "frame";
constexpr const char* const ELEM_NAME = "name";
constexpr const char* const ELEM_COMMENT = "comment";
constexpr const char* const ELEM_DURATION = "duration";
constexpr const char* const ELEM_COLOR = "color";

#include "debug.h"


bool Marker::operator<(const Marker& rhs) const
{
  return rhs.frame > frame;
}

bool Marker::load(QXmlStreamReader& stream)
{
  while (stream.readNextStartElement()) {
    auto elem_name = stream.name().toString().toLower();
    if (elem_name == ELEM_FRAME) {
      frame = stream.readElementText().toLong();
    } else if (elem_name == ELEM_NAME) {
      name = stream.readElementText();
    } else if (elem_name == ELEM_COMMENT) {
      comment_ = stream.readElementText();
    } else if (elem_name == ELEM_DURATION) {
      duration_ = stream.readElementText().toLong();
    } else if (elem_name == ELEM_COLOR) {
      color_ = QColor(static_cast<QRgb>(stream.readElementText().toUInt()));
    } else {
      qWarning() << "Unknown element" << elem_name;
      stream.skipCurrentElement();
    }
  }

  return true;
}

bool Marker::save(QXmlStreamWriter& stream) const
{
  stream.writeStartElement(START_ELEM);
  stream.writeTextElement(ELEM_FRAME, QString::number(frame));
  stream.writeTextElement(ELEM_NAME, name);
  stream.writeTextElement(ELEM_COMMENT, comment_);
  stream.writeTextElement(ELEM_DURATION, QString::number(duration_));
  stream.writeTextElement(ELEM_COLOR, QString::number(color_.rgb()));
  stream.writeEndElement();
  return true;
}
