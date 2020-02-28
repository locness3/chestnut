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

#include "project/undo.h"

using chestnut::ui::ViewerTimeline;

ViewerTimeline::ViewerTimeline(QWidget* parent)
  : QWidget(parent),
    fm_(font())
{

}

void ViewerTimeline::setViewedItem(std::weak_ptr<project::ProjectItem> item)
{
  viewed_item_ = std::move(item);
}

void ViewerTimeline::setInPoint(const long pos)
{
  auto item = viewed_item_.lock();
  if (item == nullptr)  {
    return;
  }
  auto new_in = pos;
  auto new_out = item->outPoint();
  if (new_out == new_in) {
    new_in--;
  } else if (new_out < new_in) {
    new_out = item->endFrame();
  }


  e_undo_stack.push(new SetTimelinePositionsCommand(item, true, new_in, new_out));
}

void ViewerTimeline::setOutPoint(const long pos)
{
  // TODO:
}

void ViewerTimeline::showText(const bool enable)
{
  // TODO:
}
double ViewerTimeline::zoom() const noexcept
{
  // TODO:
}

void ViewerTimeline::deleteMarkers()
{
  // TODO:
}
void ViewerTimeline::setScrollbarMax(QScrollBar* bar, const long end_frame, const int offset)
{
  // TODO:
}

void ViewerTimeline::updateZoom(const double value)
{
  // TODO:
}

void ViewerTimeline::setScroll(const int value)
{
  // TODO:
}

void ViewerTimeline::setVisibleIn(const long pos)
{
  // TODO:
}

void ViewerTimeline::showContextMenu(const QPoint &pos)
{
  // TODO:
}

void ViewerTimeline::resizedScrollListener(const double d)
{
  // TODO:
}

void ViewerTimeline::paintEvent(QPaintEvent* event)
{
  // TODO:
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

int ViewerTimeline::getHeaderScreenPointFromFrame(const long frame)
{
  // TODO:
}
