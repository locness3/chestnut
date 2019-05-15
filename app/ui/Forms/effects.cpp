#include "effects.h"
#include "ui_effects.h"

Effects::Effects(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Effects)
{
  ui->setupUi(this);
}

Effects::~Effects()
{
  delete ui;
}
