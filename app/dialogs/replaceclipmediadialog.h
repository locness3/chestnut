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
#ifndef REPLACECLIPMEDIADIALOG_H
#define REPLACECLIPMEDIADIALOG_H

#include <QDialog>

#include "project/media.h"

class SourceTable;
class QTreeView;
class QCheckBox;

class ReplaceClipMediaDialog : public QDialog {
    Q_OBJECT
  public:
    ReplaceClipMediaDialog(QWidget* parent, MediaPtr old_media);

    ReplaceClipMediaDialog(const ReplaceClipMediaDialog& ) = delete;
    ReplaceClipMediaDialog& operator=(const ReplaceClipMediaDialog&) = delete;

  private slots:
    void replace();
  private:
    MediaPtr media_;
    QTreeView* tree = nullptr;
    QCheckBox* use_same_media_in_points = nullptr;
};

#endif // REPLACECLIPMEDIADIALOG_H
