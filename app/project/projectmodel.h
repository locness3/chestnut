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
#ifndef PROJECTMODEL_H
#define PROJECTMODEL_H

#include <QAbstractItemModel>

#include "project/media.h"
#include "project/ixmlstreamer.h"

class ProjectModel : public QAbstractItemModel, public chestnut::project::IXMLStreamer
{
    Q_OBJECT
public:
    explicit ProjectModel(QObject* parent = nullptr);
    virtual ~ProjectModel() override = default;

    ProjectModel(const ProjectModel&) = delete;
    ProjectModel(const ProjectModel&&) = delete;
    ProjectModel& operator=(const ProjectModel&) = delete;
    ProjectModel& operator=(const ProjectModel&&) = delete;

    void destroy_root();
    void clear();
    chestnut::project::MediaPtr root();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    chestnut::project::MediaPtr getItem(const QModelIndex &index) const;

    bool appendChild(chestnut::project::MediaPtr parent, const chestnut::project::MediaPtr& child);
    void moveChild(const chestnut::project::MediaPtr& child, chestnut::project::MediaPtr to);
    void removeChild(chestnut::project::MediaPtr parent, const chestnut::project::MediaPtr& child);
    chestnut::project::MediaPtr child(const int index, chestnut::project::MediaPtr parent = nullptr);
    int childCount(chestnut::project::MediaPtr parent = nullptr);
    void set_icon(const chestnut::project::MediaPtr& m, const QIcon &ico);
    void setIcon(chestnut::project::FootagePtr ftg, QIcon icon);
    QModelIndex add(const chestnut::project::MediaPtr& mda);
    chestnut::project::MediaPtr get(const QModelIndex& idx);
    const chestnut::project::MediaPtr get(const QModelIndex& idx) const;
    chestnut::project::MediaPtr getFolder(const int id);

    /**
     * Get all items held in model
     * @return map of Media items
     */
    const QMap<int, chestnut::project::MediaPtr>& items() const;

    /**
     *  Search the model for a Media instance by Id
     * @param id Media's id
     * @return media ptr or null
     */
    chestnut::project::MediaPtr findItemById(const int id);

    virtual bool load(QXmlStreamReader& stream) override;
    virtual bool save(QXmlStreamWriter& stream) const override;
private:
    friend class ProjectModelTest;
    QMap<int, chestnut::project::MediaPtr> project_items;

    /**
     * @brief         Add object into managed map of objects
     * @param item    Object to manage
     * @return        true==item exists in map
     */
    bool insert(const chestnut::project::MediaPtr& item);

    /**
     * @brief         Remove an object from the managed map of objects
     * @param item    Object to remove
     * @return        true==item was removed
     */
    bool remove(const chestnut::project::MediaPtr& item);

    QModelIndex create_index(const int row, const int col, const chestnut::project::MediaPtr& mda) const;
    QModelIndex create_index(const int row, const chestnut::project::MediaPtr& mda) const;

    bool saveFolders(QXmlStreamWriter& stream) const;
    bool saveMedia(QXmlStreamWriter& stream) const;
    bool saveSequences(QXmlStreamWriter& stream) const;
    bool saveTypes(QXmlStreamWriter& stream, const MediaType mda_type) const;

};

#endif // PROJECTMODEL_H
