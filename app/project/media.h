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
#include "project/ixmlstreamer.h"


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


class Media : public std::enable_shared_from_this<Media>, public project::IXMLStreamer
{
public:
    Media();
    explicit Media(const MediaPtr& iparent);

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
    bool setFootage(const FootagePtr& ftg);
    bool setSequence(const SequencePtr& sqn);
    void setFolder();
    void setIcon(const QIcon &ico);
    void setParent(const MediaWPtr& p);
    void updateTooltip(const QString& error = nullptr);
    MediaType type() const;
    const QString& name() const;
    void setName(const QString& n);

    double frameRate(const int32_t stream = -1);
    int32_t samplingRate(const int32_t stream = -1);

    // item functions
    void appendChild(const MediaPtr& child);
    bool setData(const int32_t column, const QVariant &value);
    MediaPtr child(const int32_t row);
    int32_t childCount() const;
    int32_t columnCount() const;
    QVariant data(const int32_t column, const int32_t role);
    int32_t row();
    MediaPtr parentItem();
    void removeChild(const int32_t index);

    virtual bool load(QXmlStreamReader& stream) override;
    virtual bool save(QXmlStreamWriter& stream) const override;

    //FIXME: varnames
    int32_t temp_id = 0;  // folder id
    int32_t temp_id2 = 0; // folder parent id

protected:
    static int32_t nextID;
private:
    friend class MediaTest;
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

    bool loadAsFolder(QXmlStreamReader& stream);
    bool loadAsSequence(QXmlStreamReader& stream);
    bool loadAsFootage(QXmlStreamReader& stream);

    bool saveAsFolder(QXmlStreamWriter& stream) const;
};

#endif // MEDIA_H
