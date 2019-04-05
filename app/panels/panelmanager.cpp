#include "panelmanager.h"

#include "debug.h"

using panels::PanelManager;
using panels::HistogramViewer;

QWidget* PanelManager::parent_ = nullptr;
HistogramViewer* PanelManager::histogram_viewer_ = nullptr;
panels::ScopeViewer* PanelManager::scope_viewer_ = nullptr;
GraphEditor* PanelManager::graph_editor_ = nullptr;
Timeline* PanelManager::timeline_ = nullptr;

PanelManager::PanelManager()
{

}


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
    graph_editor_->setObjectName("timeline");
  }
  return *timeline_;
}
