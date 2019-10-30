#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include <QOpenGLWindow>

class QMutex;

class ViewerWindow : public QOpenGLWindow {
    Q_OBJECT
  public:
    explicit ViewerWindow(QOpenGLContext* share);
    ~ViewerWindow() override;

    ViewerWindow(const ViewerWindow& ) = delete;
    ViewerWindow& operator=(const ViewerWindow&) = delete;
    ViewerWindow(const ViewerWindow&& ) = delete;
    ViewerWindow& operator=(const ViewerWindow&&) = delete;

    void setTexture(const GLuint t, const double iar, QMutex *imutex);
  private:
    void paintGL() override;
    GLuint texture {0};
    double ar {0.0};
    QMutex* mutex {nullptr};
};

#endif // VIEWERWINDOW_H
