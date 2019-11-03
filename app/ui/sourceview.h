/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOURCEVIEW_H
#define SOURCEVIEW_H

#include <QObject>
#include <QDropEvent>

#include "panels/project.h"

namespace chestnut::ui
{
  class SourceView
  {
    public:
      SourceView() = delete;
      explicit SourceView(Project& project_parent);
      virtual ~SourceView() = default;

    protected:
      /**
       * @brief Identify if the event is to be accepted
       * @param event
       * @return  true==event is accepted
       */
      bool onDragEnterEvent(QDragEnterEvent& event) const;
      /**
       * @brief Identify if the event is to be accepted
       * @param event
       * @return  true==event is accepted
       */
      bool onDragMoveEvent(QDragMoveEvent& event) const;
      /**
       * @brief Identify if the event is to be accepted
       * @param event
       * @return  true==event is accepted
       */
      bool onDropEvent(QDropEvent& event) const;

      /**
       * @brief     Retrieve the item at cursor position within the widget
       * @param pos Cursor position
       * @return    Valid item or root
       */
      virtual QModelIndex viewIndex(const QPoint& pos) const = 0;
      /**
       * @brief   Retrieve all the items selected within the widget
       * @return  Items
       */
      virtual QModelIndexList selectedItems() const = 0;

    private:
      friend class ::SourceTable;
      friend class ::SourceIconView;
      Project& project_parent_;

      /**
       * @brief Create a subclip of the clip in the footage viewer and add to the Project
       * @param event
       */
      void addSubclipToProject(const QDropEvent& event) const;
      /**
       * @brief Add media to the Project from an OS drag and drop
       * @param event
       */
      void addMediaToProject(const QDropEvent& event) const;
      /**
       * @brief Move a Media item within the Project via the widget
       * @param event
       */
      void moveMedia(const QDropEvent& event) const;
  };
}

#endif // SOURCEVIEW_H
