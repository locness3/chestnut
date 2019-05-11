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

#include "trackareawidget.h"
#include "ui_trackareawidget.h"

TrackAreaWidget::TrackAreaWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::TrackAreaWidget)
{
  ui->setupUi(this);
}

TrackAreaWidget::~TrackAreaWidget()
{
  delete ui;
}


void TrackAreaWidget::initialise()
{
  Q_ASSERT(ui != nullptr);
  Q_ASSERT(ui->lockButton != nullptr);
  Q_ASSERT(ui->enableButton != nullptr);

  QObject::connect(ui->lockButton, &QPushButton::toggled, this, &TrackAreaWidget::lockTrack);
  QObject::connect(ui->enableButton, &QPushButton::toggled, this, &TrackAreaWidget::enableTrack);
}


void TrackAreaWidget::setName(const QString& name)
{
  Q_ASSERT(ui != nullptr);
  Q_ASSERT(ui->trackNameLabel != nullptr);

  ui->trackNameLabel->setText(name);
}
