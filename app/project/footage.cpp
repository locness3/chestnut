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
#include "footage.h"

#include <QDebug>
#include <QtMath>
#include <QPainter>

#include "io/previewgenerator.h"
#include "project/clip.h"

extern "C" {
#include <libavformat/avformat.h>
}

using project::FootageStreamPtr;

Footage::Footage()
{
  ready_lock.lock();
}

void Footage::reset()
{
  if (preview_gen != nullptr) {
    preview_gen->cancel();
    preview_gen->wait();
  }
  video_tracks.clear();
  audio_tracks.clear();
  ready = false;
}

bool Footage::isImage() const
{
  return (valid && !video_tracks.empty()) && video_tracks.front()->infinite_length && (audio_tracks.empty());
}


void Footage::load(const QXmlStreamReader& stream)
{

}
bool Footage::save(QXmlStreamWriter& stream) const
{
  if (!valid) {
    qWarning() << "Footage is not valid. Not saving";
    return false;
  }
  stream.writeStartElement("footage");
  stream.writeAttribute("id", QString::number(save_id));
  stream.writeAttribute("folder", QString::number(folder_));
  stream.writeAttribute("name", name());
  stream.writeAttribute("url", proj_dir_.relativeFilePath(url));
  stream.writeAttribute("duration", QString::number(length));
  stream.writeAttribute("using_inout", QString::number(using_inout));
  stream.writeAttribute("in", QString::number(in));
  stream.writeAttribute("out", QString::number(out));
  stream.writeAttribute("speed", QString::number(speed));

  for (const auto& ms : video_tracks) {
    if (!ms) continue;
    ms->save(stream);
  }

  for (const auto& ms : audio_tracks) {
    if (!ms) continue;
    ms->save(stream);
  }

  for (auto mark : markers_) {
    if (!mark) continue;
    mark->save(stream);
  }
  stream.writeEndElement();
  return true;
}

long Footage::get_length_in_frames(const double frame_rate) const {
  if (length >= 0) {
    return qFloor((static_cast<double>(length) / static_cast<double>(AV_TIME_BASE)) * frame_rate / speed);
  }
  return 0;
}

FootageStreamPtr Footage::video_stream_from_file_index(const int index)
{
  return get_stream_from_file_index(true, index);
}

FootageStreamPtr Footage::audio_stream_from_file_index(const int index)
{
  return get_stream_from_file_index(false, index);
}

bool Footage::has_stream_from_file_index(const int index)
{
  bool found = video_stream_from_file_index(index) != nullptr;
  found |= audio_stream_from_file_index(index) != nullptr;
  return found;
}


bool Footage::has_video_stream_from_file_index(const int index)
{
  return video_stream_from_file_index(index)!= nullptr;
}

FootageStreamPtr Footage::get_stream_from_file_index(const bool video, const int index)
{
  FootageStreamPtr stream;
  auto finder = [index] (auto tracks){
    for (auto track : tracks) {
      if (track->file_index == index) {
        return track;
      }
    }
    return FootageStreamPtr();
  };
  if (video) {
    stream = finder(video_tracks);
  } else {
    stream = finder(audio_tracks);
  }

  return stream;
}
