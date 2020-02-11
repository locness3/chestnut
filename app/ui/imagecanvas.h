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

#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QWidget>

namespace chestnut::ui
{
  class ImageCanvas : public QWidget
  {
      Q_OBJECT
    public:
      explicit ImageCanvas(QWidget* parent=nullptr);
      void setImage(QPixmap img);
      void clear();
      void enableSmoothResize(const bool enable) noexcept;
      void setZoom(const double zoom) noexcept;
    signals:
      void isAutoScaling(bool) const;
    protected:
      void paintEvent(QPaintEvent* event) override;
      void resizeEvent(QResizeEvent* event) override;
    private:
      QPixmap img_;
      QPixmap img_cached_;
      double zoom_ {-1};
      Qt::TransformationMode resize_mode_ {Qt::FastTransformation};
    private:
      QPixmap resizeImage(const QPixmap& img) const;
      bool autoScale() const noexcept;
  };
}

#endif // IMAGECANVAS_H
