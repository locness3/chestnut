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

#ifndef FOOTAGE_H
#define FOOTAGE_H

#include <QString>
#include <QVector>
#include <QDir>

#include "project/projectitem.h"
#include "project/ixmlstreamer.h"
#include "project/footagestream.h"


class Clip;
class PreviewGenerator;
class MediaThrobber;
class Media;

using FootagePtr = std::shared_ptr<Footage>;
using FootageWPtr = std::weak_ptr<Footage>;



class Footage : public project::ProjectItem {
public:
  Footage();
  explicit Footage(const std::shared_ptr<Media>& parent);

  QString url;
  int64_t length{0};
  QVector<project::FootageStreamPtr> video_tracks;
  QVector<project::FootageStreamPtr> audio_tracks;

  QDir proj_dir_{};
  int save_id{-1};
  int folder_{-1};

  double speed{1.0};

  PreviewGenerator* preview_gen{nullptr};

  long in{0};
  long out{0};
  bool using_inout{false};

  std::atomic_bool ready_{false};
  std::atomic_bool has_preview_{false};


  long get_length_in_frames(const double frame_rate) const;

  project::FootageStreamPtr video_stream_from_file_index(const int index);
  project::FootageStreamPtr audio_stream_from_file_index(const int index);
  bool has_stream_from_file_index(const int index);
  bool has_video_stream_from_file_index(const int index);
  bool has_audio_stream_from_file_index(const int index) const;
  void reset();
  bool isImage() const;
  virtual bool load(QXmlStreamReader& stream) override;
  virtual bool save(QXmlStreamWriter& stream) const override;
private:
  friend class FootageTest;
  project::FootageStreamPtr get_stream_from_file_index(const bool video, const int index);

  std::weak_ptr<Media> parent_mda_;

};

#endif // FOOTAGE_H
