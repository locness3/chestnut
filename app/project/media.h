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
        return std::dynamic_pointer_cast<T>(object_);
    }
    /**
     * @brief Obtain this instance unique-id
     * @return id
     */
    int32_t id() const;
    void clearObject();
    bool setFootage(FootagePtr ftg);
    bool setSequence(SequencePtr sqn);
    void setFolder();
    void setIcon(const QIcon &ico);
    void setParent(MediaWPtr p);
    void updateTooltip(const QString& error = 0);
    MediaType type() const;
    const QString& name();
    void setName(const QString& n);

    double frameRate(const int32_t stream = -1);
    int32_t samplingRate(const int32_t stream = -1);

    // item functions
    void appendChild(MediaPtr child);
    bool setData(const int32_t column, const QVariant &value);
    MediaPtr child(const int32_t row);
    int32_t childCount() const;
    int32_t columnCount() const;
    QVariant data(const int32_t column, const int32_t role);
    int32_t row();
    MediaPtr parentItem();
    void removeChild(const int32_t index);

    MediaThrobber* throbber;
    int32_t temp_id = 0;
    int32_t temp_id2 = 0;

protected:
    static int32_t nextID;
private:
    bool root_;
    MediaType type_ = MediaType::NONE;
    project::ProjectItemPtr object_;

    // item functions
    QVector<MediaPtr> children_;
    MediaWPtr parent_;
    QString folder_name_;
    QString tool_tip_;
    QIcon icon_;
    int32_t id_;

};

#endif // MEDIA_H
