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
#ifndef KEYFRAME_H
#define KEYFRAME_H

#include <QVariant>
#include "project/ixmlstreamer.h"

enum class KeyframeType{
  LINEAR = 0,
  BEZIER,
  HOLD,
  UNKNOWN,
  MIXED
};

class EffectField;


class EffectKeyframe : public project::IXMLStreamer {
  public:
    EffectKeyframe() = default;
    explicit EffectKeyframe(const EffectField* const parent);

    long time{-1};
    KeyframeType type{KeyframeType::UNKNOWN};
    QVariant data;

    // only for bezier type
    double pre_handle_x = -40;
    double pre_handle_y = 0;
    double post_handle_x = 40;
    double post_handle_y = 0;

    virtual bool load(QXmlStreamReader& stream) override;
    virtual bool save(QXmlStreamWriter& stream) const override;
private:
    const EffectField* parent_; //TODO: remove this
};

void delete_keyframes(QVector<EffectField*>& selected_key_fields, QVector<int> &selected_keys);

#endif // KEYFRAME_H
