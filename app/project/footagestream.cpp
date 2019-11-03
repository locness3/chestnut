/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
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

#include "footagestream.h"

#include <QPainter>

#include "debug.h"

using project::FootageStream;

void FootageStream::make_square_thumb()
{
  // generate square version for QListView?
  const auto max_dimension = qMax(video_preview.width(), video_preview.height());
  QPixmap pixmap(max_dimension, max_dimension);
  pixmap.fill(Qt::transparent);
  QPainter p(&pixmap);
  const auto diff = (video_preview.width() - video_preview.height()) / 2;
  const auto sqx = (diff < 0) ? -diff : 0;
  const auto sqy = (diff > 0) ? diff : 0;
  p.drawImage(sqx, sqy, video_preview);
  video_preview_square = QIcon(pixmap);
}

bool FootageStream::load(QXmlStreamReader& stream)
{
  auto name = stream.name().toString().toLower();
  if (name == "video") {
    type_ = StreamType::VIDEO;
  } else if (name == "audio") {
    type_ = StreamType::AUDIO;
  } else {
    qCritical() << "Unknown stream type" << name;
    return false;
  }

  // attributes
  for (const auto& attr : stream.attributes()) {
    const auto name = attr.name().toString().toLower();
    if (name == "id") {
      file_index = attr.value().toInt();
    } else if ( (name == "infinite")  && (type_ == StreamType::VIDEO) ) {
      infinite_length = attr.value().toString().toLower() == "true";
    } else {
      qWarning() << "Unknown attribute" << name;
    }
  }

  // elements
  while (stream.readNextStartElement()) {
    const auto name = stream.name().toString().toLower();
    if ( (name == "width") && (type_ == StreamType::VIDEO) ) {
      video_width = stream.readElementText().toInt();
    } else if ( (name == "height") && (type_ == StreamType::VIDEO) ) {
      video_height = stream.readElementText().toInt();
    } else if ( (name == "framerate") && (type_ == StreamType::VIDEO) ) {
      video_frame_rate = stream.readElementText().toDouble();
    } else if ( (name == "channels") && (type_ == StreamType::AUDIO) ) {
      audio_channels = stream.readElementText().toInt();
    } else if ( (name == "frequency") && (type_ == StreamType::AUDIO) ) {
      audio_frequency = stream.readElementText().toInt();
    } else if ( (name == "layout") && (type_ == StreamType::AUDIO) ) {
      audio_layout = stream.readElementText().toInt();
    } else {
      qWarning() << "Unhandled element" << name;
      stream.skipCurrentElement();
    }
  }

  return true;
}

bool FootageStream::save(QXmlStreamWriter& stream) const
{
  switch (type_) {
    case StreamType::VIDEO:
      stream.writeStartElement("video");
      stream.writeAttribute("id", QString::number(file_index));
      stream.writeAttribute("infinite", infinite_length ? "true" : "false");

      stream.writeTextElement("width", QString::number(video_width));
      stream.writeTextElement("height", QString::number(video_height));
      stream.writeTextElement("framerate", QString::number(video_frame_rate, 'g', 10));
      stream.writeEndElement();
      return true;
    case StreamType::AUDIO:
      stream.writeStartElement("audio");
      stream.writeAttribute("id", QString::number(file_index));

      stream.writeTextElement("channels", QString::number(audio_channels));
      stream.writeTextElement("layout", QString::number(audio_layout));
      stream.writeTextElement("frequency", QString::number(audio_frequency));
      stream.writeEndElement();
      return true;
    default:
      chestnut::throwAndLog("Unknown stream type. Not saving");
  }
  return false;
}
