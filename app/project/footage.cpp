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


#include "io/previewgenerator.h"
#include "project/clip.h"
#include "panels/project.h"

extern "C" {
#include <libavformat/avformat.h>
}

using project::FootageStreamPtr;

Footage::Footage() : Footage(nullptr)
{

}

Footage::Footage(const std::shared_ptr<Media>& parent) : parent_mda_(parent)
{
}

void Footage::reset()
{
  if (preview_gen != nullptr) {
    try {
      preview_gen->cancel();
      preview_gen->wait();
    } catch (const std::exception& ex) {
      qCritical() << "Caught an exception, msg =" << ex.what();
    } catch (...) {
      qCritical() << "Caught an unknown exception";
    }
  }
  video_tracks.clear();
  audio_tracks.clear();
  ready_ = false;
}

bool Footage::isImage() const
{
  return (has_preview_ && !video_tracks.empty()) && video_tracks.front()->infinite_length && (audio_tracks.empty());
}

bool Footage::load(QXmlStreamReader& stream)
{
  // attributes
  for (const auto& attr : stream.attributes()) {
    auto name = attr.name().toString().toLower();
    if (name == "folder") {
      folder_ = attr.value().toInt(); // Media::parent.id
      if (auto par = parent_mda_.lock()) {
        auto folder = Project::model().findItemById(folder_);
        par->setParent(folder);
      }
    } else if (name == "id") {
      save_id = attr.value().toInt();
      if (auto par = parent_mda_.lock()) {
        par->setId(save_id);
      }
    } else if (name == "using_inout") {
      using_inout = attr.value().toString() == "true";
    } else if (name == "in") {
      in = attr.value().toLong();
    } else if (name == "out") {
      out = attr.value().toLong();
    } else {
      qWarning() << "Unknown attribute" << name;
    }
  }

  //elements
  while (stream.readNextStartElement()) {
    auto elem_name = stream.name().toString().toLower();
    if ( (elem_name == "video") || (elem_name == "audio") ) {
      auto ms = std::make_shared<project::FootageStream>();
      ms->load(stream);
      if (elem_name == "video") {
        video_tracks.append(ms);
      } else {
        audio_tracks.append(ms);
      }
    } else if (elem_name == "name") {
      setName(stream.readElementText());
    } else if (elem_name == "url") {
      url = stream.readElementText();
    } else if (elem_name == "duration") {
      length = stream.readElementText().toLong();
    } else if (elem_name == "marker"){
      auto mrkr = std::make_shared<Marker>();
      mrkr->load(stream);
      markers_.append(mrkr);
    } else {
      qWarning() << "Unhandled element" << elem_name;
      stream.skipCurrentElement();
    }
  }

  //TODO: check what this does
//      if (!QFileInfo::exists(url)) { // if path is not absolute
//        QString proj_dir_test = proj_dir.absoluteFilePath(url);
//        QString internal_proj_dir_test = internal_proj_dir.absoluteFilePath(url);

//        if (QFileInfo::exists(proj_dir_test)) { // if path is relative to the project's current dir
//          url = proj_dir_test;
//          qInfo() << "Matched" << attr.value().toString() << "relative to project's current directory";
//        } else if (QFileInfo::exists(internal_proj_dir_test)) { // if path is relative to the last directory the project was saved in
//          url = internal_proj_dir_test;
//          qInfo() << "Matched" << attr.value().toString() << "relative to project's internal directory";
//        } else if (url.contains('%')) {
//          // hack for image sequences (qt won't be able to find the URL with %, but ffmpeg may)
//          url = internal_proj_dir_test;
//          qInfo() << "Guess image sequence" << attr.value().toString() << "path to project's internal directory";
//        } else {
//          qInfo() << "Failed to match" << attr.value().toString() << "to file";
//        }
//      } else {
//        qInfo() << "Matched" << attr.value().toString() << "with absolute path";
//      }
  return true;
}

bool Footage::save(QXmlStreamWriter& stream) const
{
  stream.writeStartElement("footage");
  if (auto par = parent_mda_.lock()) {
    if (par->parentItem() == nullptr) {
      chestnut::throwAndLog("Parent Media is unlinked");
    }
    stream.writeAttribute("id", QString::number(par->id()));
    stream.writeAttribute("folder", QString::number(par->parentItem()->id()));
  } else {
    chestnut::throwAndLog("Null Media parent");
  }
  stream.writeAttribute("using_inout", using_inout ? "true" : "false");
  stream.writeAttribute("in", QString::number(in));
  stream.writeAttribute("out", QString::number(out));

  stream.writeTextElement("name", name_);
  stream.writeTextElement("url", QDir(url).absolutePath());
  stream.writeTextElement("duration", QString::number(length));

  for (const auto& ms : video_tracks) {
    if (!ms) {
      continue;
    }
    ms->save(stream);
  }

  for (const auto& ms : audio_tracks) {
    if (!ms) {
      continue;
    }
    ms->save(stream);
  }

  for (auto& mark : markers_) {
    if (!mark) {
      continue;
    }
    mark->save(stream);
  }
  stream.writeEndElement();
  return true;
}

long Footage::get_length_in_frames(const double frame_rate) const
{
  Q_ASSERT(!qFuzzyIsNull(speed));
  Q_ASSERT(AV_TIME_BASE != 0);

  if (length >= 0) {
    return static_cast<long>(std::floor( (static_cast<double>(length) / AV_TIME_BASE)
                                         * (frame_rate / speed) ));
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
