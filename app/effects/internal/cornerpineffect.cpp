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
#include "cornerpineffect.h"

#include "io/path.h"
#include "project/clip.h"
#include "debug.h"

constexpr const char* const VERT_PATH = "cornerpin.vert";
constexpr const char* const FRAG_PATH = "cornerpin.frag";

CornerPinEffect::CornerPinEffect(ClipPtr c, const EffectMeta& em)
  : Effect(c, em)
{
  setCapability(Capability::COORDS);
  setCapability(Capability::SHADER);
  glsl_.vert_ = VERT_PATH;
  glsl_.frag_ = FRAG_PATH;
}

void CornerPinEffect::process_coords(double timecode, GLTextureCoords &coords, int)
{
  coords.vertices_[0].x_ += top_left_x->get_double_value(timecode);
  coords.vertices_[0].y_ += top_left_y->get_double_value(timecode);

  coords.vertices_[1].x_ += top_right_x->get_double_value(timecode);
  coords.vertices_[1].y_ += top_right_y->get_double_value(timecode);

  coords.vertices_[3].x_ += bottom_left_x->get_double_value(timecode);
  coords.vertices_[3].y_ += bottom_left_y->get_double_value(timecode);

  coords.vertices_[2].x_ += bottom_right_x->get_double_value(timecode);
  coords.vertices_[2].y_ += bottom_right_y->get_double_value(timecode);
}

void CornerPinEffect::process_shader(double timecode, GLTextureCoords &coords, const int /*iteration*/)
{
  glsl_.program_->setUniformValue("p0", static_cast<GLfloat>(coords.vertices_[3].x_), static_cast<GLfloat>(coords.vertices_[3].y_));
  glsl_.program_->setUniformValue("p1", static_cast<GLfloat>(coords.vertices_[2].x_), static_cast<GLfloat>(coords.vertices_[2].y_));
  glsl_.program_->setUniformValue("p2", static_cast<GLfloat>(coords.vertices_[0].x_), static_cast<GLfloat>(coords.vertices_[0].y_));
  glsl_.program_->setUniformValue("p3", static_cast<GLfloat>(coords.vertices_[1].x_), static_cast<GLfloat>(coords.vertices_[1].y_));
  glsl_.program_->setUniformValue("perspective", perspective->get_bool_value(timecode));
}

void CornerPinEffect::gizmo_draw(double, GLTextureCoords &coords)
{
  top_left_gizmo->world_pos[0] = QPoint(coords.vertices_[0].x_, coords.vertices_[0].y_);
  top_right_gizmo->world_pos[0] = QPoint(coords.vertices_[1].x_, coords.vertices_[1].y_);
  bottom_right_gizmo->world_pos[0] = QPoint(coords.vertices_[2].x_, coords.vertices_[2].y_);
  bottom_left_gizmo->world_pos[0] = QPoint(coords.vertices_[3].x_, coords.vertices_[3].y_);
}

void CornerPinEffect::setupUi()
{
  if (ui_setup) {
    return;
  }
  Effect::setupUi();
  EffectRowPtr top_left = add_row(tr("Top Left"));
  top_left_x = top_left->add_field(EffectFieldType::DOUBLE, "topleftx");
  top_left_y = top_left->add_field(EffectFieldType::DOUBLE, "toplefty");

  EffectRowPtr top_right = add_row(tr("Top Right"));
  top_right_x = top_right->add_field(EffectFieldType::DOUBLE, "toprightx");
  top_right_y = top_right->add_field(EffectFieldType::DOUBLE, "toprighty");

  EffectRowPtr bottom_left = add_row(tr("Bottom Left"));
  bottom_left_x = bottom_left->add_field(EffectFieldType::DOUBLE, "bottomleftx");
  bottom_left_y = bottom_left->add_field(EffectFieldType::DOUBLE, "bottomlefty");

  EffectRowPtr bottom_right = add_row(tr("Bottom Right"));
  bottom_right_x = bottom_right->add_field(EffectFieldType::DOUBLE, "bottomrightx");
  bottom_right_y = bottom_right->add_field(EffectFieldType::DOUBLE, "bottomrighty");

  perspective = add_row(tr("Perspective"))->add_field(EffectFieldType::BOOL, "perspective");
  perspective->set_bool_value(true);

  top_left_gizmo = add_gizmo(GizmoType::DOT);
  top_left_gizmo->x_field1 = top_left_x;
  top_left_gizmo->y_field1 = top_left_y;

  top_right_gizmo = add_gizmo(GizmoType::DOT);
  top_right_gizmo->x_field1 = top_right_x;
  top_right_gizmo->y_field1 = top_right_y;

  bottom_left_gizmo = add_gizmo(GizmoType::DOT);
  bottom_left_gizmo->x_field1 = bottom_left_x;
  bottom_left_gizmo->y_field1 = bottom_left_y;

  bottom_right_gizmo = add_gizmo(GizmoType::DOT);
  bottom_right_gizmo->x_field1 = bottom_right_x;
  bottom_right_gizmo->y_field1 = bottom_right_y;

  Effect::setupUiWithLoadStore();
}
