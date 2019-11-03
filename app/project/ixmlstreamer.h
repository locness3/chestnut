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

#ifndef IXMLSTREAMER_H
#define IXMLSTREAMER_H

#include <QXmlStreamReader>
#include "debug.h"

namespace chestnut {
  [[noreturn]] inline void throwAndLog(const char* const msg) {
    qCritical() << msg;
    throw std::runtime_error(msg);
  }
}

namespace project {
  class IXMLStreamer {
    public:
      IXMLStreamer() {}
      virtual ~IXMLStreamer() {}
      /**
       * Load an xml entry as the instance
       * @param stream xml entry
       */
      virtual bool load(QXmlStreamReader& stream) = 0;
      /**
       * Save this instance as an xml entry
       * @param stream
       * @return  true==saved successfully
       */
      virtual bool save(QXmlStreamWriter& stream) const = 0;
  };
}

#endif // IXMLSTREAMER_H
