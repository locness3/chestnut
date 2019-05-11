#ifndef TIMELINETRACKAREA_H
#define TIMELINETRACKAREA_H

#include <QWidget>
#include <QMap>
#include <QVBoxLayout>

#include "ui/Forms/trackareawidget.h"

namespace Ui {
  class TimelineTrackArea;
}

enum class TimelineTrackType {
  AUDIO,
  VISUAL,
  NONE
};

class TimelineTrackArea : public QWidget
{
    Q_OBJECT

  public:
    explicit TimelineTrackArea(QWidget *parent = nullptr);
    ~TimelineTrackArea();

    bool initialise(const TimelineTrackType area_type);
    bool addTrack(const int number, QString name);

    void setHeights(const QVector<int>& heights);
    TimelineTrackType type_{TimelineTrackType::NONE};

  public slots:

  private slots:
    void trackEnableChange(const bool enabled);

    void trackLockChange(const bool locked);

  signals:
    void trackEnabled(const bool enabled, const int track_number);
    void trackLocked(const bool locked, const int track_number);

  private:
    Ui::TimelineTrackArea *ui;


    QMap<int, TrackAreaWidget*> track_widgets_;

    void update();
};

#endif // TIMELINETRACKAREA_H
