/*
 * Chestnut. Chestnut is a free non-linear video editor for Linux.
 * Copyright (C) 2019 Jonathan Noble
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

#include "viewerbase.h"

#include <QLayout>

#include "panels/panelmanager.h"

using chestnut::panels::ViewerBase;
using panels::PanelManager;


ViewerBase::ViewerBase(QWidget* parent) : QDockWidget(parent)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  setupWidgets();
}

ViewerBase::~ViewerBase()
{

}

void ViewerBase::setName(QString name)
{
  name_ = std::move(name);
  this->updatePanelTitle();
}

void ViewerBase::setMedia(project::MediaWPtr mda)
{
  media_ = mda;
  this->updatePanelTitle();
  Q_ASSERT(scroll_area_);
  scroll_area_->enableMenu(true);
  enableWidgets(true);
  viewed_item_ = mda.lock()->object();
  headers_->setViewedItem(viewed_item_);
}

bool ViewerBase::hasMedia() const noexcept
{
  return !media_.expired();
}


void ViewerBase::setZoom(const double value)
{
  Q_ASSERT(frame_view_);
  frame_view_->setZoom(value);
}

void ViewerBase::togglePlay()
{
  if (isPlaying()) {
    this->pause();
  } else {
    this->play();
  }
}

void ViewerBase::clear()
{
  Q_ASSERT(frame_view_);
  frame_view_->clear();
  media_.reset();
  this->updatePanelTitle();
  Q_ASSERT(scroll_area_);
  scroll_area_->enableMenu(false);
  current_timecode_->setValue(0);
  enableWidgets(false);
}


void ViewerBase::resizeEvent(QResizeEvent* event)
{
  Q_ASSERT(frame_view_);
  frame_view_->rescale();
  QDockWidget::resizeEvent(event);
}

void ViewerBase::setupWidgets()
{
  auto contents = new QWidget();

  auto layout = new QVBoxLayout(contents);
  layout->setSpacing(0);
  layout->setMargin(0);

  scroll_area_ = new ui::MenuScrollArea(contents);
  scroll_area_->setWidgetResizable(false);
  scroll_area_->setFrameShape(QFrame::NoFrame); // Remove 1px border
  layout->addWidget(scroll_area_);

  frame_view_ = new ui::ImageCanvas(scroll_area_);
  frame_view_->enableSmoothResize(true);
  frame_view_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  scroll_area_->setWidget(frame_view_);
  connect(frame_view_, &ui::ImageCanvas::isAutoScaling, this, &ViewerBase::scrollAreaHideScrollbars);
  connect(scroll_area_, &ui::MenuScrollArea::setZoom, frame_view_, &ui::ImageCanvas::setZoom);
  connect(scroll_area_, &ui::MenuScrollArea::clear, this, &ViewerBase::clear);

  headers_ = new ui::ViewerTimeline(contents);
  connect(headers_, &ui::ViewerTimeline::updateParents, this, &ViewerBase::updateParents);
  layout->addWidget(headers_);

  horizontal_bar_ = new ResizableScrollBar(contents);
  horizontal_bar_->setSingleStep(20);
  horizontal_bar_->setOrientation(Qt::Horizontal);
  layout->addWidget(horizontal_bar_);

  auto lower_controls = new QWidget(contents);

  auto lower_control_layout = new QHBoxLayout(lower_controls);
  lower_control_layout->setMargin(0);

  // current time code
  auto current_timecode_container = new QWidget(lower_controls);
  auto current_timecode_container_layout = new QHBoxLayout(current_timecode_container);
  current_timecode_container_layout->setSpacing(0);
  current_timecode_container_layout->setMargin(0);
  current_timecode_ = new LabelSlider(current_timecode_container);
  current_timecode_->setToolTip(tr("Playhead position"));
  current_timecode_->set_display_type(SliderType::FRAMENUMBER);
  current_timecode_container_layout->addWidget(current_timecode_);
  lower_control_layout->addWidget(current_timecode_container);

  auto playback_controls = new QWidget(lower_controls);
  auto playback_control_layout = new QHBoxLayout(playback_controls);
  playback_control_layout->setSpacing(0);
  playback_control_layout->setMargin(0);

  skip_to_start_btn_ = new QPushButton(playback_controls);
  skip_to_start_btn_->setToolTip(tr("Go to In"));
  QIcon goToStartIcon;
  goToStartIcon.addFile(QStringLiteral(":/icons/prev.png"), QSize(), QIcon::Normal, QIcon::Off);
  goToStartIcon.addFile(QStringLiteral(":/icons/prev-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  skip_to_start_btn_->setIcon(goToStartIcon);
  connect(skip_to_start_btn_, &QPushButton::clicked, this, &ViewerBase::seekToInPoint);
  playback_control_layout->addWidget(skip_to_start_btn_);

  rewind_btn_ = new QPushButton(playback_controls);
  rewind_btn_->setToolTip(tr("Step back 1 frame"));
  QIcon rewindIcon;
  rewindIcon.addFile(QStringLiteral(":/icons/rew.png"), QSize(), QIcon::Normal, QIcon::Off);
  rewindIcon.addFile(QStringLiteral(":/icons/rew-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  rewind_btn_->setIcon(rewindIcon);
  connect(rewind_btn_, &QPushButton::clicked, this, &ViewerBase::previousFrame);
  playback_control_layout->addWidget(rewind_btn_);

  play_btn_ = new QPushButton(playback_controls);
  play_btn_->setToolTip(tr("Toggle playback"));
  play_icon_.addFile(QStringLiteral(":/icons/play.png"), QSize(), QIcon::Normal, QIcon::On);
  play_icon_.addFile(QStringLiteral(":/icons/play-disabled.png"), QSize(), QIcon::Disabled, QIcon::On);
  play_btn_->setIcon(play_icon_);
  connect(play_btn_, &QPushButton::clicked, this, &ViewerBase::togglePlay);
  playback_control_layout->addWidget(play_btn_);

  fast_forward_btn_ = new QPushButton(playback_controls);
  fast_forward_btn_->setToolTip(tr("Step forward 1 frame"));
  QIcon ffIcon;
  ffIcon.addFile(QStringLiteral(":/icons/ff.png"), QSize(), QIcon::Normal, QIcon::On);
  ffIcon.addFile(QStringLiteral(":/icons/ff-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  fast_forward_btn_->setIcon(ffIcon);
  connect(fast_forward_btn_, &QPushButton::clicked, this, &ViewerBase::nextFrame);
  playback_control_layout->addWidget(fast_forward_btn_);

  skip_to_end_btn_ = new QPushButton(playback_controls);
  skip_to_end_btn_->setToolTip(tr("Go to Out"));
  QIcon nextIcon;
  nextIcon.addFile(QStringLiteral(":/icons/next.png"), QSize(), QIcon::Normal, QIcon::Off);
  nextIcon.addFile(QStringLiteral(":/icons/next-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  skip_to_end_btn_->setIcon(nextIcon);
  connect(skip_to_end_btn_, &QPushButton::clicked, this, &ViewerBase::seekToOutPoint);
  playback_control_layout->addWidget(skip_to_end_btn_);

  fx_mute_btn_ = new QPushButton(playback_controls);
  fx_mute_btn_->setCheckable(true);
  fx_mute_btn_->setToolTip(tr(""));
  QIcon mute_icon;
  mute_icon.addFile(QStringLiteral(":/icons/fxmute.png"), QSize(), QIcon::Normal, QIcon::Off);
  mute_icon.addFile(QStringLiteral(":/icons/fxmute-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  fx_mute_btn_->setIcon(mute_icon);
  fx_mute_btn_->setText("fx");
  QFont fx_font;
  fx_font.setItalic(true);
  fx_mute_btn_->setFont(fx_font);
  fx_mute_btn_->setMinimumSize(16, 16);
  fx_mute_btn_->setMaximumSize(24, 24);
  connect(fx_mute_btn_, &QPushButton::toggled, this, &ViewerBase::fxMute);
  playback_control_layout->addWidget(fx_mute_btn_);
  fx_mute_btn_->setVisible(false);
  fx_mute_btn_->setEnabled(false);

  lower_control_layout->addWidget(playback_controls);

  auto end_timecode_container = new QWidget(lower_controls);
  auto end_timecode_layout = new QHBoxLayout(end_timecode_container);
  end_timecode_layout->setSpacing(0);
  end_timecode_layout->setMargin(0);
  visible_timecode_ = new QLabel(end_timecode_container);
  visible_timecode_->setToolTip(tr("In/Out duration"));
  visible_timecode_->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
  end_timecode_layout->addWidget(visible_timecode_);
  lower_control_layout->addWidget(end_timecode_container);

  layout->addWidget(lower_controls);

  setWidget(contents);
  enableWidgets(false);
}


void ViewerBase::enableWidgets(const bool enable)
{
  current_timecode_->setEnabled(enable);
  skip_to_start_btn_->setEnabled(enable);
  rewind_btn_ ->setEnabled(enable);
  play_btn_ ->setEnabled(enable);
  fast_forward_btn_ ->setEnabled(enable);
  skip_to_end_btn_ ->setEnabled(enable);
  fx_mute_btn_ ->setEnabled(enable);
  fx_mute_btn_->setEnabled(enable);
  visible_timecode_->setEnabled(enable);
}


void ViewerBase::updateHeaderZoom()
{
  auto item = viewed_item_.lock();
  if (item == nullptr) {
    return;
  }
  const auto end_frame = item->endFrame();
  if (cached_end_frame_ != end_frame) {
    minimum_zoom_ = (end_frame > 0) ? (static_cast<double>(headers_->width()) / end_frame) : 1;
    headers_->setZoom(qMax(headers_->zoom(), minimum_zoom_));
    setScrollbarMaximum();
//    viewer_widget->waveform_zoom = headers_->zoom();
  } else {
    headers_->update();
  }
}


void ViewerBase::updatePlayheadTimecode(const int64_t p)
{

}

void ViewerBase::updateEndTimecode()
{

}


void ViewerBase::setScrollbarMaximum()
{
  if (auto item = viewed_item_.lock()) {
    headers_->setScrollbarMax(*horizontal_bar_, item->endFrame(), headers_->width());
  }
}

void ViewerBase::redraw()
{
  updateHeaderZoom();
  // FIXME: render
//  viewer_widget->frame_update();
  if (auto item = viewed_item_.lock()) {
    updatePlayheadTimecode(item->playhead());
  }
  updateEndTimecode();
}


void ViewerBase::updateWidgets(const bool reload_fx)
{
  if (sequence_viewer_) {
    PanelManager::refreshPanels(reload_fx);
  } else {
    redraw();
  }
}

void ViewerBase::scrollAreaHideScrollbars(const bool hide)
{
  // Prevents a weird momentary scrollbar whilst resizing down
  const auto policy = hide ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded;
  scroll_area_->setVerticalScrollBarPolicy(policy);
  scroll_area_->setHorizontalScrollBarPolicy(policy);
}


void ViewerBase::updateParents()
{
  updateWidgets(false);
}

