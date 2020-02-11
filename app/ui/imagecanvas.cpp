/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2020 Jonathan Noble
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

#include "imagecanvas.h"

#include <QPainter>
#include <QResizeEvent>
#include <QAbstractScrollArea>

#include "debug.h"

constexpr auto AUTOSCALE_VALUE = -1;

using chestnut::ui::ImageCanvas;

ImageCanvas::ImageCanvas(QWidget* parent) : QWidget(parent)
{
}

void ImageCanvas::setImage(QPixmap img)
{
  img_ = std::move(img);
  img_cached_ = resizeImage(img_);

  const auto par = this->parentWidget();
  Q_ASSERT(par != nullptr);
  if (autoScale() && (this->size() != par->size())) {
    // ensure this widget fills the scrollarea otherwise the image will be located at the top of the available area, not center
    this->resize(par->size());
  } else if (!autoScale()) {
    // smallest the size of widget can be is the scroll area otherwise image ends up in top-left
    const auto sz = (par->width() > img_cached_.width()) || (par->height() > img_cached_.height()) ? par->size()
                                                                                                   : img_cached_.size();
    if (this->size() != sz) {
      this->resize(sz);
    }
  }
  this->update();
}


void ImageCanvas::clear()
{
  img_ = QPixmap();
  img_cached_ = QPixmap();
}

void ImageCanvas::enableSmoothResize(const bool enable) noexcept
{
  resize_mode_ = enable ? Qt::SmoothTransformation : Qt::FastTransformation;
}

void ImageCanvas::setZoom(const double zoom) noexcept
{
  if (zoom > 0) {
    zoom_ = zoom;
  } else {
    zoom_ = AUTOSCALE_VALUE;
  }
}

void ImageCanvas::paintEvent(QPaintEvent* /*event*/)
{
  QPainter painter(this);
  painter.fillRect(img_cached_.rect(), Qt::transparent);
  const QSize sz = img_cached_.size();
  if ( (sz.height() == 0) || (sz.width() == 0) ) {
    // Nothing to do
    return;
  }
  // Positioning of image in widget
  const int x = (this->rect().width() - sz.width()) >> 1;
  const int y = (this->rect().height() - sz.height()) >> 1;
  painter.drawPixmap(QPoint(x, y), img_cached_, img_cached_.rect());
}


void ImageCanvas::resizeEvent(QResizeEvent* event)
{
  Q_ASSERT(event);
  if ( (img_.height() == 0) || (img_.width() == 0) || (!autoScale())) {
    // Nothing to do
    return;
  }
  img_cached_ = resizeImage(img_);
  this->update();
  QWidget::resizeEvent(event);
}


QPixmap ImageCanvas::resizeImage(const QPixmap& img) const
{
  emit isAutoScaling(autoScale());
  if (autoScale())
  {
    const auto par = parentWidget();
    Q_ASSERT(par);
    const auto ratio = static_cast<double>(img.width()) / img.height();
    if ( (ratio * par->height()) >  par->width()) {
      // To ensure the image is contained within the scroll area
      return img.scaledToWidth(par->width(), resize_mode_);
    }
    return img.scaledToHeight(par->height(), resize_mode_);
  } else {
    // not autoscaling, so use scale to zoom value
    if (qFuzzyCompare(zoom_, 1)) {
      // No scaling required
      return img;
    }
    const auto w = img.width() * zoom_;
    return img.scaledToWidth(qRound(w), resize_mode_);
  }
}


bool ImageCanvas::autoScale() const noexcept
{
  return qFuzzyCompare(zoom_, AUTOSCALE_VALUE);
}
