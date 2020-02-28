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

#ifndef VIEWERBASE_H
#define VIEWERBASE_H

#include <QDockWidget>
#include <QPushButton>
#include <QScrollArea>

#include "project/media.h"
#include "ui/timelineheader.h"
#include "ui/resizablescrollbar.h"
#include "ui/labelslider.h"
#include "ui/imagecanvas.h"
#include "ui/menuscrollarea.h"
#include "ui/viewertimeline.h"

namespace chestnut::panels
{

  class ViewerBase : public QDockWidget
  {
    public:
      explicit ViewerBase(QWidget* parent);
      ~ViewerBase() override;

      virtual void setName(QString name);
      virtual void setMedia(project::MediaWPtr mda);
      virtual bool hasMedia() const noexcept;

      virtual void seek(const int64_t position) = 0;
      virtual void play() = 0;
      virtual void pause() = 0;
      virtual bool isPlaying() const = 0;

      virtual void zoomIn() = 0;
      virtual void zoomOut() = 0;
      virtual void setZoom(const double value);

      virtual void togglePointsEnabled() = 0;
      virtual void setInPoint() = 0;
      virtual void setOutPoint() = 0;
      virtual void clearInPoint() = 0;
      virtual void clearOutPoint() = 0;
      virtual void clearPoints() = 0;

      virtual bool isFocused() const = 0;
    public slots:
      virtual void togglePlay();
      virtual void nextFrame() = 0;
      virtual void previousFrame() = 0;

      virtual void seekToStart() = 0;
      virtual void seekToEnd() = 0;
      virtual void seekToInPoint() = 0;
      virtual void seekToOutPoint() = 0;
      virtual void fxMute(const bool value) = 0;
      virtual void clear();
    signals:
      virtual void started() = 0;
      virtual void stopped() = 0;

    protected:
      virtual void updatePanelTitle() = 0;
      void resizeEvent(QResizeEvent* event) override;

    private:
      friend class FootageViewer;
      project::MediaWPtr media_;
      QString name_;

      QIcon play_icon_;

      ui::ViewerTimeline* headers_ {nullptr};
      ResizableScrollBar* horizontal_bar_ {nullptr};
      ui::MenuScrollArea* scroll_area_ {nullptr};
      ui::ImageCanvas* frame_view_ {nullptr};
      LabelSlider* current_timecode_ {nullptr};
      QLabel* visible_timecode_ {nullptr};

      QPushButton* skip_to_start_btn_ {nullptr};
      QPushButton* rewind_btn_ {nullptr};
      QPushButton* play_btn_ {nullptr};
      QPushButton* fast_forward_btn_ {nullptr};
      QPushButton* skip_to_end_btn_ {nullptr};
      QPushButton* fx_mute_btn_ {nullptr};
    private:
    private:
      void setupWidgets();
      void enableWidgets(const bool enable);
    private slots:
      void scrollAreaHideScrollbars(const bool hide);
  };
}
Q_DECLARE_INTERFACE(chestnut::panels::ViewerBase, "ViewerBase")

#endif // VIEWERBASE_H
