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

#include "subclipcreatedialog.h"
#include "ui_subclipcreatedialog.h"


SubClipCreateDialog::SubClipCreateDialog(const QString& default_name, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SubClipCreateDialog)
{
  Q_ASSERT(ui);
  ui->setupUi(this);
  Q_ASSERT(ui->okButton);
  Q_ASSERT(ui->cancelButton);
  connect(ui->okButton, &QPushButton::pressed, this, &SubClipCreateDialog::buttonPressed);
  connect(ui->cancelButton, &QPushButton::pressed, this, &SubClipCreateDialog::buttonPressed);
  ui->label->setText(tr("Name:"));
  ui->lineEdit->setText(default_name);
  ui->lineEdit->selectAll();
  setWindowTitle(tr("Make Subclip"));
}

SubClipCreateDialog::~SubClipCreateDialog()
{
  delete ui;
}


QString SubClipCreateDialog::subClipName() const
{
  return sub_name_;
}


void SubClipCreateDialog::buttonPressed()
{
  Q_ASSERT(ui);
  Q_ASSERT(ui->lineEdit);
  if (this->sender() == ui->okButton) {
    sub_name_ = ui->lineEdit->text();
  }
  // FIXME: setResult isn't doing what we want. code returned on exec() is always QDialogCode::Rejected
  // have to use property of sub_name_ (e.g. it's length) instead
  close();
}
