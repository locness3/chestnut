#include "ui/markerdockwidget.h"

#include <QInputDialog>

using ui::MarkerDockWidget;

std::tuple<QString, bool> MarkerDockWidget::getName() const
{
  QInputDialog d;
  d.setWindowTitle(QObject::tr("Set Marker"));
  d.setLabelText(QObject::tr("Set marker name:"));
  d.setInputMode(QInputDialog::TextInput);
  bool add_marker = (d.exec() == QDialog::Accepted);
  QString marker_name = d.textValue();
  return std::make_tuple(marker_name, add_marker);
}
