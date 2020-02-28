/*
 * Chestnut. Chestnut is a free non-linear video editor.
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

#ifndef TRACK_H
#define TRACK_H

#include "project/ixmlstreamer.h"

namespace chestnut::project
{
  class Track : public IXMLStreamer
  {
    public:
      Track() = default;
      explicit Track(const int index);

      virtual bool load(QXmlStreamReader& stream) override;
      virtual bool save(QXmlStreamWriter& stream) const override;

      bool enabled_{true};
      bool locked_{false};
      QString name_;
      int index_;
    private:
  };
}

#endif // TRACK_H
