#include "markersviewer.h"
#include "ui_markersviewer.h"
#include "markerwidget.h"

#include "project/marker.h"

MarkersViewer::MarkersViewer(QWidget *parent) :
  QDockWidget(parent),
  ui(new Ui::MarkersViewer)
{
  ui->setupUi(this);
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


void MarkersViewer::refresh()
{
  auto mda = media_.lock();
  if (mda == nullptr) {
    return;
  }
  QLayoutItem* item;
  while ( (item = ui->markerLayout->takeAt(0)) != nullptr) {
    delete item->widget();
    delete item;
  }

  MarkerWidget* widg = nullptr;
  auto pj = mda->object<project::ProjectItem>();

  for (auto mark : pj->markers_) {
    widg = new MarkerWidget(mark, this);
    ui->markerLayout->addWidget(widg);
  }
  auto spacer = new QSpacerItem(0,0, QSizePolicy::Fixed, QSizePolicy::Expanding);
  ui->markerLayout->addSpacerItem(spacer);

}

void MarkersViewer::reset()
{
  media_.reset();
}
