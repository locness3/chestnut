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
#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <QVector>
#include <QMutex>

#include "project/sequence.h"

class Clip;
class ClipCache;

struct AVFrame;

extern bool texture_failed;
extern bool rendering;


void open_clip(ClipPtr clip, bool multithreaded);
void cache_clip(ClipPtr clip, long playhead, bool reset, bool scrubbing, QVector<ClipPtr>& nests);
void cache_audio_worker(ClipPtr c, bool write_A);
void cache_video_worker(ClipPtr c, long playhead);
void handle_media(Sequence* e_sequence, long playhead, bool multithreaded);
void reset_cache(ClipPtr c, long target_frame);
void get_clip_frame(ClipPtr c, long playhead);
double get_timecode(ClipPtr c, long playhead);

long playhead_to_clip_frame(ClipPtr c, long playhead);
double playhead_to_clip_seconds(ClipPtr c, long playhead);
int64_t seconds_to_timestamp(ClipPtr c, double seconds);
int64_t playhead_to_timestamp(ClipPtr c, long playhead);

int retrieve_next_frame(ClipPtr c, AVFrame* f);
void get_next_audio(ClipPtr c, bool mix);
void set_sequence(SequencePtr s);
void closeActiveClips(SequencePtr s);

#endif // PLAYBACK_H
