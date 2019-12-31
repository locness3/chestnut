/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019 Jonathan Noble
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

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <mediahandling/types.h>
#include <QAudioFormat>

namespace chestnut::defaults
{
  namespace audio
  {
    constexpr auto INTERNAL_FREQUENCY = 48000U;
    // get white-noise with 32bit
    constexpr auto INTERNAL_DEPTH = 16U;
    constexpr auto INTERNAL_FORMAT = media_handling::SampleFormat::SIGNED_16;
    constexpr auto INTERNAL_TYPE = QAudioFormat::SignedInt;
    constexpr auto INTERNAL_CODEC = "audio/pcm";
  }
  namespace video
  {
    constexpr auto INTERNAL_FORMAT = media_handling::PixelFormat::RGBA;
  }
}

#endif // DEFAULTS_H
