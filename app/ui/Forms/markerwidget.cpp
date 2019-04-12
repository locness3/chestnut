#include "markerwidget.h"
#include "ui_markerwidget.h"

MarkerWidget::MarkerWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MarkerWidget)
{
  ui->setupUi(this);
}

MarkerWidget::~MarkerWidget()
{
  delete ui;
}
