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
#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QVector>
#include "project/sequenceitem.h"
#include "project/transition.h"


constexpr int CLIPBOARD_TYPE_CLIP = 0;
constexpr int CLIPBOARD_TYPE_EFFECT = 1;

extern int e_clipboard_type;
extern QVector<TransitionPtr> e_clipboard_transitions;
extern QVector<project::SequenceItemPtr> e_clipboard;
void clear_clipboard();

#endif // CLIPBOARD_H
