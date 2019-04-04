#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>

#include "project/sequence.h"
#include "project/effect.h"

class RenderThread : public QThread {
    Q_OBJECT
  public:
    RenderThread();
    virtual ~RenderThread() override;

    RenderThread(const RenderThread& ) = delete;
    RenderThread(const RenderThread&& ) = delete;
    RenderThread& operator=(const RenderThread&) = delete;
    RenderThread& operator=(const RenderThread&&) = delete;

    QMutex mutex;
    GLuint frameBuffer;
    GLuint texColorBuffer;
    EffectPtr gizmos;
    void paint();
    void start_render(QOpenGLContext* share, SequenceWPtr s, const bool grab=false);
    bool did_texture_fail();
    void cancel();
  protected:
    void run() override;
  public slots:
    // cleanup functions
    void delete_ctx();
  signals:
    void ready();
    void frameGrabbed(QImage img);
  private:
    // cleanup functions
    void delete_texture();
    void delete_fbo();

    QWaitCondition waitCond;
    QOffscreenSurface surface;
    QOpenGLContext* share_ctx;
    QOpenGLContext* ctx;
    SequenceWPtr seq;
    int divider{};
    int tex_width;
    int tex_height;
    bool queued;
    bool texture_failed;
    bool running;
    bool frame_grabbing_{};
};

#endif // RENDERTHREAD_H
