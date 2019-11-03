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
#include "sourcetable.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QProcess>

#include "panels/project.h"
#include "panels/panelmanager.h"
#include "project/footage.h"
#include "playback/playback.h"
#include "project/undo.h"
#include "project/sequence.h"
#include "ui/mainwindow.h"
#include "ui/viewerwidget.h"
#include "io/config.h"
#include "project/media.h"
#include "project/sourcescommon.h"
#include "debug.h"


SourceTable::SourceTable(QAbstractItemModel* model, Project& proj_parent, QWidget* parent)
  : QTreeView(parent),
    chestnut::ui::SourceView(proj_parent)
{
  setModel(model);
  setSortingEnabled(true);
  setAcceptDrops(true);
  sortByColumn(0, Qt::AscendingOrder);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setDragDropMode(QAbstractItemView::DragDrop);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(itemClick(const QModelIndex&)));
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu()));
  header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}


void SourceTable::showContextMenu()
{
  Q_ASSERT(project_parent_.sources_common);
  project_parent_.sources_common->show_context_menu(this, selectionModel()->selectedRows());
}



void SourceTable::itemClick(const QModelIndex& index)
{
  Q_ASSERT(project_parent_.sources_common);
  if ((selectionModel()->selectedRows().size() == 1) && (index.column() == 0)) {
    project_parent_.sources_common->item_click(project_parent_.item_to_media(index), index);
  }
}

void SourceTable::mousePressEvent(QMouseEvent* event)
{
  Q_ASSERT(project_parent_.sources_common);
  project_parent_.sources_common->mousePressEvent(event);
  QTreeView::mousePressEvent(event);
}

void SourceTable::mouseDoubleClickEvent(QMouseEvent* event)
{
  Q_ASSERT(project_parent_.sources_common);
  project_parent_.sources_common->mouseDoubleClickEvent(event, selectionModel()->selectedRows());
}

void SourceTable::dragEnterEvent(QDragEnterEvent *event)
{
  Q_ASSERT(event);
  if (!onDragEnterEvent(*event)) {
    QTreeView::dragEnterEvent(event); //TODO: identify if this is truly needed
  }
}

void SourceTable::dragMoveEvent(QDragMoveEvent *event)
{
  Q_ASSERT(event);
  if (!onDragMoveEvent(*event)) {
    QTreeView::dragMoveEvent(event);//TODO: identify if this is truly needed
  }
}

void SourceTable::dropEvent(QDropEvent* event)
{
  Q_ASSERT(event);
  onDropEvent(*event);
}


QModelIndex SourceTable::viewIndex(const QPoint& pos) const
{
  auto index = indexAt(pos);
  if (!index.isValid()) {
    index = rootIndex();
  }
  return index;
}


QModelIndexList SourceTable::selectedItems() const
{
  return selectionModel()->selectedRows();
}
