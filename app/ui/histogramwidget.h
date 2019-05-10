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

#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QPen>
#include <QPainter>
#include <array>

constexpr int COLORS_PER_CHANNEL = 256; // QImage can only support up to 8-bit per channel

namespace ui
{
  class HistogramWidget : public QWidget
  {
      Q_OBJECT
    public:
      explicit HistogramWidget(QWidget *parent = nullptr);
      std::array<int, COLORS_PER_CHANNEL> values_{};
      QColor color_{Qt::white};
    protected:
      void paintEvent(QPaintEvent *event) override;
  };
}
#endif // HISTOGRAMWIDGET_H
