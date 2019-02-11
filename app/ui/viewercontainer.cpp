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
#include "viewercontainer.h"

#include <QWidget>
#include <QResizeEvent>
#include <QScrollBar>
#include <QVBoxLayout>

#include "viewerwidget.h"
#include "panels/viewer.h"
#include "project/sequence.h"
#include "debug.h"

// enforces aspect ratio
ViewerContainer::ViewerContainer(QWidget *parent) :
    QScrollArea(parent),
    fit(true),
    child(nullptr)
{
    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::NoFrame);

    area = new QWidget(this);
    area->move(0, 0);
    setWidget(area);

    child = new ViewerWidget(area);
    child->container = this;
}

ViewerContainer::~ViewerContainer() {
    delete child;
    delete area;
}

void ViewerContainer::dragScrollPress(const QPoint &p) {
    drag_start_x = p.x();
    drag_start_y = p.y();
    horiz_start = horizontalScrollBar()->value();
    vert_start = verticalScrollBar()->value();
}

void ViewerContainer::dragScrollMove(const QPoint &p) {
    int true_x = p.x() + (horiz_start - horizontalScrollBar()->value());
    int true_y = p.y() + (vert_start - verticalScrollBar()->value());

    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + (drag_start_x - true_x));
    verticalScrollBar()->setValue(verticalScrollBar()->value() + (drag_start_y - true_y));

    drag_start_x = true_x;
    drag_start_y = true_y;
}

void ViewerContainer::adjust() {
    if (auto sqn = viewer->getSequence()) {
        if (child->waveform) {
            child->move(0, 0);
            child->resize(size());
        } else if (fit) {
            const auto aspect_ratio = static_cast<double>(sqn->width())/static_cast<double>(sqn->height());

            auto widget_x = 0;
            auto widget_y = 0;
            auto widget_width = width();
            auto widget_height = height();
            const auto widget_ar = static_cast<double>(widget_width) / static_cast<double>(widget_height);

            const bool widget_is_wider_than_sequence = widget_ar > aspect_ratio;

            if (widget_is_wider_than_sequence) {
                widget_width = widget_height * aspect_ratio;
                widget_x = (width() / 2) - (widget_width / 2);
            } else {
                widget_height = widget_width / aspect_ratio;
                widget_y = (height() / 2) - (widget_height / 2);
            }

            child->move(widget_x, widget_y);
            child->resize(widget_width, widget_height);

            zoom = static_cast<double>(widget_width) / static_cast<double>(sqn->width());
        } else {
            const auto zoomed_width = qRound(viewer->getSequence()->width() * zoom);
            const auto zoomed_height = qRound(viewer->getSequence()->height() * zoom);
            auto zoomed_x = 0;
            auto zoomed_y = 0;

            if (zoomed_width < width()) {
                zoomed_x = (width()>>1)-(zoomed_width>>1);
            }
            if (zoomed_height < height()) {
                zoomed_y = (height()>>1)-(zoomed_height>>1);
            }

            child->move(zoomed_x, zoomed_y);
            child->resize(zoomed_width, zoomed_height);
        }
    }
    area->resize(qMax(width(), child->width()), qMax(height(), child->height()));
}

void ViewerContainer::resizeEvent(QResizeEvent *event) {
    event->accept();
    adjust();
}
