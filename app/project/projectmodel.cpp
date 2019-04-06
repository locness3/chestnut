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

#include "panels/panelmanager.h"
#include "ui/viewerwidget.h"
#include "project/media.h"
#include "debug.h"

constexpr int DEFAULT_COLUMN = 0;

using panels::PanelManager;

ProjectModel::ProjectModel(QObject *parent)
  : QAbstractItemModel(parent)
{
  insert(std::make_shared<Media>());
}

ProjectModel::~ProjectModel()
{
}

void ProjectModel::destroy_root()
{
  PanelManager::sequenceViewer().viewer_widget->delete_function();
  PanelManager::footageViewer().viewer_widget->delete_function();
}

void ProjectModel::clear()
{
  beginResetModel();
  destroy_root();
  endResetModel();
  project_items.clear();
  insert(std::make_shared<Media>());
}

MediaPtr ProjectModel::root()
{
  return project_items.first();
}

QVariant ProjectModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  if (auto item = getItem(index)) {
    return item->data(index.column(), role);
  }
  return QVariant();
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsDropEnabled;

  return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

QVariant ProjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && project_items.first()) {
    return project_items.first()->data(section, role);
  }

  return QVariant();
}

QModelIndex ProjectModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  const auto parentItem = parent.isValid() ? getItem(parent) : project_items.first();

  auto childItem = parentItem->child(row);
  QModelIndex idx;
  if (childItem) {
    idx = create_index(row, column, childItem);
  }
  return idx;
}


QModelIndex ProjectModel::create_index(const int row, const int col, const MediaPtr& mda) const
{
  return createIndex(row, col, static_cast<quintptr>(mda->id()));
}


QModelIndex ProjectModel::create_index(const int row, const MediaPtr& mda) const
{
  return create_index(row, DEFAULT_COLUMN, mda);
}

QModelIndex ProjectModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  auto childItem = getItem(index);

  if (childItem != nullptr) {
    if (const auto parentItem = childItem->parentItem()) {
      if (parentItem == project_items.first()) {
        return QModelIndex();
      }
      return create_index(parentItem->row(), parentItem);
    }
  }

  childItem = getItem(index);
  if (childItem != nullptr) {
    if (const auto parentItem = childItem->parentItem()) {
      if (parentItem == project_items.first()) {
        return QModelIndex();
      }
      return create_index(parentItem->row(), parentItem);
    }
  }
  return QModelIndex();
}

bool ProjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (role != Qt::EditRole) {
    return false;
  }

  const auto item = getItem(index);
  const auto result = item->setData(index.column(), value);

  if (result) {
    emit dataChanged(index, index);
  }

  return result;
}

int ProjectModel::rowCount(const QModelIndex &parent) const
{
  MediaPtr parentItem;
  if (parent.column() > 0)
    return 0;

  if (!parent.isValid()) {
    parentItem = project_items.first();
  } else {
    parentItem = getItem(parent);
  }

  if (parentItem != nullptr) {
    return parentItem->childCount();
  }
  return 0;
}

int ProjectModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid()) {
    const auto mda = get(parent);
    if (mda != nullptr) {
      return mda->columnCount();
    }
  } else if (project_items.first()) {
    return project_items.first()->columnCount();
  }
  return -1;
}

MediaPtr ProjectModel::getItem(const QModelIndex &index) const
{
  if (index.isValid()) {
    return get(index);
  }
  return project_items.first();
}

void ProjectModel::set_icon(const MediaPtr& m, const QIcon &ico)
{
  insert(m);
  const auto index = create_index(m->row(), m);
  m->setIcon(ico);
  emit dataChanged(index, index);

}

QModelIndex ProjectModel::add(const MediaPtr& mda)
{
  insert(mda);
  return create_index(mda->row(), mda);
}


MediaPtr ProjectModel::get(const QModelIndex& idx)
{
  return project_items.value(idx.internalId());
}


const MediaPtr ProjectModel::get(const QModelIndex& idx) const
{
  return project_items.value(idx.internalId());
}

bool ProjectModel::appendChild(MediaPtr parent, const MediaPtr& child)
{
  if (project_items.empty()) {
    return false;
  }
  if (parent == nullptr) {
    parent = project_items.first();
  }
  auto item = parent == project_items.first() ? QModelIndex() : create_index(parent->row(), parent);
  beginInsertRows(item,
                  parent->childCount(),
                  parent->childCount());
  parent->appendChild(child);

  insert(child);
  endInsertRows();
  return true;
}

void ProjectModel::moveChild(const MediaPtr& child, MediaPtr to)
{
  if (to == nullptr) {
    to = project_items.first();
  }
  if (const auto parPtr = child->parentItem()) {
    beginMoveRows(
          parPtr == project_items.first() ? QModelIndex() : create_index(parPtr->row(), parPtr),
          child->row(),
          child->row(),
          to == project_items.first() ? QModelIndex() : create_index(to->row(), to),
          to->childCount()
          );
    parPtr->removeChild(child->row());
    to->appendChild(child);
    endMoveRows();
  }
}

void ProjectModel::removeChild(MediaPtr parent, const MediaPtr& m)
{
  if (parent == nullptr) {
    parent = project_items.first();
  }
  beginRemoveRows(parent == project_items.first() ? QModelIndex() : create_index(parent->row(), parent),
                  m->row(),
                  m->row());
  parent->removeChild(m->row());
  endRemoveRows();
}

MediaPtr ProjectModel::child(const int index, MediaPtr parent)
{
  if (parent == nullptr) {
    parent = project_items.first();
  }
  return parent->child(index);
}

int ProjectModel::childCount(MediaPtr parent)
{
  if (parent == nullptr) {
    parent = project_items.first();
  }
  return parent->childCount();
}

/**
 * @brief insert Add object into managed map of objects
 * @param item    Object to manage
 * @return  true==item exists in map
 */
bool ProjectModel::insert(const MediaPtr& item)
{
  auto exists = false;

  if (item != nullptr) {
    project_items.insert(item->id(), item);
    exists = true;
  }

  return exists;
}
