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
#include <memory>

#include "project/projectitem.h"
#include "project/sequence.h"
#include "project/footage.h"


enum class MediaType {
    FOOTAGE = 0,
    SEQUENCE = 1,
    FOLDER = 2,
    NONE
};

struct UnhandledMediaTypeException: public std::exception
{
    const char* what() const noexcept
    {
        return "Media type is unhandled";
    }
};

class MediaThrobber;
class Media;



using MediaUPtr = std::unique_ptr<Media>;
using MediaPtr = std::shared_ptr<Media>;
using MediaWPtr = std::weak_ptr<Media>;


class Media : public std::enable_shared_from_this<Media>
{
public:

    Media();
    explicit Media(MediaPtr iparent);
    ~Media();

    Media(const Media& cpy) = delete;
    const Media& operator=(const Media& rhs) = delete;

    template<typename T>
    auto object() {
        return std::dynamic_pointer_cast<T>(heldObject);
    }
    /**
     * @brief Obtain this instance unique-id
     * @return id
     */
    int id() const;
    void clear_object();
    void set_footage(FootagePtr ftg);
    void set_sequence(SequencePtr sqn);
    void set_folder();
    void set_icon(const QIcon &ico);
    void set_parent(MediaWPtr p);
    void update_tooltip(const QString& error = 0);
    MediaType get_type() const;
    const QString& get_name();
    void set_name(const QString& n);

    double get_frame_rate(const int stream = -1);
    int get_sampling_rate(const int stream = -1);

    // item functions
    void appendChild(MediaPtr child);
    bool setData(int col, const QVariant &value);
    MediaPtr child(const int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column, int role);
    int row();
    MediaPtr parentItem();
    void removeChild(int i);

    MediaThrobber* throbber;
    int temp_id = 0;
    int temp_id2 = 0;

protected:
    static int nextID;
private:
    bool root;
    MediaType type = MediaType::NONE;
    project::ProjectItemPtr heldObject;

    // item functions
    QVector<MediaPtr> children;
    MediaWPtr parent;
    QString folder_name;
    QString tooltip;
    QIcon icon;
    int instanceId;

};

#endif // MEDIA_H
