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
#ifndef CORNERPINEFFECT_H
#define CORNERPINEFFECT_H

#include "project/effect.h"

class CornerPinEffect : public Effect {
    Q_OBJECT
public:
    CornerPinEffect(ClipPtr c, const EffectMeta* em);
    void process_coords(double timecode, GLTextureCoords& coords, int data);
	void process_shader(double timecode, GLTextureCoords& coords);
    void gizmo_draw(double timecode, GLTextureCoords& coords);
private:
    EffectFieldPtr top_left_x;
    EffectFieldPtr top_left_y;
    EffectFieldPtr top_right_x;
    EffectFieldPtr top_right_y;
    EffectFieldPtr bottom_left_x;
    EffectFieldPtr bottom_left_y;
    EffectFieldPtr bottom_right_x;
    EffectFieldPtr bottom_right_y;
    EffectFieldPtr perspective;

    EffectGizmoPtr top_left_gizmo;
    EffectGizmoPtr top_right_gizmo;
    EffectGizmoPtr bottom_left_gizmo;
    EffectGizmoPtr bottom_right_gizmo;
};

#endif // CORNERPINEFFECT_H
