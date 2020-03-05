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

#include "viewertimeline.h"
#include <QMenu>
#include <QPainter>
#include <QMouseEvent>

#include "project/undo.h"
#include "panels/panelmanager.h"


constexpr int SUBLINE_MIN_PADDING = 50; //TODO: play with this
constexpr int LINE_MIN_PADDING = 50;
constexpr int MARKER_SIZE = 4;
constexpr int MARKER_OUTLINE_WIDTH = 3;
constexpr int PLAYHEAD_SIZE = 6;
constexpr int CLICK_RANGE = 5;


using chestnut::ui::ViewerTimeline;
using chestnut::project::MarkerPtr;
using panels::PanelManager;

ViewerTimeline::ViewerTimeline(QWidget* parent)
  : QWidget(parent),
    fm_(font())
{
  height_actual_ = fm_.height();
  setCursor(Qt::ArrowCursor);
  setMouseTracking(true);
  setFocusPolicy(Qt::ClickFocus);
  showText(true);

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(show_context_menu(const QPoint &)));
}

void ViewerTimeline::setViewedItem(std::weak_ptr<project::ProjectItem> item)
{
  viewed_item_ = std::move(item);
}

void ViewerTimeline::setInPoint(const int64_t pos)
{
  auto item = viewed_item_.lock();
  assert(item);
  auto new_in = pos;
  auto new_out = item->outPoint();
  if (new_out == new_in) {
    new_in--;
  } else if (new_out < new_in) {
    new_out = item->endFrame();
  }
  e_undo_stack.push(new SetTimelinePositionsCommand(viewed_item_, true, new_in, new_out));
  emit updateParents();
}

void ViewerTimeline::setOutPoint(const int64_t pos)
{
  auto item = viewed_item_.lock();
  assert(item);
  auto new_out = pos;
  auto new_in = item->inPoint();
  if (new_out == new_in) {
    new_out++;
  } else if ( (new_in > new_out) || (new_in < 0)) {
    new_in = 0;
  }

  e_undo_stack.push(new SetTimelinePositionsCommand(viewed_item_, true, new_in, new_out));
  emit updateParents();
}

void ViewerTimeline::showText(const bool enable)
{
  text_enabled_ = enable;
  const auto height = enable ? height_actual_ * 2 : height_actual_;
  setFixedHeight(height);
  update();
}

double ViewerTimeline::zoom() const noexcept
{
  return zoom_;
}

void ViewerTimeline::deleteMarkers()
{
  auto item = viewed_item_.lock();
  assert(item);
  if (selected_markers_.empty()) {
    return;
  }
  auto dma = new DeleteItemMarkerAction(viewed_item_);
  for (const auto marker : selected_markers_) {
    dma->addMarker(marker);
  }
  e_undo_stack.push(dma);
  emit updateParents();
}

void ViewerTimeline::setScrollbarMax(QScrollBar& bar, const int64_t end_frame, const int offset)
{
  const auto value = static_cast<int>(getScreenPointFromFrame(zoom_, end_frame) - offset);
  bar.setMaximum(qMax(0, value));
}

void ViewerTimeline::setZoom(const double value)
{
  zoom_ = value;
  update();
}

void ViewerTimeline::setScroll(const int value)
{
  scroll_ = value;
  update();
}

void ViewerTimeline::setVisibleIn(const long pos)
{
  in_visible_ = pos;
  update();
}

void ViewerTimeline::showContextMenu(const QPoint &pos)
{
  QMenu menu(this);
  MainWindow::instance().make_inout_menu(menu);
  menu.exec(mapToGlobal(pos));
}

void ViewerTimeline::resizedScrollListener(const double value)
{
  setZoom(zoom_ * value);
}

void ViewerTimeline::paintEvent(QPaintEvent* event)
{
  auto item = viewed_item_.lock();
  if ( (item == nullptr) || (zoom_ <= 0.0) ) {
    return;
  }

  QPainter p(this);
  const auto yoff = (text_enabled_) ? height() / 2 : 0;
  const auto interval = item->frameRate().toDouble();
  auto textWidth = 0;
  auto lastTextBoundary = INT_MIN;

  auto i = 0;
  auto lastLineX = INT_MIN;

  auto sublineCount = 1;
  auto sublineTest = qRound(interval * zoom_);
  auto sublineInterval = 1;
  while ( (sublineTest > SUBLINE_MIN_PADDING) && (sublineInterval >= 1) ) {
    sublineCount *= 2;
    sublineInterval = qRound(interval / sublineCount);
    sublineTest = qRound(sublineInterval * zoom_);
  }

  sublineCount = qMin(sublineCount, qRound(interval));

  int text_x;
  int fullTextWidth;
  QString timecode;

  while (true) {
    const auto frame = qRound(interval*i);
    const auto lineX = qRound(frame * zoom_) - scroll_;

    if (lineX > width()) {
      break;
    }

    // draw text
    bool draw_text = false;
    if (text_enabled_ && (lineX-textWidth > lastTextBoundary) ) {
      // FIXME:
//      timecode = frame_to_timecode(frame + in_visible_, global::config.timecode_view, sqn->frameRate());
//      fullTextWidth = fm.horizontalAdvance(timecode);
      textWidth = fullTextWidth >> 1;
      text_x = lineX - textWidth;
      lastTextBoundary = lineX + textWidth;
      if (lastTextBoundary >= 0) {
        draw_text = true;
      }
    }

    if (lineX > (lastLineX + LINE_MIN_PADDING) ) {
      if (draw_text) {
        p.setPen(Qt::white);
        p.drawText(QRect(text_x, 0, fullTextWidth, yoff), timecode);
      }

      // draw line markers
      p.setPen(Qt::gray);
      p.drawLine(lineX, yoff, lineX, height());

      // draw sub-line markers
      for (int j = 1; j < sublineCount; j++) {
        const auto sublineX = lineX + (qRound(j * interval / sublineCount) * zoom_);
        p.drawLine(sublineX, yoff, sublineX, yoff+(height()/4));
      }

      lastLineX = lineX;
    }

    // TODO wastes cycles here, could just bring it up to 0
    i++;
  }//while

  // draw in/out selection
  int in_x;
  if (item->workareaActive()) {
    in_x = getHeaderScreenPointFromFrame((resizing_workarea_ ? temp_workarea_in_ : item->inPoint()));
    const int out_x = getHeaderScreenPointFromFrame((resizing_workarea_ ? temp_workarea_out_ : item->outPoint()));
    p.fillRect(QRect(in_x, 0, out_x-in_x, height()), item->workareaEnabled() ? QColor(0, 192, 255, 128)
                                                                             : QColor(255, 255, 255, 64));
    p.setPen(Qt::white);
    p.drawLine(in_x, 0, in_x, height());
    p.drawLine(out_x, 0, out_x, height());
  }

  // draw markers
  for (auto j = 0; j < item->markers_.size(); ++j) {
    const MarkerPtr& m = item->markers_.at(j);
    if (m == nullptr) {
      continue;
    }
    const int marker_x = getHeaderScreenPointFromFrame(m->frame);
    const QPoint points[5] = {
      QPoint(marker_x, height() - 1),
      QPoint(marker_x + MARKER_SIZE, height() - MARKER_SIZE - 1),
      QPoint(marker_x + MARKER_SIZE, yoff),
      QPoint(marker_x - MARKER_SIZE, yoff),
      QPoint(marker_x - MARKER_SIZE, height() - MARKER_SIZE - 1)
    };

    bool selected = false;
    for (auto k = 0; k < selected_markers_.size(); ++k) {
      if (selected_markers_.at(k) == j) {
        selected = true;
        break;
      }
    }

    if (selected) {
      QColor invert;
      invert.setRed(255 - m->color_.red());
      invert.setGreen(255 - m->color_.green());
      invert.setBlue(255 - m->color_.blue());
      QPen pen(invert);
      pen.setWidth(MARKER_OUTLINE_WIDTH);
      p.setPen(pen);
    } else {
      p.setPen(m->color_);
    }
    p.setBrush(m->color_);
    p.drawPolygon(points, 5);
  }

  // draw playhead triangle
  p.setRenderHint(QPainter::Antialiasing);

  in_x = getHeaderScreenPointFromFrame(item->playhead());
  const QPoint start(in_x, height() + 2);
  QPainterPath path;
  path.moveTo(start + QPoint(1, 0));
  path.lineTo(in_x - PLAYHEAD_SIZE, yoff);
  path.lineTo(in_x + PLAYHEAD_SIZE + 1, yoff);
  path.lineTo(start);
  p.fillPath(path, Qt::red);
}

void ViewerTimeline::mousePressEvent(QMouseEvent* event)
{
  auto item = viewed_item_.lock();
  if (item == nullptr) {
    return;
  }

  if (event->buttons() & Qt::LeftButton) {
    if (resizing_workarea_) {
      item_end_ = item->endFrame();
    } else {
      MarkerPtr mark;
      const auto shift = (event->modifiers() & Qt::ShiftModifier);
      auto clicked_on_marker = false;
      for (auto i = 0; i < item->markers_.size(); i++) {
        mark = item->markers_.at(i);
        if (mark == nullptr) {
          continue;
        }
        const auto marker_pos = getHeaderScreenPointFromFrame(mark->frame);
        if ( (event->pos().x() > (marker_pos - MARKER_SIZE)) && (event->pos().x() < (marker_pos + MARKER_SIZE) )) {
          auto found = false;
          for (auto j = 0; j < selected_markers_.size(); j++) {
            if (selected_markers_.at(j) != i) {
              continue;
            }
            if (shift) {
              selected_markers_.removeAt(j);
            }
            found = true;
            break;
          }

          if (!found) {
            if (!shift) {
              selected_markers_.clear();
            }
            selected_markers_.append(i);
          }
          clicked_on_marker = true;
          update();
          break;
        }
      }//for

      if (clicked_on_marker) {
        selected_marker_original_times_.resize(selected_markers_.size());
        for (auto i = 0; i < selected_markers_.size(); ++i) {
          mark = item->markers_.at(selected_markers_.at(i));
          if (mark) {
            selected_marker_original_times_[i] = mark->frame;
          }
        }
        drag_start_ = event->pos().x();
        dragging_markers_ = true;
      } else {
        if (!selected_markers_.empty()) {
          selected_markers_.clear();
          update();
        }
        setPlayhead(event->pos().x());
      }
    }
    dragging_ = true;
  }
}

void ViewerTimeline::mouseMoveEvent(QMouseEvent* event)
{

  auto item = viewed_item_.lock();
  if (item == nullptr) {
    return;
  }
  if (dragging_) {
    if (resizing_workarea_) {
      auto frame = getHeaderFrameFromScreenPoint(event->pos().x());
      if (snap_.enabled_) {
        snapToTimeline(frame, true, true, false);
      }

      if (resizing_workarea_in_) {
        temp_workarea_in_ = qMax(qMin(temp_workarea_out_ - 1, frame), static_cast<int64_t>(0));
      } else {
        temp_workarea_out_ = qMin(qMax(temp_workarea_in_ + 1, frame), item_end_);
      }

      emit updateParents();
    } else if (dragging_markers_) {
      auto frame_movement = getHeaderFrameFromScreenPoint(event->pos().x()) - getHeaderFrameFromScreenPoint(drag_start_);

      // snap markers
      for (auto i = 0; i < selected_markers_.size(); i++) {
        auto fmv = selected_marker_original_times_.at(i) + frame_movement;
        if (snap_.enabled_ && snapToTimeline(fmv, true, false, true)) {
          frame_movement = fmv - selected_marker_original_times_.at(i);
          break;
        }
      }

      // validate markers (ensure none go below 0)
      long validator;
      for (auto i = 0; i < selected_markers_.size(); i++) {
        validator = selected_marker_original_times_.at(i) + frame_movement;
        if (validator < 0) {
          frame_movement -= validator;
        }
      }

      // move markers
      for (auto i = 0; i < selected_markers_.size(); ++i) {
        if (MarkerPtr mark = item->markers_.at(selected_markers_.at(i))) {
          mark->frame = selected_marker_original_times_.at(i) + frame_movement;
        }
      }

      emit updateParents();
    } else {
      setPlayhead(event->pos().x());
    }
  } else {
    resizing_workarea_ = false;
    unsetCursor();
    if (item->workareaActive()) {
      const auto min_frame = getHeaderFrameFromScreenPoint(event->pos().x() - CLICK_RANGE) - 1;
      const auto max_frame = getHeaderFrameFromScreenPoint(event->pos().x() + CLICK_RANGE) + 1;
      if ( (item->inPoint() > min_frame) && (item->inPoint() < max_frame) ) {
        resizing_workarea_ = true;
        resizing_workarea_in_ = true;
      } else if ( (item->outPoint() > min_frame) && (item->outPoint() < max_frame) ) {
        resizing_workarea_ = true;
        resizing_workarea_in_ = false;
      }
      if (resizing_workarea_) {
        temp_workarea_in_ = item->inPoint();
        temp_workarea_out_ = item->outPoint();
        setCursor(Qt::SizeHorCursor);
      }
    }
  }
}

void ViewerTimeline::mouseReleaseEvent(QMouseEvent* event)
{
  auto item = viewed_item_.lock();
  if (item == nullptr) {
    return;
  }
  dragging_ = false;
  if (resizing_workarea_) {
    e_undo_stack.push(new SetTimelinePositionsCommand(item, true, temp_workarea_in_, temp_workarea_out_));
  } else if (dragging_markers_ && !selected_markers_.empty()) {
    auto moved = false;
    auto ca = new ComboAction();
    for (auto i = 0; i < selected_markers_.size(); ++i) {
      if (auto m = item->markers_.at(selected_markers_.at(i))) {
        if (selected_marker_original_times_.at(i) != m->frame) {
          ca->append(new MoveMarkerAction(m, selected_marker_original_times_.at(i), m->frame));
          moved = true;
        }
      }
    }
    if (moved) {
      e_undo_stack.push(ca);
    } else {
      delete ca;
    }
  }

  resizing_workarea_ = false;
  dragging_ = false;
  dragging_markers_ = false;
  snap_.enabled_ = false;
  emit updateParents();
}

void ViewerTimeline::focusOutEvent(QFocusEvent* event)
{
  selected_markers_.clear();
  update();
}


void ViewerTimeline::setPlayhead(const int mouse_x)
{
  // TODO:
}

int64_t ViewerTimeline::getHeaderFrameFromScreenPoint(const int x)
{
  // TODO:
}

int ViewerTimeline::getHeaderScreenPointFromFrame(const int64_t frame)
{
  // TODO:
}
