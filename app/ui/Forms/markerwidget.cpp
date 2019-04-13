#include "markerwidget.h"
#include "ui_markerwidget.h"

MarkerWidget::MarkerWidget(MarkerPtr mark, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MarkerWidget),
  marker_(mark)
{
  Q_ASSERT(mark != nullptr);
  ui->setupUi(this);
  ui->nameEdit->setText(marker_->name);
  ui->inEdit->setText(QString::number(marker_->frame));
  ui->outEdit->setText(QString::number(marker_->frame + marker_->duration_));
  ui->commentsEdit->setPlainText(marker_->comment_);

  connect(ui->nameEdit, &QLineEdit::textChanged, this, &MarkerWidget::onValChange);
  connect(ui->inEdit, &QLineEdit::textChanged, this, &MarkerWidget::onValChange);
  connect(ui->outEdit, &QLineEdit::textChanged, this, &MarkerWidget::onValChange);
  connect(ui->commentsEdit, &QPlainTextEdit::textChanged, this, &MarkerWidget::onCommentChanged);
}

MarkerWidget::~MarkerWidget()
{
  delete ui;
}

void MarkerWidget::onValChange(const QString& val)
{
  //TODO: command
  if (sender() == ui->nameEdit) {
    marker_->name = val;
  } else if (sender() == ui->inEdit) {
    marker_->frame = val.toLong();
  } else if (sender() == ui->outEdit) {
    marker_->duration_ = val.toLong() - marker_->frame;
  } else if (sender() == ui->commentsEdit) {
    marker_->comment_ = val;
  }
}

void MarkerWidget::onCommentChanged()
{
  //TODO: command
  marker_->comment_ = ui->commentsEdit->document()->toPlainText();
}
