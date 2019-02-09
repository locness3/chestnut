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
#ifndef MEDIAPROPERTIESDIALOG_H
#define MEDIAPROPERTIESDIALOG_H

#include <QDialog>

#include "project/media.h"

class Footage;
class QComboBox;
class QLineEdit;
class QListWidget;
class QDoubleSpinBox;

class MediaPropertiesDialog : public QDialog {
    Q_OBJECT
  public:
    MediaPropertiesDialog(QWidget *parent, MediaPtr mda);

    MediaPropertiesDialog(const MediaPropertiesDialog& ) = delete;
    MediaPropertiesDialog& operator=(const MediaPropertiesDialog&) = delete;

  private:
    QComboBox* interlacing_box;
    QLineEdit* name_box;
    MediaPtr item;
    QListWidget* track_list;
    QDoubleSpinBox* conform_fr;
  private slots:
    void accept();
};

#endif // MEDIAPROPERTIESDIALOG_H
