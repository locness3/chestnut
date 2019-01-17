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
#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H

#include "project/effect.h"

#include <QFont>
#include <QImage>
class QOpenGLTexture;

class TextEffect : public Effect {
	Q_OBJECT
public:
    TextEffect(ClipPtr c, const EffectMeta *em);
	void redraw(double timecode);

    EffectFieldPtr text_val;
    EffectFieldPtr size_val;
    EffectFieldPtr set_color_button;
    EffectFieldPtr set_font_combobox;
    EffectFieldPtr halign_field;
    EffectFieldPtr valign_field;
    EffectFieldPtr word_wrap_field;

    EffectFieldPtr outline_bool;
    EffectFieldPtr outline_width;
    EffectFieldPtr outline_color;

    EffectFieldPtr shadow_bool;
    EffectFieldPtr shadow_distance;
    EffectFieldPtr shadow_color;
    EffectFieldPtr shadow_softness;
    EffectFieldPtr shadow_opacity;
private slots:
	void outline_enable(bool);
	void shadow_enable(bool);
	void text_edit_menu();
	void open_text_edit();
private:
    QFont font;
};

#endif // TEXTEFFECT_H
