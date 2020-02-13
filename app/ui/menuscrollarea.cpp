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

#include "menuscrollarea.h"

#include <QMenu>

#include "debug.h"

using chestnut::ui::MenuScrollArea;

MenuScrollArea::MenuScrollArea(QWidget* parent) : QScrollArea(parent)
{

  setContextMenuPolicy(Qt::CustomContextMenu);
  connect(this, &QWidget::customContextMenuRequested, this, &MenuScrollArea::showContextMenu);
}

void MenuScrollArea::mousePressEvent(QMouseEvent* event)
{
  if (event->buttons() & ~Qt::MouseButton::RightButton) {
    return;
  }
  qDebug() << "Righty!";
  emit customContextMenuRequested(event->pos());
}


void MenuScrollArea::showContextMenu(const QPoint& /*point*/)
{
  QMenu menu(this);
  QMenu zoom_menu(tr("Zoom"));
  zoom_menu.addAction("Fit")->setData(-1);
  zoom_menu.addAction("10%")->setData(0.1);
  zoom_menu.addAction("25%")->setData(0.25);
  zoom_menu.addAction("50%")->setData(0.5);
  zoom_menu.addAction("75%")->setData(0.75);
  zoom_menu.addAction("100%")->setData(1.0);
  zoom_menu.addAction("150%")->setData(1.5);
  zoom_menu.addAction("200%")->setData(2.0);
  zoom_menu.addAction("400%")->setData(4.0);

  connect(&zoom_menu, &QMenu::triggered, this, &MenuScrollArea::setMenuZoom);
  menu.addMenu(&zoom_menu);
  menu.addAction(tr("Close Media"));//, viewer, SLOT(close_media()));
  menu.exec(QCursor::pos());
}


void MenuScrollArea::setMenuZoom(QAction* action)
{
  Q_ASSERT(action);
  bool ok;
  const auto zoom = action->data().toDouble(&ok);
  Q_ASSERT(ok);
  emit setZoom(zoom);
}
