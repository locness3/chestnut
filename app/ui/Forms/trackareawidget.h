#ifndef TRACKAREAWIDGET_H
#define TRACKAREAWIDGET_H

#include <QWidget>

namespace Ui {
  class TrackAreaWidget;
}

class TrackAreaWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit TrackAreaWidget(QWidget *parent = nullptr);
    ~TrackAreaWidget();

    void initialise();
    void setName(const QString& name);
  signals:
    void enableTrack(bool value);
    void lockTrack(bool value);
  private:
    Ui::TrackAreaWidget *ui;
};


#endif // TRACKAREAWIDGET_H
