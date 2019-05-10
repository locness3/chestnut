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

#include "ui/markerdockwidget.h"

#include <QInputDialog>

using ui::MarkerDockWidget;

std::tuple<QString, bool> MarkerDockWidget::getName() const
{
  QInputDialog d;
  d.setWindowTitle(QObject::tr("Set Marker"));
  d.setLabelText(QObject::tr("Set marker name:"));
  d.setInputMode(QInputDialog::TextInput);
  bool add_marker = (d.exec() == QDialog::Accepted);
  QString marker_name = d.textValue();
  return std::make_tuple(marker_name, add_marker);
}
