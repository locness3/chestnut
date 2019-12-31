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

#include "audioworker.h"
#include "debug.h"
#include "defaults.h"

using chestnut::system::AudioWorker;

AudioWorker::AudioWorker(QObject *parent) : QObject(parent)
{
}

AudioWorker::~AudioWorker()
{

}

void AudioWorker::queueData(QByteArray data)
{
  QMutexLocker lock(&queue_mutex_);
  queue_.push_back(std::move(data));
  wait_cond_.wakeAll();
}

void AudioWorker::process()
{
  qDebug() << "Audio playback worker started";
  while (running_) {
    wait_cond_.wait(&mutex_);
    QMutexLocker lock(&queue_mutex_);
    while (!queue_.empty() && running_) {
      const auto samples(queue_.dequeue());
      Q_ASSERT(audio_io_);
      audio_io_->write(samples.data(), samples.size());
    }
  }
  qWarning() << "Audio playback worker finished";
  emit finished();
}

void AudioWorker::setup(const int channels)
{
  QAudioFormat format;
  format.setSampleRate(chestnut::defaults::audio::INTERNAL_FREQUENCY);
  format.setChannelCount(channels);
  format.setSampleSize(chestnut::defaults::audio::INTERNAL_DEPTH);
  format.setCodec(chestnut::defaults::audio::INTERNAL_CODEC);
  format.setByteOrder(QAudioFormat::LittleEndian);
  format.setSampleType(chestnut::defaults::audio::INTERNAL_TYPE);

  const auto device = QAudioDeviceInfo::defaultOutputDevice();
  if (!device.isFormatSupported(format)) {
    qWarning() << "Format not supported";
  }

  audio_output_ = std::make_unique<QAudioOutput>(device, format);
  audio_io_ = audio_output_->start();
  Q_ASSERT(audio_io_);
}
