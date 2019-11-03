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

#ifndef SUBCLIPCREATEDIALOG_H
#define SUBCLIPCREATEDIALOG_H

#include <QDialog>

namespace Ui {
  class SubClipCreateDialog;
}

class SubClipCreateDialog : public QDialog
{
    Q_OBJECT

  public:
    SubClipCreateDialog(const QString& default_name, QWidget *parent = nullptr);
    ~SubClipCreateDialog();

    QString subClipName() const;
  private:
    Ui::SubClipCreateDialog *ui {nullptr};
    QString sub_name_;

  private slots:
    void buttonPressed();
};

#endif // SUBCLIPCREATEDIALOG_H
