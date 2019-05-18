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

#ifndef TRACKAREAWIDGET_H
#define TRACKAREAWIDGET_H

#include <QWidget>

namespace Ui {
  class TrackAreaWidget;
}

class TrackAreaWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit TrackAreaWidget(QWidget *parent = nullptr);
    ~TrackAreaWidget();

    TrackAreaWidget(const TrackAreaWidget&) = delete;
    TrackAreaWidget(const TrackAreaWidget&&) = delete;
    TrackAreaWidget& operator=(const TrackAreaWidget&) = delete;
    TrackAreaWidget& operator=(const TrackAreaWidget&&) = delete;

    void initialise();
    void setName(const QString& name);
    void setLocked(const bool locked);
    void setEnabled(const bool enabled);
  signals:
    void enableTrack(bool value);
    void lockTrack(bool value);
  private:
    Ui::TrackAreaWidget *ui;
};


#endif // TRACKAREAWIDGET_H
