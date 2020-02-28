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

#include "markerwidget.h"
#include "ui_markerwidget.h"

#include <QPainter>
#include "debug.h"

constexpr int PAINT_DIM = 8;
constexpr int MIN_ICON_WIDTH = 96;
constexpr int MIN_ICON_HEIGHT = 64;

MarkerIcon::MarkerIcon(QWidget *parent) : QWidget(parent)
{
  setMinimumSize(MIN_ICON_WIDTH, MIN_ICON_HEIGHT);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
}

void MarkerIcon::paintEvent(QPaintEvent* /*event*/)
{
  if (!isVisible()) {
    return;
  }
  QPainter painter(this);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);

  QPainterPath path;
  path.moveTo (0, 0);
  path.lineTo (0, PAINT_DIM);
  path.lineTo (PAINT_DIM,   0);
  path.lineTo (0, 0);
  painter.setPen (Qt::NoPen);
  painter.fillPath (path, QBrush (clr_));
}


void MarkerIcon::mouseDoubleClickEvent(QMouseEvent *event)
{
  //TODO:
  qInfo() << "Mouse dbl-click";
}

MarkerWidget::MarkerWidget(chestnut::project::MarkerPtr mark, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MarkerWidget),
  marker_(mark)
{
  Q_ASSERT(mark != nullptr);
  ui->setupUi(this);
  ui->nameEdit->setText(marker_->name);
  ui->inEdit->setText(QString::number(marker_->frame));
  ui->outEdit->setText(QString::number(marker_->frame + marker_->duration_));
  ui->commentsEdit->setPlainText(marker_->comment_);

  // replace placeholder-widget
  marker_icon_ = new MarkerIcon(this);
  marker_icon_->clr_ = marker_->color_;
  ui->topHLayout->removeWidget(ui->iconWidget);
  delete ui->iconWidget;
  ui->topHLayout->insertWidget(0, marker_icon_);

  connect(ui->nameEdit, &QLineEdit::textChanged, this, &MarkerWidget::onValChange);
  connect(ui->inEdit, &QLineEdit::textChanged, this, &MarkerWidget::onValChange);
  connect(ui->outEdit, &QLineEdit::textChanged, this, &MarkerWidget::onValChange);
  connect(ui->commentsEdit, &QPlainTextEdit::textChanged, this, &MarkerWidget::onCommentChanged);
}

MarkerWidget::~MarkerWidget()
{
  delete ui;
}

void MarkerWidget::onValChange(const QString& val)
{
  //TODO: command
  if (sender() == ui->nameEdit) {
    marker_->name = val;
  } else if (sender() == ui->inEdit) {
    marker_->frame = val.toLong();
  } else if (sender() == ui->outEdit) {
    marker_->duration_ = val.toLong() - marker_->frame;
  } else if (sender() == ui->commentsEdit) {
    marker_->comment_ = val;
  }
}

void MarkerWidget::onCommentChanged()
{
  //TODO: command
  marker_->comment_ = ui->commentsEdit->document()->toPlainText();
}
