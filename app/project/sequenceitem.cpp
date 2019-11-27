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

#include "sequenceitem.h"

using project::SequenceItem;

SequenceItem::SequenceItem()
  : name_(),
    type_(project::SequenceItemType::NONE)
{

}

SequenceItem::SequenceItem(const project::SequenceItemType sequenceType)
  : name_(),
    type_(sequenceType)
{

}


void SequenceItem::setName(QString name)
{
  //TODO: input validation
  name_ = std::move(name);
}

QString SequenceItem::name() const
{
  return name_;
}


project::SequenceItemType SequenceItem::type() const noexcept
{
  return type_;
}
