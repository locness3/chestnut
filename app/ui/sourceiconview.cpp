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
#include "sourceiconview.h"

#include <QMimeData>
#include <QMouseEvent>

#include "panels/project.h"
#include "project/media.h"
#include "project/sourcescommon.h"
#include "debug.h"

SourceIconView::SourceIconView(Project& projParent, QWidget *parent)
  : QListView(parent),
    chestnut::ui::SourceView(projParent)
{
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setResizeMode(QListView::Adjust);
  setContextMenuPolicy(Qt::CustomContextMenu);
  setAcceptDrops(true);
  connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(item_click(const QModelIndex&)));
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(show_context_menu()));
}


void SourceIconView::show_context_menu()
{
  Q_ASSERT(project_parent_.sources_common_);
  project_parent_.sources_common_->show_context_menu(this, selectedIndexes());
}

void SourceIconView::item_click(const QModelIndex& index)
{
  Q_ASSERT(project_parent_.sources_common_);
  if ( (selectedIndexes().size() == 1) && (index.column() == 0) ) {
    project_parent_.sources_common_->item_click(project_parent_.item_to_media(index), index);
  }
}

void SourceIconView::mousePressEvent(QMouseEvent* event)
{
  Q_ASSERT(project_parent_.sources_common_);
  project_parent_.sources_common_->mousePressEvent(event);
  if (!indexAt(event->pos()).isValid()) {
    selectionModel()->clear();
  }
  QListView::mousePressEvent(event);
}

void SourceIconView::mouseDoubleClickEvent(QMouseEvent *event)
{
  bool default_behavior = true;
  if (selectedIndexes().size() == 1) {
    if (auto m = project_parent_.item_to_media(selectedIndexes().front())) {
      if (m->type() == MediaType::FOLDER) {
        default_behavior = false;
        setRootIndex(selectedIndexes().at(0));
        emit changed_root();
      }
    } else {
      qCritical() << "Selection failed";
    }
  }
  if (default_behavior) {
    Q_ASSERT(project_parent_.sources_common_);
    project_parent_.sources_common_->mouseDoubleClickEvent(event, selectedIndexes());
  }
}

void SourceIconView::dragEnterEvent(QDragEnterEvent *event)
{
  Q_ASSERT(event);
  if (!onDragEnterEvent(*event)) {
    QListView::dragEnterEvent(event); //TODO: identify if this is truly needed
  }
}

void SourceIconView::dragMoveEvent(QDragMoveEvent *event)
{
  Q_ASSERT(event);
  if (!onDragMoveEvent(*event)) {
    QListView::dragMoveEvent(event);//TODO: identify if this is truly needed
  }
}

void SourceIconView::dropEvent(QDropEvent* event)
{
  Q_ASSERT(event);
  onDropEvent(*event);
}

QModelIndex SourceIconView::viewIndex(const QPoint& pos) const
{
  auto index = indexAt(pos);
  if (!index.isValid()) {
    index = rootIndex();
  }
  return index;
}

QModelIndexList SourceIconView::selectedItems() const
{
  return selectedIndexes();
}
