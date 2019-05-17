/*
 * Chestnut. Chestnut is a free non-linear video editor.
 * Copyright (C) 2019
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

#include "project/track.h"
#include "debug.h"

using project::Track;

Track::Track(const int index) : index_(index)
{

}


bool Track::load(QXmlStreamReader& stream)
{
  while (stream.readNextStartElement()) {
    const auto name = stream.name().toString().toLower();
    if (name == "index") {
      index_ = stream.readElementText().toInt();
    } else if (name == "enabled") {
      enabled_ = stream.readElementText() == "true";
    } else if (name == "locked") {
      locked_ = stream.readElementText() == "true";
    } else if (name == "name") {
      name_ = stream.readElementText();
    } else {
      qWarning() << "Unhandled element" << name;
      stream.skipCurrentElement();
    }

  }
  return false;
}

bool Track::save(QXmlStreamWriter& stream) const
{
  stream.writeStartElement("track");
  stream.writeTextElement("index", QString::number(index_));
  stream.writeTextElement("enabled", enabled_ ? "true" : "false");
  stream.writeTextElement("locked", locked_ ? "true" : "false");
  stream.writeTextElement("name", name_);
  stream.writeEndElement();
  return true;
}
