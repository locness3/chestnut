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


Footage::Footage()
    : ProjectItem(),
      ready(false),
      invalid(false),
      speed(1.0),
      preview_gen(nullptr),
      in(0),
      out(0)
{
    ready_lock.lock();
}

Footage::~Footage() {
    reset();
}

void Footage::reset() {
    if (preview_gen != nullptr) {
        preview_gen->cancel();
//        preview_gen->wait(); //FIXME: segfault. hard to ascertain why
    }
    video_tracks.clear();
    audio_tracks.clear();
    ready = false;
}

long Footage::get_length_in_frames(const double frame_rate) const {
    if (length >= 0) {
        return qFloor((static_cast<double>(length) / static_cast<double>(AV_TIME_BASE)) * frame_rate / speed);
    }
    return 0;
}


bool Footage::get_stream_from_file_index(const bool video, const int index, FootageStream& stream)
{
    auto found = false;
    if (video) {
        for (auto& track : video_tracks) {
            if (track.file_index == index) {
                found = true;
                stream = track;
                break;
            }
        }
    } else {
        for (auto& track : audio_tracks) {
            if (track.file_index == index) {
                found = true;
                stream = track;
                break;
            }
        }
    }
    return found;
}

void FootageStream::make_square_thumb() {
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
