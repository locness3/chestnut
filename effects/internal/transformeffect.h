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
    EffectField* position_x = NULL;
    EffectField* position_y = NULL;
    EffectField* scale_x = NULL;
    EffectField* scale_y = NULL;
    EffectField* uniform_scale_field = NULL;
    EffectField* rotation = NULL;
    EffectField* anchor_x_box = NULL;
    EffectField* anchor_y_box = NULL;
    EffectField* opacity = NULL;
    EffectField* blend_mode_box = NULL;

    EffectGizmo* top_left_gizmo = NULL;
    EffectGizmo* top_center_gizmo = NULL;
    EffectGizmo* top_right_gizmo = NULL;
    EffectGizmo* bottom_left_gizmo = NULL;
    EffectGizmo* bottom_center_gizmo = NULL;
    EffectGizmo* bottom_right_gizmo = NULL;
    EffectGizmo* left_center_gizmo = NULL;
    EffectGizmo* right_center_gizmo = NULL;
    EffectGizmo* anchor_gizmo = NULL;
    EffectGizmo* rotate_gizmo = NULL;
    EffectGizmo* rect_gizmo = NULL;

	bool set;
};

#endif // TRANSFORMEFFECT_H
