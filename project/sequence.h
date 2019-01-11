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

#include "project/marker.h"
#include "project/selection.h"

//FIXME: this is used EVERYWHERE. This has to be water-tight and heavily tested.

struct Clip;
class Transition;
class Media;

class Sequence {
public:
    Sequence();
	~Sequence();
    Sequence* copy();
	void getTrackLimits(int* video_tracks, int* audio_tracks);
	long getEndFrame();
	void hard_delete_transition(Clip *c, int type);

    //TODO: use Q_PROPERTY
    const QString&  getName() const;
    void setName(const QString& name);
    const QPair<int,int>& getDimensions() const;
    void setDimensions(const QPair<int,int>& dimensions);
    double getFrameRate() const;
    void setFrameRate(const double frameRate);
    int getAudioFrequency() const;
    void setAudioFrequency(const int audioFrequency);
    int getAudioLayout() const;
    void setAudioLayout(const int audioLayout);

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
    QString m_name;
    QPair<int,int> m_dimensions;

    double m_frameRate = 0.0;
    int m_audioFrequency = 0;
    int m_audioLayout = 0;
};

// static variable for the currently active sequence
extern Sequence* sequence;

#endif // SEQUENCE_H
