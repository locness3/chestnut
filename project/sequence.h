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
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <QVector>
#include <memory>

#include "project/marker.h"
#include "project/selection.h"
#include "project/projectitem.h"

//FIXME: this is used EVERYWHERE. This has to be water-tight and heavily tested.

struct Clip;
class Transition;
class Media;

class Sequence : public project::ProjectItem {
public:
    Sequence();
    virtual ~Sequence();
    Sequence* copy();
    void getTrackLimits(int* video_tracks, int* audio_tracks);
    long getEndFrame();
    void hard_delete_transition(Clip *c, int type);

    void setWidth(const int val);
    int getWidth() const;
    void setHeight(const int val);
    int getHeight() const;
    double getFrameRate() const;
    void setFrameRate(const double frameRate);
    int getAudioFrequency() const;
    void setAudioFrequency(const int frequency);
    int getAudioLayout() const;
    void setAudioLayout(const int layout);

    QVector<Selection> selections;
    QVector<Clip*> clips;
    int save_id = 0;
    bool using_workarea = false;
    bool enable_workarea = true;
    long workarea_in = 0;
    long workarea_out = 0;
    QVector<Transition*> transitions;
    QVector<Marker> markers;
    long playhead = 0;
    bool wrapper_sequence = false;

private:
    int width = 0;
    int height = 0;
    double frame_rate = 0.0;
    int audio_frequency = 0;
    int audio_layout = 0;
};

typedef std::shared_ptr<Sequence> SequencePtr;

// static variable for the currently active sequence
extern SequencePtr e_sequence;

#endif // SEQUENCE_H
