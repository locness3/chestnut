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

#include "projectitem.h"

using chestnut::project::ProjectItem;

ProjectItem::ProjectItem(const QString& itemName)
  : name_(itemName)
{

}

void ProjectItem::setName(const QString& val)
{
  name_ = val;
}
const QString& ProjectItem::name() const
{
  return name_;
}

bool ProjectItem::load(QXmlStreamReader& stream)
{
  return false;
}

bool ProjectItem::save(QXmlStreamWriter& stream) const
{
  return false;
}


void ProjectItem::setInPoint(const long pos)
{
  workarea_.in_point_ = pos;
}

long ProjectItem::inPoint() const noexcept
{
  return workarea_.in_point_;
}

void ProjectItem::setOutPoint(const long pos)
{
  workarea_.out_point_ = pos;
}

long ProjectItem::outPoint() const noexcept
{
  return workarea_.out_point_;
}

void ProjectItem::setWorkareaEnabled(const bool enabled)
{
  workarea_.enabled_ = enabled;
}

bool ProjectItem::workareaEnabled() const noexcept
{
  return workarea_.enabled_;
}

void ProjectItem::setWorkareaActive(const bool active)
{
  workarea_.active_ = active;
}

bool ProjectItem::workareaActive() const noexcept
{
  return workarea_.active_;
}

