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
#include "projectmodel.h"

#include "panels/panels.h"
#include "panels/viewer.h"
#include "ui/viewerwidget.h"
#include "project/media.h"
#include "debug.h"

ProjectModel::ProjectModel(QObject *parent)
    : QAbstractItemModel(parent),
      root_item(nullptr),
      project_items()
{
    make_root();
}

ProjectModel::~ProjectModel() {
    destroy_root();
}

void ProjectModel::make_root() {
    root_item = std::make_shared<Media>(nullptr);
    root_item->temp_id = 0;
    root_item->root = true;
}

void ProjectModel::destroy_root() {
    if (e_panel_sequence_viewer != nullptr) e_panel_sequence_viewer->viewer_widget->delete_function();
    if (e_panel_footage_viewer != nullptr) e_panel_footage_viewer->viewer_widget->delete_function();
}

void ProjectModel::clear() {
    beginResetModel();
    destroy_root();
    make_root();
    endResetModel();
}

MediaPtr ProjectModel::get_root() {
    return root_item;
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    MediaPtr item = getItem(index);
    if (item != nullptr) {
        return item->data(index.column(), role);
    }
    return QVariant();
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

QVariant ProjectModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return root_item->data(section, role);

    return QVariant();
}

QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    MediaPtr parentItem;

    if (!parent.isValid()) {
        parentItem = root_item;
    } else {
        parentItem = getItem(parent);
    }

    MediaPtr childItem = parentItem->child(row);
    if (childItem) {
        return create_index(row, column, childItem);
    } else {
        return QModelIndex();
    }
}


QModelIndex ProjectModel::create_index(const int row, const int col, const MediaPtr& mda) const {
    return createIndex(row, col, static_cast<quintptr>(mda->getId()));
}

QModelIndex ProjectModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    MediaPtr childItem = getItem(index);

    if (childItem != nullptr) {
        MediaWPtr parentItem = childItem->parentItem();
        if (!parentItem.expired()) {
            MediaPtr parPtr = parentItem.lock();
            if (parPtr == root_item) {
                return QModelIndex();
            }
            return create_index(parPtr->row(), 0 , parPtr);
        }
    }

    childItem = getItem(index);
    if (childItem != nullptr) {
        MediaWPtr parentItem = childItem->parentItem();
        if (!parentItem.expired()) {
            MediaPtr parPtr = parentItem.lock();
            if (parPtr == root_item) {
                return QModelIndex();
            }
            return create_index(parPtr->row(), 0, parPtr);
        }
    }
    return QModelIndex();
}

bool ProjectModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role != Qt::EditRole) {
        return false;
    }

    MediaPtr item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result) {
        emit dataChanged(index, index);
    }

    return result;
}

int ProjectModel::rowCount(const QModelIndex &parent) const {
    MediaPtr parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        parentItem = root_item;
    } else {
        parentItem = getItem(parent);
    }

    if (parentItem != nullptr) {
        return parentItem->childCount();
    }
    return 0;
}

int ProjectModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        MediaPtr mda = project_items.value(static_cast<int>(parent.internalId()));
        if (mda != nullptr) {
            return mda->columnCount();
        }
    } else {
        return root_item->columnCount();
    }
    return -1;
}

MediaPtr ProjectModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        const int id = static_cast<int>(index.internalId());
        return project_items.value(id);
    }
    return root_item;
}

void ProjectModel::set_icon(MediaPtr m, const QIcon &ico) {
    // This seems to be the only point at which data is added to the model
    project_items.insert(m->getId(), m);
    QModelIndex index = create_index(m->row(), 0 , m);
    m->set_icon(ico);
    emit dataChanged(index, index);

}

QModelIndex ProjectModel::add(MediaPtr mda) {
    project_items.insert(mda->getId(), mda);
    return create_index(mda->row(), 0 , mda);
}


MediaPtr ProjectModel::get(const QModelIndex& idx) {
    return project_items.value(idx.internalId());
}

void ProjectModel::appendChild(MediaPtr parent, MediaPtr child) {
    if (parent == nullptr) {
        parent = root_item;
    }
    beginInsertRows(parent == root_item ? QModelIndex() : create_index(parent->row(), 0, parent),
                    parent->childCount(),
                    parent->childCount());
    parent->appendChild(child);
    endInsertRows();
}

void ProjectModel::moveChild(MediaPtr child, MediaPtr to) {
    if (to == nullptr) {
        to = root_item;
    }
    if (auto parPtr = child->parentItem().lock()) {
        beginMoveRows(
                    parPtr == root_item ? QModelIndex() : create_index(parPtr->row(), 0 , parPtr),
                    child->row(),
                    child->row(),
                    to == root_item ? QModelIndex() : create_index(to->row(), 0, to),
                    to->childCount()
                    );
        parPtr->removeChild(child->row());
        to->appendChild(child);
        endMoveRows();
    }
}

void ProjectModel::removeChild(MediaPtr parent, MediaPtr m) {
    if (parent == nullptr) {
        parent = root_item;
    }
    beginRemoveRows(parent == root_item ? QModelIndex() : create_index(parent->row(), 0, parent),
                    m->row(),
                    m->row());
    parent->removeChild(m->row());
    endRemoveRows();
}

MediaPtr ProjectModel::child(int i, MediaPtr parent) {
    if (parent == nullptr) {
        parent = root_item;
    }
    return parent->child(i);
}

int ProjectModel::childCount(MediaPtr parent) {
    if (parent == nullptr) {
        parent = root_item;
    }
    return parent->childCount();
}
