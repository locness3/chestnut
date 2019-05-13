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

#ifndef COLORCONVERSIONS_H
#define COLORCONVERSIONS_H

#include <QRgb>

//FIXME: remove all of this

constexpr double LUMA_RED_COEFF = 0.2126;
constexpr double LUMA_GREEN_COEFF = 0.7152;
constexpr double LUMA_BLUE_COEFF = 0.0722;


namespace io
{
  namespace color_conversion
  {
    int32_t rgbToLuma(const QRgb clr);
  }
}

#endif // COLORCONVERSIONS_H
