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
#ifndef EFFECTGIZMO_H
#define EFFECTGIZMO_H

#define GIZMO_DOT_SIZE 2.5
#define GIZMO_TARGET_SIZE 5.0

#include <QString>
#include <QRect>
#include <QPoint>
#include <QVector>
#include <QColor>

#include <memory>

#include "project/effectfield.h"


enum class GizmoType {
    DOT = 0,
    POLY = 1,
    TARGET = 2
};

class EffectGizmo
{
public:
    explicit EffectGizmo(const GizmoType type);
    ~EffectGizmo();

    EffectGizmo(const EffectGizmo& ) = delete;
    EffectGizmo& operator=(const EffectGizmo&) = delete;

    QVector<QPoint> world_pos;
    QVector<QPoint> screen_pos;

    EffectField* x_field1;
    double x_field_multi1;
    EffectField* y_field1;
    double y_field_multi1;
    EffectField* x_field2;
    double x_field_multi2;
    EffectField* y_field2;
    double y_field_multi2;

    void set_previous_value();

    QColor color;
    int get_point_count();

    GizmoType get_type() const;

    int get_cursor() const;
    void set_cursor(const int value);
private:
    GizmoType _type;
    int cursor;
};

typedef std::shared_ptr<EffectGizmo> EffectGizmoPtr;

#endif // EFFECTGIZMO_H
