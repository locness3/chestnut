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
#ifndef MEDIA_H
#define MEDIA_H

#include <QList>
#include <QVariant>
#include <QIcon>

#include "project/projectitem.h"
#include "project/sequence.h"
#include "project/footage.h"


enum MediaType_E {
    MEDIA_TYPE_FOOTAGE = 0,
    MEDIA_TYPE_SEQUENCE = 1,
    MEDIA_TYPE_FOLDER = 2,
    MEDIA_TYPE_NONE
};

class MediaThrobber;


class Media
{
public:
    explicit Media(Media* iparent);
    ~Media();

    template<typename T>
    std::shared_ptr<T> get_object() {
        return std::dynamic_pointer_cast<T>(object);
    }

    void clear_object();
    void set_footage(FootagePtr ftg);
    void set_sequence(SequencePtr sqn);
    void set_folder();
    void set_icon(const QIcon &ico);
    void set_parent(Media* p);
    void update_tooltip(const QString& error = 0);
    MediaType_E get_type() const;
    const QString& get_name();
    void set_name(const QString& n);

	double get_frame_rate(int stream = -1);
	int get_sampling_rate(int stream = -1);

    // item functions
    void appendChild(Media *child);
    bool setData(int col, const QVariant &value);
    Media *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column, int role);
    int row() const;
    Media *parentItem();
    void removeChild(int i);

    MediaThrobber* throbber;
    bool root;
    int temp_id = 0;
    int temp_id2 = 0;
private:
    MediaType_E type;
    project::ProjectItemPtr object;

    // item functions
    QList<Media*> children;
    Media* parent;
    QString folder_name;
    QString tooltip;
    QIcon icon;

    // Explicity impl as required
    Media();
    Media(const Media& cpy);
    const Media& operator=(const Media& rhs);
};

#endif // MEDIA_H
