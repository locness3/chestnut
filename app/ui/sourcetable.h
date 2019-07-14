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

class Project;
class Media;

class SourceTable : public QTreeView
{
    Q_OBJECT
public:
    explicit SourceTable(QWidget* parent = nullptr);

    SourceTable(const SourceTable& ) = delete;
    SourceTable& operator=(const SourceTable&) = delete;
    
    Project* project_parent_;
protected:
    void mousePressEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
private slots:
    void item_click(const QModelIndex& index);
    void show_context_menu();
};

#endif // SOURCETABLE_H
