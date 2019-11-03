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
#ifndef SOURCETABLE_H
#define SOURCETABLE_H

#include <QTreeView>
#include <QTimer>
#include <QUndoCommand>

#include "ui/sourceview.h"

class Project;
class Media;

class SourceTable : public QTreeView, public chestnut::ui::SourceView {
    Q_OBJECT
  public:
    SourceTable(QAbstractItemModel *model, Project& proj_parent, QWidget* parent);

  protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    QModelIndex viewIndex(const QPoint& pos) const override;
    QModelIndexList selectedItems() const override;
  private slots:
    void itemClick(const QModelIndex& index);
    void showContextMenu();
  private:
};

#endif // SOURCETABLE_H
