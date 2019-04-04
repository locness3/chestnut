#include "scopeviewer.h"

#include <QVBoxLayout>

#include "debug.h"

using panels::ScopeViewer;


constexpr int PANEL_WIDTH = 640;
constexpr int PANEL_HEIGHT = 480;

ScopeViewer::ScopeViewer(QWidget* parent) : QDockWidget(parent)
{
  setup();
}

/**
 * @brief On the receipt of a grabbed frame from the renderer generate scopes
 * @param img Image of the final, rendered frame, minus the gizmos
 */
void ScopeViewer::frameGrabbed(const QImage& img)
{
  if (color_scope_ == nullptr) {
    qCritical() << "Scope instance is null";
    return;
  }
  color_scope_->updateImage(img);
}

/*
 * Populate this viewer with its widgets
 */
void ScopeViewer::setup()
{
  //TODO: add buttons to disable/enable channels
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  setWindowTitle(tr("Scope Viewer"));
  resize(PANEL_WIDTH, PANEL_HEIGHT);
  auto main_widget = new QWidget();
  setWidget(main_widget);

  auto layout = new QVBoxLayout();
  main_widget->setLayout(layout);

  color_scope_ = new ui::ColorScopeWidget();
  layout->addWidget(color_scope_);
  color_scope_->setMinimumSize(PANEL_WIDTH, 256);

  auto spacer = new QSpacerItem(20,20, QSizePolicy::Minimum, QSizePolicy::Expanding);
  layout->addSpacerItem(spacer);
}
