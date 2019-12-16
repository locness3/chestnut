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
#ifndef MARKER_H
#define MARKER_H

#include <QString>
#include <QColor>
#include <QXmlStreamReader>
#include <memory>

#include "project/ixmlstreamer.h"


class Marker : public project::IXMLStreamer {
  public:
    Marker() = default;

    bool operator<(const Marker& rhs) const;
    virtual bool load(QXmlStreamReader& stream) override;
    virtual bool save(QXmlStreamWriter& stream) const override;

    long frame{};
    double frame_rate_; // Needed to properly display timecode
    QString name{};
    long duration_{};
    QString comment_{};
    QColor color_{Qt::white};
    QString thumb_hash_; // TODO: create thumbnail and load
};

using MarkerPtr = std::shared_ptr<Marker>;
using MarkerWPtr = std::weak_ptr<Marker>;
#endif // MARKER_H
