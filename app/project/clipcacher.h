#ifndef CLIPCACHER_H
#define CLIPCACHER_H

#include <QObject>
#include <QVector>
#include <QWaitCondition>
#include <QMutex>
#include <QColor>
#include <memory>

class Clip;
class Media;

struct TimeLineInfo {
    bool enabled = true;
    long clip_in = 0;
    long in = 0;
    long out = 0;
    int track = 0;
    QString name = "";
    QColor color = {0,0,0};
    std::shared_ptr<Media> media = nullptr;
    int media_stream = -1;
    double speed = 1.0;
    double cached_fr = 0.0;
    bool reverse = false;
    bool maintain_audio_pitch = true;
    bool autoscale = true;
};

class ClipCacher : public QObject
{
    Q_OBJECT
public:
    ClipCacher(std::shared_ptr<Clip> clipCached, QObject *parent = nullptr);
    virtual ~ClipCacher();

signals:
    void workDone();

public slots:
    void doWork(const TimeLineInfo& info);
    void update(const long playhead, const bool resetCache, const bool scrubbing, QVector<std::shared_ptr<Clip>>& nests);

private:
    std::shared_ptr<Clip> m_cachedClip;
    bool m_caching = false;
    // must be set before caching
    long m_playhead = 0;
    bool m_reset = false;
    bool m_scrubbing = false;
    bool m_interrupt = false;

    QMutex m_lock;
    QWaitCondition m_canCache;


    QVector<std::shared_ptr<Clip>> m_nests = QVector<std::shared_ptr<Clip>>();

    bool initialise(const TimeLineInfo &info);
    void uninitialise();

    void worker(const long playhead, const bool resetCache, const bool m_scrubbing, QVector<std::shared_ptr<Clip>>& nests);
    void reset(const long playhead);
};

#endif // CLIPCACHER_H
