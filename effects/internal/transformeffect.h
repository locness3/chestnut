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
#ifndef TRANSFORMEFFECT_H
#define TRANSFORMEFFECT_H

#include "project/effect.h"

class TransformEffect : public Effect {
	Q_OBJECT
public:
    TransformEffect(ClipPtr c, const EffectMeta* em);
    virtual ~TransformEffect();
	void refresh();
	void process_coords(double timecode, GLTextureCoords& coords, int data);

    virtual void gizmo_draw(double timecode, GLTextureCoords& coords);
public slots:
	void toggle_uniform_scale(bool enabled);
private:
    EffectFieldPtr position_x = nullptr;
    EffectFieldPtr position_y = nullptr;
    EffectFieldPtr scale_x = nullptr;
    EffectFieldPtr scale_y = nullptr;
    EffectFieldPtr uniform_scale_field = nullptr;
    EffectFieldPtr rotation = nullptr;
    EffectFieldPtr anchor_x_box = nullptr;
    EffectFieldPtr anchor_y_box = nullptr;
    EffectFieldPtr opacity = nullptr;
    EffectFieldPtr blend_mode_box = nullptr;

    EffectGizmoPtr top_left_gizmo = nullptr;
    EffectGizmoPtr top_center_gizmo = nullptr;
    EffectGizmoPtr top_right_gizmo = nullptr;
    EffectGizmoPtr bottom_left_gizmo = nullptr;
    EffectGizmoPtr bottom_center_gizmo = nullptr;
    EffectGizmoPtr bottom_right_gizmo = nullptr;
    EffectGizmoPtr left_center_gizmo = nullptr;
    EffectGizmoPtr right_center_gizmo = nullptr;
    EffectGizmoPtr anchor_gizmo = nullptr;
    EffectGizmoPtr rotate_gizmo = nullptr;
    EffectGizmoPtr rect_gizmo = nullptr;

	bool set;
};

#endif // TRANSFORMEFFECT_H
