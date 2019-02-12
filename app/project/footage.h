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
#include <QMetaType>
#include <QVariant>
#include <QMutex>
#include <QPixmap>
#include <QIcon>

#include "project/projectitem.h"

#define VIDEO_PROGRESSIVE 0
#define VIDEO_TOP_FIELD_FIRST 1
#define VIDEO_BOTTOM_FIELD_FIRST 2


class Clip;
class PreviewGenerator;
class MediaThrobber;


using FootagePtr = std::shared_ptr<Footage>;
using FootageWPtr = std::weak_ptr<Footage>;


class FootageStream {
  public:
    FootageStream() = default;
    FootageStream(const FootageStream&) = default;
    FootageStream& operator=(const FootageStream&) = default;

    void make_square_thumb();

    int file_index;
    int video_width;
    int video_height;
    bool infinite_length;
    double video_frame_rate;
    int video_interlacing;
    int video_auto_interlacing;
    int audio_channels;
    int audio_layout;
    int audio_frequency;
    bool enabled;

    // preview thumbnail/waveform
    bool preview_done;
    QImage video_preview;
    QIcon video_preview_square;
    QVector<char> audio_preview;
};
using FootageStreamPtr = std::shared_ptr<FootageStream>;
using FootageStreamWPtr = std::weak_ptr<FootageStream>;

class Footage : public project::ProjectItem {
public:
    Footage();
    virtual ~Footage();
    Footage(const Footage&) = delete;
    Footage(const Footage&&) = delete;
    Footage& operator=(const Footage&) = delete;
    Footage& operator=(const Footage&&) = delete;

    QString url;
    int64_t length = 0;
    QVector<FootageStreamPtr> video_tracks;
    QVector<FootageStreamPtr> audio_tracks;
    int save_id;
    bool ready;
    bool invalid;
    double speed;

    PreviewGenerator* preview_gen;
    QMutex ready_lock;

    bool using_inout = false;
    long in;
    long out;

    long get_length_in_frames(const double frame_rate) const;

    FootageStreamPtr video_stream_from_file_index(const int index);
    FootageStreamPtr audio_stream_from_file_index(const int index);
    bool has_stream_from_file_index(const int index);
    bool has_video_stream_from_file_index(const int index);
    bool has_audio_stream_from_file_index(const int index) const;
    void reset();
  private:
    FootageStreamPtr get_stream_from_file_index(const bool video, const int index);

};

#endif // FOOTAGE_H
