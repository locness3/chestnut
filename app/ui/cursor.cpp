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

#include "cursor.h"

#include <QSvgRenderer>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include "debug.h"

using chestnut::ui::Cursor;
using chestnut::ui::CursorType;


QMap<CursorType, std::shared_ptr<Cursor>> Cursor::cursors_;

QCursor Cursor::get(const CursorType type)
{
  if (cursors_.contains(type)) {
    return cursors_[type]->cursor_;
  }

  try {
    cursors_[type] = std::make_shared<Cursor>(type);
    return cursors_[type]->cursor_;
  } catch (const CursorNotImpl& ex) {
    qDebug() << ex.what();
  }
  return QCursor(); // TODO: some null like cursor
}

Cursor::Cursor(const CursorType type) : type_(type)
{
  load();
}


QString Cursor::filePath() const
{
  switch (type_) {
    case CursorType::LEFT_TRIM :
      return ":/cursors/left_trim.svg";
    case CursorType::RIGHT_TRIM :
      return ":/cursors/right_trim.svg";
    case CursorType::LEFT_RIPPLE :
      return ":/cursors/left_ripple.svg";
    case CursorType::RIGHT_RIPPLE :
      return ":/cursors/right_ripple.svg";
    case CursorType::SLIP :
      return ":/cursors/slip.svg";
    case CursorType::RAZOR :
      return ":/cursors/razor.svg";
    case CursorType::LEFT_TRIM_TRANSITION :
      return ":/cursors/left_trim_transition.svg";
    case CursorType::RIGHT_TRIM_TRANSITION :
      return ":/cursors/right_trim_transition.svg";
    case CursorType::TRACK_SELECT :
      [[fallthrough]];
    case CursorType::UNKNOWN:
      [[fallthrough]];
    default:
      break;
  }
  throw chestnut::ui::CursorNotImpl();
}


void Cursor::load()
{
  const QString file(filePath());
  // load specified file into a pixmap
  QSvgRenderer renderer(file);

  const int cursor_size_dpiaware = chestnut::ui::CURSOR_SIZE * QApplication::desktop()->devicePixelRatio();
  QPixmap pixmap(cursor_size_dpiaware, cursor_size_dpiaware);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  renderer.render(&painter, pixmap.rect());

  // set cursor's horizontal hotspot
//  if (right_aligned) {
//    hotX = pixmap.width() - hotX;
//  }

  cursor_ = QCursor(pixmap);
}

