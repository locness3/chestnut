/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
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

#include "objectclip.h"

ObjectClip::ObjectClip(const Clip& base) :
  Clip(base.sequence)
{
  id_ = base.id_;
  transition_ = base.transition_;
  timeline_info = base.timeline_info;
  linked = base.linked;
  effects = base.effects;
  setName(base.name());
}

ObjectClip::ObjectClip(chestnut::project::SequencePtr seq) : Clip(std::move(seq))
{

}

bool ObjectClip::mediaOpen() const
{
  return true;
}


void ObjectClip::frame(const long /*playhead*/, bool& texture_failed)
{
  // Do nothing
  texture_failed = false;
}

bool ObjectClip::isCreatedObject() const
{
  return true;
}
