/*
 * Chestnut. Chestnut is a free non-linear video editor.
 * Copyright (C) 2020 Jon Noble
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

#ifndef VIEWERTIMELINE_H
#define VIEWERTIMELINE_H

#include <QWidget>
#include <QScrollBar>

#include "project/projectitem.h"
#include "ui/timelinebase.h"

namespace chestnut::ui
{
  class ViewerTimeline : public QWidget, public TimelineBase
  {
      Q_OBJECT
    public:
      explicit ViewerTimeline(QWidget* parent = nullptr);

      void setViewedItem(std::weak_ptr<project::ProjectItem> item);
      void setInPoint(const int64_t pos);
      void setOutPoint(const int64_t pos);
      void showText(const bool enable);
      double zoom() const noexcept;
      void deleteMarkers();
      void setScrollbarMax(QScrollBar& bar, const int64_t end_frame, const int offset);

    public slots:
      void setZoom(const double value);
      void setScroll(const int value);
      void setVisibleIn(const long pos);
      void showContextMenu(const QPoint &pos);
      void resizedScrollListener(const double value);

    signals:
      void updateParents();

    protected:
      void paintEvent(QPaintEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void focusOutEvent(QFocusEvent* event) override;

    private:
      bool dragging_ {false};
      bool resizing_workarea_ {false};
      bool resizing_workarea_in_ {false};
      int64_t temp_workarea_in_ {0};
      int64_t temp_workarea_out_ {0};
      int64_t item_end_ {0};
      double zoom_ {-1};
      long in_visible_ {0};
      QFontMetrics fm_;
      int drag_start_ {0};
      bool dragging_markers_ {false};
      QVector<int> selected_markers_;
      QVector<long> selected_marker_original_times_;
      int scroll_ {0};
      int height_actual_ {0};
      bool text_enabled_ {false};
    private:
      void setPlayhead(const int mouse_x);
      int64_t getHeaderFrameFromScreenPoint(const int x);
      int getHeaderScreenPointFromFrame(const int64_t frame);
  };
}

#endif // VIEWERTIMELINE_H
