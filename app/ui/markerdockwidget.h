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

#ifndef MARKERDOCKWIDGET_H
#define MARKERDOCKWIDGET_H

#include <QString>
#include <QObject>
#include <tuple>

namespace ui {
  class MarkerDockWidget { //TODO: come up with a better name. its not a dock widget and markerwidget already exists
    public:
      MarkerDockWidget() = default;
      virtual ~MarkerDockWidget() = default;
      /**
       * Create a new marker of an object in the widget
       */
      virtual void setMarker() const = 0;
      /**
       * Get the name of a new marker via a dialog
       * @return name & user accepted
       */
      std::tuple<QString, bool> getName() const;
  };
}
#endif // MARKERDOCKWIDGET_H
