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


bool MarkersViewer::setMedia(const MediaPtr& mda)
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

  auto pj = mda->object<project::ProjectItem>();
  if (pj == nullptr) {
    // nothing to do
    return;
  }

  // populate
  MarkerWidget* widg = nullptr;
  for (auto mark : pj->markers_) {
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
