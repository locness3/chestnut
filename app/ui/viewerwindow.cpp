#include "viewerwindow.h"

#include <QMutex>
#include <QMutexLocker>

ViewerWindow::ViewerWindow(QOpenGLContext *share) :
  QOpenGLWindow(share)
{

}

ViewerWindow::~ViewerWindow()
{
  mutex = nullptr;
}

void ViewerWindow::setTexture(const GLuint t, const double iar, QMutex* imutex)
{
  texture = t;
  ar = iar;
  mutex = imutex;
  update();
}

void ViewerWindow::paintGL()
{
  if (texture <= 0) {
    return;
  }

  QMutexLocker locker(mutex);

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, texture);

  glLoadIdentity();
  glOrtho(0, 1, 0, 1, -1, 1);

  glBegin(GL_QUADS);

  double top = 0;
  double left = 0;
  double right = 1;
  double bottom = 1;

  const double widget_ar = static_cast<double>(width()) / height();

  if (widget_ar > ar) {
    const double width = 1.0 * ar / widget_ar;
    left = (1.0 - width) * 0.5;
    right = left + width;
  } else {
    const double height = 1.0 / ar * widget_ar;
    top = (1.0 - height) * 0.5;
    bottom = top + height;
  }

  glVertex2d(left, top);
  glTexCoord2d(0, 0);
  glVertex2d(left, bottom);
  glTexCoord2d(1, 0);
  glVertex2d(right, bottom);
  glTexCoord2d(1, 1);
  glVertex2d(right, top);
  glTexCoord2d(0, 1);

  glEnd();

  glBindTexture(GL_TEXTURE_2D, 0);

  glDisable(GL_TEXTURE_2D);
}
