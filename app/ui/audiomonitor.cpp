/* 
 * Olive. Olive is a free non-linear video editor for Windows, macOS, and Linux.
 * Copyright (C) 2018  {{ organization }}
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
 *along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "audiomonitor.h"

#include <QPainter>
#include <QLinearGradient>
#include <QtMath>
#include <limits>

#include "project/sequence.h"
#include "playback/audio.h"
#include "panels/panelmanager.h"

extern "C" {
#include "libavformat/avformat.h"
}

constexpr int AUDIO_MONITOR_PEAK_HEIGHT = 15;
constexpr int AUDIO_MONITOR_GAP = 3;

AudioMonitor::AudioMonitor(QWidget *parent) : QWidget(parent)
{
  reset();
}

void AudioMonitor::reset()
{
  sample_cache_offset = -1;
  sample_cache.clear();
  update();
}

void AudioMonitor::resizeEvent(QResizeEvent *e)
{
  gradient = QLinearGradient(QPoint(0, rect().top()), QPoint(0, rect().bottom()));
  gradient.setColorAt(0, Qt::red);
  gradient.setColorAt(0.25, Qt::yellow);
  gradient.setColorAt(1, Qt::green);
  QWidget::resizeEvent(e);
}

void AudioMonitor::paintEvent(QPaintEvent *)
{
  if (global::sequence == nullptr) {
    return;
  }

  QPainter p(this);
  int channel_x = AUDIO_MONITOR_GAP;
  const int channel_count = av_get_channel_layout_nb_channels(static_cast<uint64_t>(global::sequence->audioLayout()));

  const int channel_width = (width() / channel_count) - AUDIO_MONITOR_GAP;
  int32_t playhead_offset = -1;
  for (auto i = 0; i < channel_count; i++) {
    QRect r(channel_x, AUDIO_MONITOR_PEAK_HEIGHT + AUDIO_MONITOR_GAP, channel_width, height());
    p.fillRect(r, gradient);

    if ( (sample_cache_offset != -1) && !sample_cache.empty()) {
      playhead_offset = static_cast<int32_t>((global::sequence->playhead_ - sample_cache_offset) * channel_count);
      if ( (playhead_offset >= 0) && (playhead_offset < sample_cache.size()) ) {
        const double multiplier = 1 - qAbs(static_cast<double>(sample_cache.at(playhead_offset + i))
                                           / std::numeric_limits<int16_t>::max()); // 16-bit int divided to float
        r.setHeight(qRound(r.height() * multiplier));
      } else {
        reset();
      }
    }
    p.fillRect(r, QColor(0, 0, 0, 160));

    channel_x += channel_width + AUDIO_MONITOR_GAP;
  }

  if (playhead_offset > -1) {
    // clean up used samples
    bool error = false;
    while (sample_cache_offset < global::sequence->playhead_ && !error) {
      sample_cache_offset++;
      for (auto i = 0; i<channel_count; i++) {
        if (sample_cache.isEmpty()) {
          reset();
          error = true;
          break;
        } else {
          sample_cache.removeFirst();
        }
      }//for
    }//while
  }
}
