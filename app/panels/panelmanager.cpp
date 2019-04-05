#include "panelmanager.h"

#include "debug.h"

using panels::PanelManager;
using panels::HistogramViewer;

QWidget* PanelManager::parent_ = nullptr;
HistogramViewer* PanelManager::histogram_viewer_ = nullptr;
panels::ScopeViewer* PanelManager::scope_viewer_ = nullptr;
GraphEditor* PanelManager::graph_editor_ = nullptr;
Timeline* PanelManager::timeline_ = nullptr;
EffectControls* PanelManager::fx_controls_ = nullptr;


bool PanelManager::setParent(QWidget* parent)
{
  if (parent_ == nullptr) {
    parent_ = parent;
    return true;
  }
  qCritical() << "Parent object already set";
  return false;
}

HistogramViewer& PanelManager::histogram()
{
  if (histogram_viewer_ == nullptr) {
    histogram_viewer_ = new HistogramViewer(parent_);
    histogram_viewer_->setObjectName("histogram viewer");
  }
  return *histogram_viewer_;
}


panels::ScopeViewer& PanelManager::colorScope()
{
  if (scope_viewer_ == nullptr) {
    scope_viewer_ = new panels::ScopeViewer(parent_);
    scope_viewer_->setObjectName("scope viewer");
  }
  return *scope_viewer_;
}


GraphEditor& PanelManager::graphEditor()
{
  if (graph_editor_ == nullptr) {
    graph_editor_ = new GraphEditor(parent_);
    graph_editor_->setObjectName("graph editor");
  }
  return *graph_editor_;
}


Timeline& PanelManager::timeLine()
{
  if (timeline_ == nullptr) {
    timeline_ = new Timeline(parent_);
    timeline_->setObjectName("timeline");
  }
  return *timeline_;
}


EffectControls& PanelManager::fxControls()
{
  if (fx_controls_ == nullptr) {
    fx_controls_ = new EffectControls(parent_);
    fx_controls_->setObjectName("fx controls");
    init_effects(); // TODO: remove
  }
  return *fx_controls_;
}

void PanelManager::tearDown()
{
  parent_ = nullptr;
  delete histogram_viewer_;
  histogram_viewer_ = nullptr;
  delete scope_viewer_;
  scope_viewer_ = nullptr;
  delete graph_editor_;
  graph_editor_ = nullptr;
  delete timeline_;
  timeline_ = nullptr;
  delete fx_controls_;
  fx_controls_ = nullptr;
}
