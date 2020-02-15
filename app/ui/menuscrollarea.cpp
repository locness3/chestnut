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
#include <map>

#include "debug.h"

using chestnut::ui::MenuScrollArea;

namespace
{
  std::vector<std::pair<QString, double>> ZOOM_OPTIONS =
  {
    {"Fit", -1},
    {"10%", 0.1},
    {"25%", 0.25},
    {"50%", 0.5},
    {"75%", 0.75},
    {"100%", 1.0},
    {"150%", 1.5},
    {"200%", 2.0},
    {"400%", 4.0}
  };
}

MenuScrollArea::MenuScrollArea(QWidget* parent)
  : QScrollArea(parent),
    zoom_level_(ZOOM_OPTIONS[0].second)
{
  setContextMenuPolicy(Qt::CustomContextMenu);
}


void MenuScrollArea::enableMenu(const bool enable)
{
  if (enable) {
    connect(this, &QWidget::customContextMenuRequested, this, &MenuScrollArea::showContextMenu);
  } else {
    disconnect(this, &QWidget::customContextMenuRequested, this, &MenuScrollArea::showContextMenu);
  }
}


void MenuScrollArea::showContextMenu(const QPoint& /*point*/)
{
  QMenu menu(this);
  QMenu zoom_menu(tr("Zoom"));
  for (const auto& [text, val] : ZOOM_OPTIONS) {
    const auto prefix = qFuzzyCompare(val, zoom_level_) ? "* " : "  ";
    zoom_menu.addAction(prefix + text)->setData(val);
  }

  connect(&zoom_menu, &QMenu::triggered, this, &MenuScrollArea::setMenuZoom);
  menu.addMenu(&zoom_menu);
  menu.addAction(tr("Close Media"), this, &MenuScrollArea::clearArea);
  menu.exec(QCursor::pos());
}


void MenuScrollArea::setMenuZoom(QAction* action)
{
  Q_ASSERT(action);
  bool ok;
  const auto zoom = action->data().toDouble(&ok);
  Q_ASSERT(ok);
  zoom_level_ = zoom;
  emit setZoom(zoom);
}


void MenuScrollArea::clearArea()
{
  emit clear();
}
