#ifndef EFFECTS_H
#define EFFECTS_H

#include <QWidget>

namespace Ui {
  class Effects;
}

class Effects : public QWidget
{
    Q_OBJECT

  public:
    explicit Effects(QWidget *parent = nullptr);
    ~Effects();

  private:
    Ui::Effects *ui;
};

#endif // EFFECTS_H
