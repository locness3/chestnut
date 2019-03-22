#ifndef VIEWERWINDOW_H
#define VIEWERWINDOW_H

#include <QOpenGLWindow>

class QMutex;

class ViewerWindow : public QOpenGLWindow {
    Q_OBJECT
  public:
    explicit ViewerWindow(QOpenGLContext* share);

    ViewerWindow(const ViewerWindow& ) = delete;
    ViewerWindow& operator=(const ViewerWindow&) = delete;

    void set_texture(GLuint t, double iar, QMutex *imutex);
  private:
    void paintGL() override;
    GLuint texture;
    double ar;
    QMutex* mutex;
};

#endif // VIEWERWINDOW_H
