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

#ifndef MARKERSVIEWER_H
#define MARKERSVIEWER_H

#include <QDockWidget>

#include "project/media.h"

namespace Ui {
class MarkersViewer;
}

class MarkersViewer : public QDockWidget
{
  Q_OBJECT

public:
  explicit MarkersViewer(QWidget *parent = nullptr);
  ~MarkersViewer() override;

  /**
   * Set this viewers media item
   * @param mda
   * @return true==media set,  false==null ptr or invalid type
   */
  bool setMedia(const MediaPtr& mda);
  /**
   * Rebuild the viewer
   * @param filter a simple 'contains' search function in marker name or comment
   */
  void refresh(const QString& filter="");
  /**
   * Clear this viewers media item
   */
  void reset();


private:
  Ui::MarkersViewer *ui{};
  MediaWPtr media_{};
};

#endif // MARKERSVIEWER_H
