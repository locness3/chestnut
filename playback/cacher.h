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
#ifndef CACHER_H
#define CACHER_H

#include <QThread>
#include <QVector>
#include <memory>

#include "project/clip.h"

class Cacher : public QThread
{
//	Q_OBJECT
public:
    Cacher(ClipPtr c);
    bool caching = false;

	// must be set before caching
    long playhead = -1;
    bool reset = false;
    bool scrubbing = false;
    bool interrupt = false;
    QVector<ClipPtr> nests = QVector<ClipPtr>();

protected:
    virtual void run();

private:
    ClipPtr clip;
};
typedef std::shared_ptr<Cacher> CacherPtr;


#endif // CACHER_H
