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

#include "markersviewer.h"
#include "ui_markersviewer.h"
#include "markerwidget.h"

#include "project/marker.h"

MarkersViewer::MarkersViewer(QWidget *parent) :
  QDockWidget(parent),
  ui(new Ui::MarkersViewer)
{
  ui->setupUi(this);

  // basic search functionality
  connect(ui->searchLine, &QLineEdit::textChanged, this, &MarkersViewer::refresh);
}

MarkersViewer::~MarkersViewer()
{
  delete ui;
}


bool MarkersViewer::setMedia(const chestnut::project::MediaPtr& mda)
{
  if (mda == nullptr) {
    return false;
  }
  if ( (mda->type() == MediaType::NONE) || (mda->type() == MediaType::FOLDER) ) {
    // markers not possible with these types
    return false;
  }
  if (media_.lock() == mda) {
    // already set
    return false;
  }
  media_ = mda;
  return true;
}

// std::sort on vector of shared_ptr doesn't compare instances
bool markerLessThan(const chestnut::project::MarkerPtr& lhs, const chestnut::project::MarkerPtr& rhs)
{
  Q_ASSERT(lhs != nullptr && rhs != nullptr);
  return *lhs < *rhs;
}

void MarkersViewer::refresh(const QString& filter)
{
  auto mda = media_.lock();
  if (mda == nullptr) {
    return;
  }
  // clean up current view
  QLayoutItem* item;
  while ( (item = ui->markerLayout->takeAt(0)) != nullptr) {
    delete item->widget();
    delete item;
  }

  auto pj = mda->object<chestnut::project::ProjectItem>();
  if (pj == nullptr) {
    // nothing to do
    return;
  }

  // populate
  MarkerWidget* widg = nullptr;
  auto markers = pj->markers_;
  std::sort(markers.begin(), markers.end(), markerLessThan);
  for (auto mark : markers) {
    if ( (mark != nullptr) && (mark->name.contains(filter) || mark->comment_.contains(filter))) {
      widg = new MarkerWidget(mark, this);
      ui->markerLayout->addWidget(widg);
    }
  }
  auto spacer = new QSpacerItem(0,0, QSizePolicy::Fixed, QSizePolicy::Expanding);
  ui->markerLayout->addSpacerItem(spacer);

}

void MarkersViewer::reset()
{
  media_.reset();
}
