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

#ifndef TIMELINETRACKAREA_H
#define TIMELINETRACKAREA_H

#include <QWidget>
#include <QMap>
#include <QVBoxLayout>

#include "ui/Forms/trackareawidget.h"

namespace Ui {
  class TimelineTrackArea;
}

enum class TimelineTrackType {
  AUDIO,
  VISUAL,
  NONE
};

class TimelineTrackArea : public QWidget
{
    Q_OBJECT

  public:
    explicit TimelineTrackArea(QWidget *parent = nullptr);
    ~TimelineTrackArea();

    TimelineTrackArea(const TimelineTrackArea&) = delete;
    TimelineTrackArea(const TimelineTrackArea&&) = delete;
    TimelineTrackArea& operator=(const TimelineTrackArea&) = delete;
    TimelineTrackArea& operator=(const TimelineTrackArea&&) = delete;

    bool initialise(const TimelineTrackType area_type);
    bool addTrack(const int number, const QString& name);

    void setHeights(const QVector<int>& heights);
    TimelineTrackType type_{TimelineTrackType::NONE};

  public slots:

  private slots:
    void trackEnableChange(const bool enabled);

    void trackLockChange(const bool locked);

  signals:
    void trackEnabled(const bool enabled, const int track_number);
    void trackLocked(const bool locked, const int track_number);

  private:
    Ui::TimelineTrackArea *ui;


    QMap<int, TrackAreaWidget*> track_widgets_;

    void update();
};

#endif // TIMELINETRACKAREA_H
