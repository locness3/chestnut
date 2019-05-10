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

#ifndef SCOPEVIEWER_H
#define SCOPEVIEWER_H

#include <QDockWidget>
#include <QComboBox>

#include "ui/colorscopewidget.h"

namespace panels
{
  class ScopeViewer : public QDockWidget
  {
      Q_OBJECT
    public:
      explicit ScopeViewer(QWidget* parent = nullptr);

    public slots:
      /**
       * @brief On the receipt of a grabbed frame from the renderer generate scopes
       * @param img Image of the final, rendered frame, minus the gizmos
       */
      void frameGrabbed(const QImage& img);

    private:
      ui::ColorScopeWidget* color_scope_{nullptr};
      QComboBox* waveform_combo_{nullptr};

      /*
       * Populate this viewer with its widgets
       */
      void setup();

    private slots:
      void indexChanged(int index);
  };
}

#endif // SCOPEVIEWER_H
