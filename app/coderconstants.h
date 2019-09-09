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

#ifndef CODERCONSTANTS_H
#define CODERCONSTANTS_H

/**
 * Various constants for encoders and decoders used project-wide
 */

constexpr auto MPEG2_SIMPLE_PROFILE = "SP";
constexpr auto MPEG2_MAIN_PROFILE = "MP";
constexpr auto MPEG2_HIGH_PROFILE = "HP";
constexpr auto MPEG2_422_PROFILE = "422";

constexpr auto MPEG2_LOW_LEVEL = "LL";
constexpr auto MPEG2_MAIN_LEVEL = "ML";
constexpr auto MPEG2_HIGH1440_LEVEL = "H-14";
constexpr auto MPEG2_HIGH_LEVEL = "HL";

constexpr auto X264_MINIMUM_CRF = 0;
constexpr auto X264_MAXIMUM_CRF = 51;



#endif // CODERCONSTANTS_H
