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
#include "resizablescrollbar.h"

#include <QMouseEvent>
#include <QStyle>
#include <QStyleOptionSlider>

#include "debug.h"

constexpr int RESIZE_HANDLE_SIZE = 10;
constexpr int SCROLL_STEP_SIZE = 20;

ResizableScrollBar::ResizableScrollBar(QWidget *parent) :
  QScrollBar(parent)
{
  setSingleStep(SCROLL_STEP_SIZE);
  setMaximum(0);
  setMouseTracking(true);
}

bool ResizableScrollBar::is_resizing() const noexcept
{
  return resize_proc;
}

void ResizableScrollBar::resizeEvent(QResizeEvent *event)
{
  Q_ASSERT(event);
  setPageStep(event->size().width());
}

void ResizableScrollBar::mousePressEvent(QMouseEvent *event)
{
  if (resize_init) {
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    const QRect sr(style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                           QStyle::SC_ScrollBarSlider, this));

    resize_proc = true;
    Q_ASSERT(event);
    resize_start = event->pos().x();

    resize_start_max = maximum();
    resize_start_width = sr.width();
  } else {
    QScrollBar::mousePressEvent(event);
  }
}

void ResizableScrollBar::mouseMoveEvent(QMouseEvent *event)
{
  Q_ASSERT(event);
  QStyleOptionSlider opt;
  initStyleOption(&opt);

  const QRect sr(style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                         QStyle::SC_ScrollBarSlider, this));
  const QRect gr(style()->subControlRect(QStyle::CC_ScrollBar, &opt,
                                         QStyle::SC_ScrollBarGroove, this));

  if (resize_proc) {
    int diff = (event->pos().x() - resize_start);
    if (resize_top) {
      diff = -diff;
    }
    const double scale = static_cast<double>(sr.width()) / (sr.width() + diff);
    if (!qIsInf(scale) && !qIsNull(scale)) {
      emit resize_move(scale);
      resize_start = event->pos().x();

      if (resize_top) {
        const int slider_min = gr.x();
        const int slider_max = gr.right() - (sr.width() + diff);
        const int val = QStyle::sliderValueFromPosition(minimum(), maximum(),
                                                        event->pos().x() - slider_min,
                                                        slider_max - slider_min,
                                                        opt.upsideDown);
        setValue(val);
      } else {
        setValue(qRound(value() * scale));
      }
    }
  } else {
    bool new_resize_init = false;

    if (( (orientation() == Qt::Horizontal)
          && (event->pos().x() > (sr.left() - RESIZE_HANDLE_SIZE))
          && (event->pos().x() < (sr.left() + RESIZE_HANDLE_SIZE)) )
        || ( (orientation() == Qt::Vertical)
             && (event->pos().y() > (sr.top() - RESIZE_HANDLE_SIZE))
             && (event->pos().y() < (sr.top() + RESIZE_HANDLE_SIZE))) ) {
      new_resize_init = true;
      resize_top = true;
    } else if (( (orientation() == Qt::Horizontal)
                 && (event->pos().x() > (sr.right() - RESIZE_HANDLE_SIZE))
                 && (event->pos().x() < (sr.right() + RESIZE_HANDLE_SIZE)) )
               || ( (orientation() == Qt::Vertical)
                    && (event->pos().y() > (sr.bottom() - RESIZE_HANDLE_SIZE))
                    && (event->pos().y() < (sr.bottom() + RESIZE_HANDLE_SIZE)) ) ) {
      new_resize_init = true;
      resize_top = false;
    }

    if (resize_init != new_resize_init) {
      if (new_resize_init) {
        setCursor(Qt::SizeHorCursor);
      } else {
        unsetCursor();
      }
      resize_init = new_resize_init;
    }

    QScrollBar::mouseMoveEvent(event);
  }
}

void ResizableScrollBar::mouseReleaseEvent(QMouseEvent *event)
{
  if (resize_proc) {
    resize_proc = false;
  } else {
    QScrollBar::mouseReleaseEvent(event);
  }
}
