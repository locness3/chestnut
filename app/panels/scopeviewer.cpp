/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scopeviewer.h"

#include <QVBoxLayout>

#include "debug.h"

using panels::ScopeViewer;


constexpr int PANEL_WIDTH = 320;
constexpr int PANEL_HEIGHT = 240;

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
  // Frame may have been retrieved after an effect change
  color_scope_->update();
}

/*
 * Populate this viewer with its widgets
 */
void ScopeViewer::setup()
{
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  setWindowTitle(tr("Scope Viewer"));
  resize(PANEL_WIDTH, PANEL_HEIGHT);
  auto main_widget = new QWidget();
  setWidget(main_widget);

  auto layout = new QVBoxLayout();
  main_widget->setLayout(layout);

  auto h_layout = new QHBoxLayout();
  waveform_combo_ = new QComboBox();
  QStringList items = {"RGB", "Luma"};
  waveform_combo_->addItems(items);
  connect(waveform_combo_, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChanged(int)));
  h_layout->addWidget(waveform_combo_);

  auto spacer = new QSpacerItem(0,0, QSizePolicy::Expanding);
  h_layout->addSpacerItem(spacer);
  layout->addLayout(h_layout);

  color_scope_ = new ui::ColorScopeWidget();
  layout->addWidget(color_scope_);
  color_scope_->setMinimumSize(PANEL_WIDTH, PANEL_HEIGHT);
}


void ScopeViewer::indexChanged(int index)
{
  color_scope_->mode_ = index;
  color_scope_->update();
}
