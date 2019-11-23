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
#include "fontcombobox.h"

#include <QFontDatabase>

FontCombobox::FontCombobox(QWidget* parent) : ComboBoxEx(parent)
{
  addItems(QFontDatabase().families());

  value_ = currentText();

  connect(this, SIGNAL(currentTextChanged(QString)), this, SLOT(updateInternals()));
}

const QString& FontCombobox::getPreviousValue()
{
  return previous_value_;
}

QVariant FontCombobox::getValue() const
{
  return value_;
}

void FontCombobox::setValue(QVariant val)
{
  previous_value_ = value_;
  value_ = val.toString();
}

void FontCombobox::updateInternals()
{
  previous_value_ = value_;
  value_ = currentText();
}
