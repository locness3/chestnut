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

#ifndef CURSOR_H
#define CURSOR_H

#include <memory>
#include <QCursor>
#include <QString>

namespace chestnut
{
  namespace ui
  {
    constexpr int CURSOR_SIZE = 24;

    enum class CursorType {
      LEFT_TRIM,
      RIGHT_TRIM,
      LEFT_RIPPLE,
      RIGHT_RIPPLE,
      SLIP,
      RAZOR,
      TRACK_SELECT,
      UNKNOWN
    };

    struct CursorNotImpl : public std::exception
    {
      const char * what () const noexcept
        {
          return "No Cursor available for this type";
        }
    };

    class Cursor
    {
      public:
        static QCursor get(const CursorType type);

        explicit Cursor(const CursorType type);


      private:
        static QMap<CursorType, std::shared_ptr<Cursor>> cursors_;
        QCursor cursor_{};
        CursorType type_{CursorType::UNKNOWN};

        QString filePath() const noexcept(false);
        void load();

    };
  }
}


#endif // CURSOR_H
