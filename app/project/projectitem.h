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

#ifndef PROJECTITEM_H
#define PROJECTITEM_H

#include <QString>
#include <memory>
#include <QVector>

#include "project/marker.h"

class Sequence;
class Footage;

namespace project {

class ProjectItem : public IXMLStreamer {
public:
  ProjectItem() = default;
  explicit ProjectItem(const QString& itemName);
  virtual ~ProjectItem() = default;
  ProjectItem(const ProjectItem&) = default;
  ProjectItem& operator=(const ProjectItem&) = default;

  void setName(const QString& val);
  const QString& name() const;

  virtual bool load(QXmlStreamReader& stream) override;
  virtual bool save(QXmlStreamWriter& stream) const override;

  QVector<MarkerPtr> markers_{};
private:
  friend class ::Sequence;
  friend class ::Footage;
  QString name_{};
};

typedef std::shared_ptr<ProjectItem> ProjectItemPtr;
}
#endif // PROJECTITEM_H
