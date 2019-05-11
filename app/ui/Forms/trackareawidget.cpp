#include "trackareawidget.h"
#include "ui_trackareawidget.h"

TrackAreaWidget::TrackAreaWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::TrackAreaWidget)
{
  ui->setupUi(this);
}

TrackAreaWidget::~TrackAreaWidget()
{
  delete ui;
}


void TrackAreaWidget::initialise()
{
  Q_ASSERT(ui != nullptr);
  Q_ASSERT(ui->lockButton != nullptr);
  Q_ASSERT(ui->enableButton != nullptr);

  QObject::connect(ui->lockButton, &QPushButton::toggled, this, &TrackAreaWidget::lockTrack);
  QObject::connect(ui->enableButton, &QPushButton::toggled, this, &TrackAreaWidget::enableTrack);
}


void TrackAreaWidget::setName(const QString& name)
{
  Q_ASSERT(ui != nullptr);
  Q_ASSERT(ui->trackNameLabel != nullptr);

  ui->trackNameLabel->setText(name);
}
