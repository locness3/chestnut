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
#ifndef DEBUG_H
#define DEBUG_H

#include <QDebug>
#include <QDateTime>

#ifndef QT_DEBUG
#define dout debug_out << "\n"
extern QDebug debug_out;
#else
#define dout qDebug() << QDateTime().currentDateTimeUtc().toString(Qt::ISODate)
#ifdef __GNUC__
#define dwarning dout << "[WARNING]" << __builtin_FILE () << __builtin_LINE () << __builtin_FUNCTION () << ":"
#define dinfo dout << "[INFO]" << __builtin_FILE () << __builtin_LINE () << __builtin_FUNCTION () << ":"
#define derror dout << "[ERROR]" << __builtin_FILE () << __builtin_LINE () << __builtin_FUNCTION ()  << ":"
#define dverbose dout << "[DEBUG]" << __builtin_FILE () << __builtin_LINE () << __builtin_FUNCTION ()  << ":"
#else
#define dwarning dout << "[WARNING]" << ":"
#define dinfo dout << "[INFO]" << ":"
#define derror dout << "[ERROR]" << ":"
#define dverbose dout << "[DEBUG]" << ":"
#endif
#endif


void setup_debug();
void close_debug();

#endif // DEBUG_H
