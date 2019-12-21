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

#ifndef FOOTAGEVIEWER_H
#define FOOTAGEVIEWER_H

#include <QTimer>

#include "viewerbase.h"
#include "project/footagestream.h"


namespace chestnut::panels
{

  class FootageViewer : public ViewerBase
  {
      Q_OBJECT
      Q_INTERFACES(chestnut::panels::ViewerBase)
    public:
      explicit FootageViewer(QWidget* parent = nullptr);
      void clear() override;

      void setMedia(MediaWPtr mda) override;
      void seek(const int64_t position) override;
      void play() override;
      void pause() override;
      bool isPlaying() const override;

      void zoomIn() override;
      void zoomOut() override;

      void togglePointsEnabled() override;
      void setInPoint() override;
      void setOutPoint() override;
      void clearInPoint() override;
      void clearOutPoint() override;
      void clearPoints() override;

      bool isFocused() const override;
    signals:
      void started() override;
      void stopped() override;

    public slots:
      void nextFrame() override;
      void previousFrame() override;

      void seekToStart() override;
      void seekToEnd() override;
      void seekToInPoint() override;
      void seekToOutPoint() override;

      void fxMute(const bool) override {}
    protected:
      void updatePanelTitle() override;
    private slots:
      void onTimeout();
    private:
      FootagePtr footage_ {nullptr};
      QTimer play_timer_;
      ::project::FootageStreamPtr video_track_ {nullptr};
      std::vector<::project::FootageStreamPtr> audio_tracks_;
      bool playing_ {false};
      bool end_of_stream_{false};

      void setupTimer(const double frame_rate);
      void setPlayPauseIcon(const bool playing);
  };
}

#endif // FOOTAGEVIEWER_H
