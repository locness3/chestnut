#include "markerwidget.h"
#include "ui_markerwidget.h"

MarkerWidget::MarkerWidget(Marker mark, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MarkerWidget),
  marker_(std::move(mark))
{
  ui->setupUi(this);
  ui->nameEdit->setText(marker_.name);
  ui->inEdit->setText(QString::number(marker_.frame));
  ui->outEdit->setText(QString::number(marker_.frame + marker_.duration_));
  ui->commentsEdit->setPlainText(marker_.comment_);
}

MarkerWidget::~MarkerWidget()
{
  delete ui;
}
