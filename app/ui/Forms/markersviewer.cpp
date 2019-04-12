#include "markersviewer.h"
#include "ui_markersviewer.h"

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
