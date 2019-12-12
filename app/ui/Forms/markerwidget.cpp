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

MarkerWidget::MarkerWidget(MarkerWPtr mark, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MarkerWidget),
  marker_(std::move(mark))
{
  auto marker = marker_.lock();
  Q_ASSERT(marker);
  ui->setupUi(this);
  ui->nameEdit->setText(marker->name);
  ui->commentsEdit->setPlainText(marker->comment_);

  // replace placeholder-widgets
  marker_icon_ = new MarkerIcon(this);
  marker_icon_->clr_ = marker->color_;
  ui->topHLayout->removeWidget(ui->iconWidget);
  delete ui->iconWidget;
  ui->topHLayout->insertWidget(0, marker_icon_);

  ui->formLayout->removeWidget(ui->inEdit);
  delete ui->inEdit;
  in_slider_ = new LabelSlider(marker->frame);
  in_slider_->set_frame_rate(marker->frame_rate_);
  in_slider_->decimal_places = 0;
  in_slider_->set_minimum_value(0);
  in_slider_->set_display_type(SliderType::FRAMENUMBER);
  ui->formLayout->insertRow(1, ui->label_2, in_slider_);

  ui->formLayout->removeWidget(ui->outEdit);
  out_slider_ = new LabelSlider(marker->frame + marker->duration_);
  out_slider_->set_frame_rate(marker->frame_rate_);
  out_slider_->decimal_places = 0;
  out_slider_->set_display_type(SliderType::FRAMENUMBER);
  out_slider_->set_minimum_value(0);
  ui->formLayout->insertRow(2, ui->label_3, out_slider_);
  delete ui->outEdit;

  connect(ui->nameEdit, &QLineEdit::textChanged, this, &MarkerWidget::onValChange);
  connect(ui->commentsEdit, &QPlainTextEdit::textChanged, this, &MarkerWidget::onCommentChanged);
  connect(in_slider_, &LabelSlider::valueChanged, this, &MarkerWidget::onSliderChanged);
  connect(out_slider_, &LabelSlider::valueChanged, this, &MarkerWidget::onSliderChanged);
}

MarkerWidget::~MarkerWidget()
{
  delete ui;
}

void MarkerWidget::onValChange(const QString& val)
{
  auto marker = marker_.lock();
  Q_ASSERT(marker);
  if (sender() == ui->nameEdit) {
    marker->name = val;
  } else if (sender() == ui->commentsEdit) {
    marker->comment_ = val;
  }
}

void MarkerWidget::onSliderChanged()
{
  auto marker = marker_.lock();
  Q_ASSERT(marker);
  if (sender() == in_slider_) {
    marker->frame = qRound(in_slider_->value());
  } else if (sender() == out_slider_) {
    marker->duration_ = qRound(out_slider_->value()) - marker->frame;
  } else {
    qWarning() << "Unhandled slider value change";
  }
}

void MarkerWidget::onCommentChanged()
{
  auto marker = marker_.lock();
  Q_ASSERT(marker);
  marker->comment_ = ui->commentsEdit->document()->toPlainText();
}
