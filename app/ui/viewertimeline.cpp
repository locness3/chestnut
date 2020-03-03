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

#include "project/undo.h"


constexpr int SUBLINE_MIN_PADDING = 50; //TODO: play with this
constexpr int LINE_MIN_PADDING = 50;
constexpr int MARKER_SIZE = 4;
constexpr int MARKER_OUTLINE_WIDTH = 3;
constexpr int PLAYHEAD_SIZE = 6;


using chestnut::ui::ViewerTimeline;
using chestnut::project::MarkerPtr;

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
  updateParents();
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
  updateParents();
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
  updateParents();
}

void ViewerTimeline::setScrollbarMax(QScrollBar& bar, const long end_frame, const int offset)
{
  bar.setMaximum(qMax(0, getScreenPointFromFrame(zoom_, end_frame) - offset));
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
  // TODO:
}

void ViewerTimeline::mouseMoveEvent(QMouseEvent* event)
{
  // TODO:
}

void ViewerTimeline::mouseReleaseEvent(QMouseEvent* event)
{
  // TODO:
}

void ViewerTimeline::focusOutEvent(QFocusEvent* event)
{
  // TODO:
}

void ViewerTimeline::updateParents()
{
  // TODO:
}

void ViewerTimeline::setPlayhead(const int mouse_x)
{
  // TODO:
}

long ViewerTimeline::getHeaderFrameFromScreenPoint(const int x)
{
  // TODO:
}

int ViewerTimeline::getHeaderScreenPointFromFrame(const int64_t frame)
{
  // TODO:
}
