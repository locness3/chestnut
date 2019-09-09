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
#include "exportdialog.h"

#include <QOpenGLWidget>
#include <QFileDialog>
#include <QThread>
#include <QMessageBox>
#include <QtMath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QProgressBar>
#include <QStandardPaths>
#include <limits>

#include "debug.h"
#include "panels/panelmanager.h"
#include "ui/viewerwidget.h"
#include "project/sequence.h"
#include "io/exportthread.h"
#include "playback/playback.h"
#include "ui/mainwindow.h"
#include "coderconstants.h"

extern "C" {
#include <libavformat/avformat.h>
}

constexpr auto X264_DEFAULT_CRF = 23;
constexpr auto DEFAULT_B_FRAMES = 2;
constexpr auto MIN_H264_LEVEL = 2;
constexpr auto MAX_H264_LEVEL = 5;


namespace
{
  const QStringList MPEG2_PROFILES {MPEG2_SIMPLE_PROFILE, MPEG2_MAIN_PROFILE, MPEG2_HIGH_PROFILE, MPEG2_422_PROFILE};
  const QStringList MPEG2_LEVELS {MPEG2_LOW_LEVEL, MPEG2_MAIN_LEVEL, MPEG2_HIGH1440_LEVEL, MPEG2_HIGH_LEVEL};
  const QStringList H264_PROFILES {H264_BASELINE_PROFILE, H264_EXTENDED_PROFILE, H264_MAIN_PROFILE, H264_HIGH_PROFILE};
  const QStringList FRAME_RATES {"10", "12", "15", "23.976", "24", "25", "29.97", "30", "48", "50", "59.94", "60"};
}

enum ExportFormats {
  FORMAT_3GPP,
  FORMAT_AIFF,
  FORMAT_APNG,
  FORMAT_AVI,
  FORMAT_DNXHD,
  FORMAT_AC3,
  FORMAT_FLV,
  FORMAT_GIF,
  FORMAT_IMG,
  FORMAT_MP2,
  FORMAT_MP3,
  FORMAT_MPEG1,
  FORMAT_MPEG2,
  FORMAT_MPEG4,
  FORMAT_MPEGTS,
  FORMAT_MKV,
  FORMAT_OGG,
  FORMAT_MOV,
  FORMAT_WAV,
  FORMAT_WEBM,
  FORMAT_WMV,
  FORMAT_SIZE
};

struct LevelConstraints
{
    int max_width;
    int max_height;
    int max_bitrate;
    QStringList framerates;
};

static const LevelConstraints MPEG2_LL_CONSTRAINTS {352, 288, 4000000, MPEG2_LOW_LEVEL_FRATES};
static const LevelConstraints MPEG2_ML_CONSTRAINTS {720, 576, 15000000, MPEG2_MAIN_LEVEL_FRATES};
static const LevelConstraints MPEG2_H14_CONSTRAINTS {1440, 1152, 60000000, MPEG2_HIGH1440_LEVEL_FRATES};
static const LevelConstraints MPEG2_HL_CONSTRAINTS {1920, 1080, 80000000, MPEG2_HIGH_LEVEL_FRATES};

static const LevelConstraints H264_20_CONSTRAINTS {-1, -1, 2000000, {}};
static const LevelConstraints H264_21_CONSTRAINTS {-1, -1, 4000000, {}};
static const LevelConstraints H264_22_CONSTRAINTS {-1, -1, 4000000, {}};
static const LevelConstraints H264_30_CONSTRAINTS {-1, -1, 10000000, {}};
static const LevelConstraints H264_31_CONSTRAINTS {-1, -1, 14000000, {}};
static const LevelConstraints H264_32_CONSTRAINTS {-1, -1, 20000000, {}};
static const LevelConstraints H264_40_CONSTRAINTS {-1, -1, 20000000, {}};
static const LevelConstraints H264_41_CONSTRAINTS {-1, -1, 50000000, {}};
static const LevelConstraints H264_42_CONSTRAINTS {-1, -1, 50000000, {}};
static const LevelConstraints H264_50_CONSTRAINTS {-1, -1, 135000000, {}};
static const LevelConstraints H264_51_CONSTRAINTS {-1, -1, 240000000, {}};
static const LevelConstraints H264_52_CONSTRAINTS {-1, -1, 240000000, {}};


using panels::PanelManager;

ExportDialog::ExportDialog(SequencePtr seq, QWidget *parent)
  : QDialog(parent),
    sequence_(std::move(seq)),
    output_dir_()
{
  Q_ASSERT(sequence_);
  if (!QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).empty()) {
    output_dir_ = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();
  }
  setWindowTitle(tr("Export \"%1\"").arg(sequence_->name()));
  setup_ui();

  rangeCombobox->setCurrentIndex(0);
  if (sequence_->workarea_.using_) {
    rangeCombobox->setEnabled(true);
    if (sequence_->workarea_.enabled_) {
      rangeCombobox->setCurrentIndex(1);
    }
  }

  format_strings.resize(FORMAT_SIZE);
  format_strings[FORMAT_3GPP] = "3GPP";
  format_strings[FORMAT_AIFF] = "AIFF";
  format_strings[FORMAT_APNG] = "Animated PNG";
  format_strings[FORMAT_AVI] = "AVI";
  format_strings[FORMAT_DNXHD] = "DNxHD";
  format_strings[FORMAT_AC3] = "Dolby Digital (AC3)";
  format_strings[FORMAT_FLV] = "FLV";
  format_strings[FORMAT_GIF] = "GIF";
  format_strings[FORMAT_IMG] = "Image Sequence";
  format_strings[FORMAT_MP2] = "MP2 Audio";
  format_strings[FORMAT_MP3] = "MP3 Audio";
  format_strings[FORMAT_MPEG1] = "MPEG-1 Video";
  format_strings[FORMAT_MPEG2] = "MPEG-2 Video";
  format_strings[FORMAT_MPEG4] = "MPEG-4 Video";
  format_strings[FORMAT_MPEGTS] = "MPEG-TS";
  format_strings[FORMAT_MKV] = "Matroska MKV";
  format_strings[FORMAT_OGG] = "Ogg";
  format_strings[FORMAT_MOV] = "QuickTime MOV";
  format_strings[FORMAT_WAV] = "WAVE Audio";
  format_strings[FORMAT_WEBM] = "WebM";
  format_strings[FORMAT_WMV] = "Windows Media";

  for (int i=0;i<FORMAT_SIZE;i++) {
    formatCombobox->addItem(format_strings[i]);
  }
  formatCombobox->setCurrentIndex(FORMAT_MPEG4);

  widthSpinbox->setValue(sequence_->width());
  heightSpinbox->setValue(sequence_->height());
  samplingRateSpinbox->setValue(sequence_->audioFrequency());
  const int ix = framerate_box_->findText(QString::number(sequence_->frameRate(), 'g', 4));
  framerate_box_->setCurrentIndex(ix);
  gop_length_box_->setValue(qRound(sequence_->frameRate() * 10));
  b_frame_box_->setValue(DEFAULT_B_FRAMES);
}

ExportDialog::~ExportDialog()
{

}

void ExportDialog::format_changed(int index)
{
  assert(vcodecCombobox);
  assert(acodecCombobox);
  assert(profile_box_);
  assert(level_box_);


  profile_box_->setEnabled(false);
  level_box_->setEnabled(false);

  format_vcodecs.clear();
  format_acodecs.clear();
  vcodecCombobox->clear();
  acodecCombobox->clear();

  int default_vcodec = 0;
  int default_acodec = 0;

  switch (index) {
    case FORMAT_3GPP:
      format_vcodecs.append(AV_CODEC_ID_MPEG4);
      format_vcodecs.append(AV_CODEC_ID_H264);

      format_acodecs.append(AV_CODEC_ID_AAC);

      default_vcodec = 1;
      break;
    case FORMAT_AIFF:
      format_acodecs.append(AV_CODEC_ID_PCM_S16LE);
      break;
    case FORMAT_APNG:
      format_vcodecs.append(AV_CODEC_ID_APNG);
      break;
    case FORMAT_AVI:
      format_vcodecs.append(AV_CODEC_ID_H264);
      format_vcodecs.append(AV_CODEC_ID_MPEG4);
      format_vcodecs.append(AV_CODEC_ID_MJPEG);
      format_vcodecs.append(AV_CODEC_ID_MSVIDEO1);
      format_vcodecs.append(AV_CODEC_ID_RAWVIDEO);
      format_vcodecs.append(AV_CODEC_ID_HUFFYUV);
      format_vcodecs.append(AV_CODEC_ID_DVVIDEO);

      format_acodecs.append(AV_CODEC_ID_AAC);
      format_acodecs.append(AV_CODEC_ID_AC3);
      format_acodecs.append(AV_CODEC_ID_FLAC);
      format_acodecs.append(AV_CODEC_ID_MP2);
      format_acodecs.append(AV_CODEC_ID_MP3);
      format_acodecs.append(AV_CODEC_ID_PCM_S16LE);

      default_vcodec = 3;
      default_acodec = 5;
      break;
    case FORMAT_DNXHD:
      format_vcodecs.append(AV_CODEC_ID_DNXHD);

      format_acodecs.append(AV_CODEC_ID_PCM_S16LE);
      break;
    case FORMAT_AC3:
      format_acodecs.append(AV_CODEC_ID_AC3);
      format_acodecs.append(AV_CODEC_ID_EAC3);
      break;
    case FORMAT_FLV:
      format_vcodecs.append(AV_CODEC_ID_FLV1);

      format_acodecs.append(AV_CODEC_ID_MP3);
      break;
    case FORMAT_GIF:
      format_vcodecs.append(AV_CODEC_ID_GIF);
      break;
    case FORMAT_IMG:
      format_vcodecs.append(AV_CODEC_ID_BMP);
      format_vcodecs.append(AV_CODEC_ID_MJPEG);
      format_vcodecs.append(AV_CODEC_ID_JPEG2000);

#ifndef DISABLE_PSD
      format_vcodecs.append(AV_CODEC_ID_PSD);
#endif
      format_vcodecs.append(AV_CODEC_ID_PNG);
      format_vcodecs.append(AV_CODEC_ID_TIFF);

      default_vcodec = 4;
      break;
    case FORMAT_MP2:
      format_acodecs.append(AV_CODEC_ID_MP2);
      break;
    case FORMAT_MP3:
      format_acodecs.append(AV_CODEC_ID_MP3);
      break;
    case FORMAT_MPEG1:
      format_vcodecs.append(AV_CODEC_ID_MPEG1VIDEO);

      format_acodecs.append(AV_CODEC_ID_AC3);
      format_acodecs.append(AV_CODEC_ID_MP2);
      format_acodecs.append(AV_CODEC_ID_MP3);
      format_acodecs.append(AV_CODEC_ID_PCM_S16LE);

      default_acodec = 1;
      break;
    case FORMAT_MPEG2:
      setupForMpeg2();
      default_acodec = 1;
      break;
    case FORMAT_MPEG4:
      setupForMpeg4();
      default_vcodec = 1;
      break;
    case FORMAT_MPEGTS:
      format_vcodecs.append(AV_CODEC_ID_MPEG2VIDEO);

      format_acodecs.append(AV_CODEC_ID_AAC);
      format_acodecs.append(AV_CODEC_ID_AC3);
      format_acodecs.append(AV_CODEC_ID_MP2);
      format_acodecs.append(AV_CODEC_ID_MP3);

      default_acodec = 2;
      break;
    case FORMAT_MKV:
      format_vcodecs.append(AV_CODEC_ID_MPEG4);
      format_vcodecs.append(AV_CODEC_ID_H264);

      format_acodecs.append(AV_CODEC_ID_AAC);
      format_acodecs.append(AV_CODEC_ID_AC3);
      format_acodecs.append(AV_CODEC_ID_EAC3);
      format_acodecs.append(AV_CODEC_ID_FLAC);
      format_acodecs.append(AV_CODEC_ID_MP2);
      format_acodecs.append(AV_CODEC_ID_MP3);
      format_acodecs.append(AV_CODEC_ID_OPUS);
      format_acodecs.append(AV_CODEC_ID_PCM_S16LE);
      format_acodecs.append(AV_CODEC_ID_VORBIS);
      format_acodecs.append(AV_CODEC_ID_WAVPACK);
      format_acodecs.append(AV_CODEC_ID_WMAV1);
      format_acodecs.append(AV_CODEC_ID_WMAV2);

      default_vcodec = 1;
      break;
    case FORMAT_OGG:
      format_vcodecs.append(AV_CODEC_ID_THEORA);

      format_acodecs.append(AV_CODEC_ID_OPUS);
      format_acodecs.append(AV_CODEC_ID_VORBIS);

      default_acodec = 1;
      break;
    case FORMAT_MOV:
      format_vcodecs.append(AV_CODEC_ID_QTRLE);
      format_vcodecs.append(AV_CODEC_ID_MPEG4);
      format_vcodecs.append(AV_CODEC_ID_H264);
      format_vcodecs.append(AV_CODEC_ID_MJPEG);
      format_vcodecs.append(AV_CODEC_ID_PRORES);

      format_acodecs.append(AV_CODEC_ID_AAC);
      format_acodecs.append(AV_CODEC_ID_AC3);
      format_acodecs.append(AV_CODEC_ID_MP2);
      format_acodecs.append(AV_CODEC_ID_MP3);
      format_acodecs.append(AV_CODEC_ID_PCM_S16LE);

      default_vcodec = 2;
      break;
    case FORMAT_WAV:
      format_acodecs.append(AV_CODEC_ID_PCM_S16LE);
      break;
    case FORMAT_WEBM:
      format_vcodecs.append(AV_CODEC_ID_VP8);
      format_vcodecs.append(AV_CODEC_ID_VP9);

      format_acodecs.append(AV_CODEC_ID_OPUS);
      format_acodecs.append(AV_CODEC_ID_VORBIS);

      default_vcodec = 1;
      break;
    case FORMAT_WMV:
      format_vcodecs.append(AV_CODEC_ID_WMV1);
      format_vcodecs.append(AV_CODEC_ID_WMV2);

      format_acodecs.append(AV_CODEC_ID_WMAV1);
      format_acodecs.append(AV_CODEC_ID_WMAV2);

      default_vcodec = 1;
      default_acodec = 1;
      break;
    default:
      qCritical() << "Invalid format selection - this is a bug, please inform the developers";
  }

  AVCodec* codec_info;
  for (const auto& fmt: format_vcodecs) {
    codec_info = avcodec_find_encoder(static_cast<AVCodecID>(fmt));
    if (codec_info == nullptr) {
      vcodecCombobox->addItem("nullptr");
    } else {
      vcodecCombobox->addItem(codec_info->long_name);
    }
  }
  for (auto& acodec : format_acodecs) {
    codec_info = avcodec_find_encoder(static_cast<AVCodecID>(acodec));
    if (codec_info == nullptr) {
      acodecCombobox->addItem("nullptr");
    } else {
      acodecCombobox->addItem(codec_info->long_name);
    }
  }

  vcodecCombobox->setCurrentIndex(default_vcodec);
  acodecCombobox->setCurrentIndex(default_acodec);

  const auto video_enabled = format_vcodecs.size() != 0;
  const auto audio_enabled = format_acodecs.size() != 0;
  videoGroupbox->setChecked(video_enabled);
  audioGroupbox->setChecked(audio_enabled);
  videoGroupbox->setEnabled(video_enabled);
  audioGroupbox->setEnabled(audio_enabled);
}

void ExportDialog::render_thread_finished()
{
  if (progressBar->value() < 100 && !cancelled) {
    QMessageBox::critical(
          this,
          tr("Export Failed"),
          tr("Export failed - %1").arg(export_error),
          QMessageBox::Ok
          );
  }
  prep_ui_for_render(false);
  PanelManager::sequenceViewer().viewer_widget->makeCurrent();
  PanelManager::sequenceViewer().viewer_widget->initializeGL();
  PanelManager::refreshPanels(false);
  if (progressBar->value() >= 100) {
    accept();
  }
}

void ExportDialog::prep_ui_for_render(bool r)
{
  export_button->setEnabled(!r);
  cancel_button->setEnabled(!r);
  renderCancel->setEnabled(r);
}

void ExportDialog::export_action()
{
  Q_ASSERT(formatCombobox);
  Q_ASSERT(widthSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);
  Q_ASSERT(closed_gop_box_);

  if ( (widthSpinbox->value() % 2) == 1 ||  (heightSpinbox->value() % 2) == 1) {
    QMessageBox::critical(
          this,
          tr("Invalid dimensions"),
          tr("Export width and height must both be even numbers/divisible by 2."),
          QMessageBox::Ok
          );
    return;
  }

  QString ext;
  switch (formatCombobox->currentIndex()) {
    case FORMAT_3GPP:
      ext = "3gp";
      break;
    case FORMAT_AIFF:
      ext = "aiff";
      break;
    case FORMAT_APNG:
      ext = "apng";
      break;
    case FORMAT_AVI:
      ext = "avi";
      break;
    case FORMAT_DNXHD:
      ext = "mxf";
      break;
    case FORMAT_AC3:
      ext = "ac3";
      break;
    case FORMAT_FLV:
      ext = "flv";
      break;
    case FORMAT_GIF:
      ext = "gif";
      break;
    case FORMAT_IMG:
      switch (format_vcodecs.at(vcodecCombobox->currentIndex())) {
        case AV_CODEC_ID_BMP:
          ext = "bmp";
          break;
        case AV_CODEC_ID_MJPEG:
          ext = "jpg";
          break;
        case AV_CODEC_ID_JPEG2000:
          ext = "jp2";
          break;
#ifndef DISABLE_PSD
        case AV_CODEC_ID_PSD:
          ext = "psd";
          break;
#endif
        case AV_CODEC_ID_PNG:
          ext = "png";
          break;
        case AV_CODEC_ID_TIFF:
          ext = "tif";
          break;
        default:
          qCritical() << "Invalid codec selection for an image sequence";
          QMessageBox::critical(
                this,
                tr("Invalid codec"),
                tr("Couldn't determine output parameters for the selected codec. This is a bug, please contact the developers."),
                QMessageBox::Ok
                );
          return;
      }
      break;
    case FORMAT_MP3:
      ext = "mp3";
      break;
    case FORMAT_MPEG1:
      if (videoGroupbox->isChecked() && !audioGroupbox->isChecked()) {
        ext = "m1v";
      } else if (!videoGroupbox->isChecked() && audioGroupbox->isChecked()) {
        ext = "m1a";
      } else {
        ext = "mpg";
      }
      break;
    case FORMAT_MPEG2:
      if (videoGroupbox->isChecked() && !audioGroupbox->isChecked()) {
        ext = "m2v";
      } else if (!videoGroupbox->isChecked() && audioGroupbox->isChecked()) {
        ext = "m2a";
      } else {
        ext = "mpg";
      }
      break;
    case FORMAT_MPEG4:
      if (videoGroupbox->isChecked() && !audioGroupbox->isChecked()) {
        ext = "m4v";
      } else if (!videoGroupbox->isChecked() && audioGroupbox->isChecked()) {
        ext = "m4a";
      } else {
        ext = "mp4";
      }
      break;
    case FORMAT_MPEGTS:
      ext = "ts";
      break;
    case FORMAT_MKV:
      if (!videoGroupbox->isChecked()) {
        ext = "mka";
      } else {
        ext = "mkv";
      }
      break;
    case FORMAT_OGG:
      ext = "ogg";
      break;
    case FORMAT_MOV:
      ext = "mov";
      break;
    case FORMAT_WAV:
      ext = "wav";
      break;
    case FORMAT_WEBM:
      ext = "webm";
      break;
    case FORMAT_WMV:
      if (videoGroupbox->isChecked()) {
        ext = "wmv";
      } else {
        ext = "wma";
      }
      break;
    default:
      qCritical() << "Invalid format - this is a bug, please inform the developers";
      QMessageBox::critical(
            this,
            tr("Invalid format"),
            tr("Couldn't determine output format. This is a bug, please contact the developers."),
            QMessageBox::Ok
            );
      return;
  }
  QString filename = QFileDialog::getSaveFileName(
                       this,
                       tr("Export Media"),
                       output_dir_,
                       format_strings[formatCombobox->currentIndex()] + " (*." + ext + ")"
                     );
  if (!filename.isEmpty()) {
    if (!filename.endsWith("." + ext, Qt::CaseInsensitive)) {
      filename += "." + ext;
    }

    if (formatCombobox->currentIndex() == FORMAT_IMG) {
      int ext_location = filename.lastIndexOf('.');
      if (ext_location > filename.lastIndexOf('/')) {
        filename.insert(ext_location, 'd');
        filename.insert(ext_location, '5');
        filename.insert(ext_location, '0');
        filename.insert(ext_location, '%');
      }
    }

    et = new ExportThread();

    connect(et, SIGNAL(finished()), et, SLOT(deleteLater()));
    connect(et, SIGNAL(finished()), this, SLOT(render_thread_finished()));
    connect(et, SIGNAL(progress_changed(int, qint64)), this, SLOT(update_progress_bar(int, qint64)));

    sequence_->closeActiveClips();

    MainWindow::instance().set_rendering_state(true);

    MainWindow::instance().autorecover_interval();

    e_rendering = true;
    PanelManager::sequenceViewer().viewer_widget->context()->doneCurrent();
    PanelManager::sequenceViewer().viewer_widget->context()->moveToThread(et);

    prep_ui_for_render(true);

    et->filename = filename;
    et->video_params.enabled = videoGroupbox->isChecked();
    if (et->video_params.enabled) {
      et->video_params.codec = format_vcodecs.at(vcodecCombobox->currentIndex());
      et->video_params.width = widthSpinbox->value();
      et->video_params.height = heightSpinbox->value();
      bool ok;
      et->video_params.frame_rate = framerate_box_->currentText().toDouble(&ok);
      assert(ok);
      et->video_params.compression_type = static_cast<CompressionType>(compressionTypeCombobox->currentData().toInt());
      et->video_params.bitrate = videobitrateSpinbox->value();
      et->video_params.gop_length_ = gop_length_box_->value();
      et->video_params.closed_gop_ = closed_gop_box_->isChecked();
      et->video_params.b_frames_ = b_frame_box_->value();
      et->video_params.profile_ = profile_box_->currentText().toStdString();
      et->video_params.level_ = level_box_->currentText().toStdString();
    }
    et->audio_params.enabled = audioGroupbox->isChecked();
    if (et->audio_params.enabled) {
      et->audio_params.codec = format_acodecs.at(acodecCombobox->currentIndex());
      et->audio_params.sampling_rate = samplingRateSpinbox->value();
      et->audio_params.bitrate = audiobitrateSpinbox->value();
    }

    et->start_frame = 0;
    et->end_frame = sequence_->endFrame(); // entire sequence
    if (rangeCombobox->currentIndex() == 1) {
      et->start_frame = qMax(sequence_->workarea_.in_, et->start_frame);
      et->end_frame = qMin(sequence_->workarea_.out_, et->end_frame);
    }

    et->ed = this;
    cancelled = false;

    et->start();
  }
}

void ExportDialog::update_progress_bar(int value, qint64 remaining_ms)
{
  // convert ms to H:MM:SS
  const int seconds = qFloor(remaining_ms * 0.001) % 60;
  const int minutes = qFloor(remaining_ms / 60000) % 60;
  const int hours = qFloor(remaining_ms / 3600000);
  progressBar->setFormat("%p% (ETA: " + QString::number(hours) + ":" + QString::number(minutes).rightJustified(2, '0')
                         + ":" + QString::number(seconds).rightJustified(2, '0') + ")");

  progressBar->setValue(value);
}

void ExportDialog::cancel_render() {
  et->continue_encode_ = false;
  cancelled = true;
  et->wake();
}

void ExportDialog::vcodec_changed(int index)
{
  Q_ASSERT(compressionTypeCombobox);
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);
  Q_ASSERT(formatCombobox);

  compressionTypeCombobox->clear();
  if (!format_vcodecs.empty() && (format_vcodecs.at(index) == AV_CODEC_ID_H264)) {
    compressionTypeCombobox->setEnabled(true);
    compressionTypeCombobox->addItem(tr("Quality-based (Constant Rate Factor)"), static_cast<int>(CompressionType::CFR));
    setupForH264();
  } else if (formatCombobox->currentIndex() == FORMAT_MPEG2) {
    compressionTypeCombobox->addItem(tr("Constant Bitrate"), static_cast<int>(CompressionType::CBR));
    compressionTypeCombobox->setCurrentIndex(0);
    compressionTypeCombobox->setEnabled(false);
  } else {
    compressionTypeCombobox->addItem(tr("Constant Bitrate"), static_cast<int>(CompressionType::CBR));
    compressionTypeCombobox->setCurrentIndex(0);
    compressionTypeCombobox->setEnabled(false);
    profile_box_->clear();
    profile_box_->setEnabled(false);
    level_box_->clear();
    level_box_->setEnabled(false);
  }
}

void ExportDialog::comp_type_changed(int)
{
  videobitrateSpinbox->setToolTip("");
  videobitrateSpinbox->setMinimum(0);
  videobitrateSpinbox->setMaximum(99.99);
  const auto compressionType = static_cast<CompressionType>(compressionTypeCombobox->currentData().toInt());
  switch (compressionType) {
    case CompressionType::CBR:
    case CompressionType::TARGETBITRATE:
      videoBitrateLabel->setText(tr("Bitrate (Mbps):"));
      videobitrateSpinbox->setValue(qMax(0.5, (double) qRound((0.01528 * sequence_->height()) - 4.5))); // FIXME: magic numbers
      break;
    case CompressionType::CFR:
      videoBitrateLabel->setText(tr("Quality (CRF):"));
      videobitrateSpinbox->setValue(X264_DEFAULT_CRF);
      videobitrateSpinbox->setMinimum(X264_MINIMUM_CRF);
      videobitrateSpinbox->setMaximum(X264_MAXIMUM_CRF);
      videobitrateSpinbox->setToolTip(tr("Quality Factor:\n\n0 = lossless\n17-18 = visually lossless "
                                         "(compressed, but unnoticeable)\n23 = high quality\n51 = lowest quality possible"));
      break;
    case CompressionType::TARGETSIZE:
      videoBitrateLabel->setText(tr("Target File Size (MB):"));
      videobitrateSpinbox->setValue(100);
      break;
    default:
      qWarning() << "Unhandled selected Compression type" << static_cast<int>(compressionType);
      break;
  }//switch
}


void ExportDialog::profile_changed(int)
{
  Q_ASSERT(formatCombobox);
  Q_ASSERT(vcodecCombobox);

  const auto format = formatCombobox->currentText();
  if (format == format_strings[FORMAT_MPEG2]) {
    constrainMpeg2();
  } else if ( (format == format_strings[FORMAT_MPEG4]) &&
              (vcodecCombobox->currentText().toStdString() == getCodecLongName(AV_CODEC_ID_H264)) ) { // possible to use format_vcodecs but aim to remove that
    constrainH264();
  } else {
    unconstrain();
  }
}

void ExportDialog::level_changed(int)
{
  assert(formatCombobox);
  const auto format = formatCombobox->currentText();
  if (format == format_strings[FORMAT_MPEG2]) {
    constrainMpeg2();
  } else if ( (format == format_strings[FORMAT_MPEG4]) &&
              (vcodecCombobox->currentText().toStdString() == getCodecLongName(AV_CODEC_ID_H264)) ) { // possible to use format_vcodecs but aim to remove that
    constrainH264();
  } else {
    unconstrain();
  }
}

void ExportDialog::setup_ui()
{
  auto verticalLayout = new QVBoxLayout(this);

  auto format_layout = new QHBoxLayout();

  format_layout->addWidget(new QLabel(tr("Format:")));

  formatCombobox = new QComboBox(this);

  format_layout->addWidget(formatCombobox);

  verticalLayout->addLayout(format_layout);

  auto range_layout = new QHBoxLayout();

  range_layout->addWidget(new QLabel(tr("Range:")));

  rangeCombobox = new QComboBox(this);
  rangeCombobox->addItem(tr("Entire Sequence"));
  rangeCombobox->addItem(tr("In to Out"));

  range_layout->addWidget(rangeCombobox);

  verticalLayout->addLayout(range_layout);

  videoGroupbox = new QGroupBox(this);
  videoGroupbox->setTitle(tr("Video"));
  videoGroupbox->setFlat(false);
  videoGroupbox->setCheckable(true);

  auto videoGridLayout = new QGridLayout(videoGroupbox);

  int row = 0;
  videoGridLayout->addWidget(new QLabel(tr("Codec:")), row, 0, 1, 1);
  vcodecCombobox = new QComboBox(videoGroupbox);
  videoGridLayout->addWidget(vcodecCombobox, row, 1, 1, 1);
  row++;

  videoGridLayout->addWidget(new QLabel(tr("Profile")), row, 0, 1, 1);
  profile_box_ = new QComboBox(videoGroupbox);
  videoGridLayout->addWidget(profile_box_, row, 1, 1, 1);
  row++;

  videoGridLayout->addWidget(new QLabel(tr("Level")), row, 0, 1, 1);
  level_box_ = new QComboBox(videoGroupbox);
  videoGridLayout->addWidget(level_box_, row, 1, 1, 1);
  row++;

  videoGridLayout->addWidget(new QLabel(tr("Width:")), row, 0, 1, 1);
  widthSpinbox = new QSpinBox(videoGroupbox);
  widthSpinbox->setMaximum(16777216);
  videoGridLayout->addWidget(widthSpinbox, row, 1, 1, 1);
  row++;

  videoGridLayout->addWidget(new QLabel(tr("Height:")), row, 0, 1, 1);
  heightSpinbox = new QSpinBox(videoGroupbox);
  heightSpinbox->setMaximum(16777216);
  videoGridLayout->addWidget(heightSpinbox, row, 1, 1, 1);
  row++;

  videoGridLayout->addWidget(new QLabel(tr("Frame Rate:")), row, 0, 1, 1);
  framerate_box_ = new QComboBox(videoGroupbox);
  framerate_box_->addItems(FRAME_RATES);
  videoGridLayout->addWidget(framerate_box_, row, 1, 1, 1);
  row++;

  videoGridLayout->addWidget(new QLabel(tr("Compression Type:")), row, 0, 1, 1);
  compressionTypeCombobox = new QComboBox(videoGroupbox);
  videoGridLayout->addWidget(compressionTypeCombobox, row, 1, 1, 1);
  row++;

  videoBitrateLabel = new QLabel(videoGroupbox);
  videoGridLayout->addWidget(videoBitrateLabel, row, 0, 1, 1);
  videobitrateSpinbox = new QDoubleSpinBox(videoGroupbox);
  videobitrateSpinbox->setMaximum(100);
  videobitrateSpinbox->setValue(2);
  videoGridLayout->addWidget(videobitrateSpinbox, row, 1, 1, 1);
  row++;


  videoGridLayout->addWidget(new QLabel(tr("GOP Length:")), row, 0, 1, 1);
  gop_length_box_ = new QSpinBox(videoGroupbox);
  gop_length_box_->setMinimum(1);
  gop_length_box_->setMaximum(std::numeric_limits<int>::max());
  videoGridLayout->addWidget(gop_length_box_, row, 1, 1, 1);
  row++;

  videoGridLayout->addWidget(new QLabel(tr("Closed GOP:")), row, 0, 1, 1);
  closed_gop_box_ = new QCheckBox(videoGroupbox);
  closed_gop_box_->setCheckable(true);
  closed_gop_box_->setChecked(true);
  videoGridLayout->addWidget(closed_gop_box_, row, 1, 1, 1);
  row++;

  videoGridLayout->addWidget(new QLabel(tr("B-Frame count:")), row, 0, 1, 1);
  b_frame_box_ = new QSpinBox(videoGroupbox);
  b_frame_box_->setMinimum(0);
  videoGridLayout->addWidget(b_frame_box_, row, 1, 1, 1);
  row++;

  verticalLayout->addWidget(videoGroupbox);

  audioGroupbox = new QGroupBox(this);
  audioGroupbox->setTitle("Audio");
  audioGroupbox->setCheckable(true);

  auto audioGridLayout = new QGridLayout(audioGroupbox);
  row = 0;

  audioGridLayout->addWidget(new QLabel(tr("Codec:")), row, 0, 1, 1);
  acodecCombobox = new QComboBox(audioGroupbox);
  audioGridLayout->addWidget(acodecCombobox, row, 1, 1, 1);
  row++;

  audioGridLayout->addWidget(new QLabel(tr("Sampling Rate:")), row, 0, 1, 1);
  samplingRateSpinbox = new QSpinBox(audioGroupbox);
  samplingRateSpinbox->setMaximum(96000);
  samplingRateSpinbox->setValue(0);
  audioGridLayout->addWidget(samplingRateSpinbox, row, 1, 1, 1);
  row++;

  audioGridLayout->addWidget(new QLabel(tr("Bitrate (Kbps/CBR):")), row, 0, 1, 1);
  audiobitrateSpinbox = new QSpinBox(audioGroupbox);
  audiobitrateSpinbox->setMaximum(320);
  audiobitrateSpinbox->setValue(256);
  audioGridLayout->addWidget(audiobitrateSpinbox, row, 1, 1, 1);

  verticalLayout->addWidget(audioGroupbox);

  auto progressLayout = new QHBoxLayout();
  progressBar = new QProgressBar(this);
  progressBar->setFormat("%p% (ETA: 0:00:00)");
  progressBar->setEnabled(false);
  progressBar->setValue(0);
  progressLayout->addWidget(progressBar);

  renderCancel = new QPushButton(this);
  renderCancel->setText("x");
  renderCancel->setEnabled(false);
  renderCancel->setMaximumSize(QSize(20, 16777215));
  connect(renderCancel, SIGNAL(clicked(bool)), this, SLOT(cancel_render()));
  progressLayout->addWidget(renderCancel);

  verticalLayout->addLayout(progressLayout);

  auto buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch();

  export_button = new QPushButton(this);
  export_button->setText("Export");
  connect(export_button, SIGNAL(clicked(bool)), this, SLOT(export_action()));

  buttonLayout->addWidget(export_button);

  cancel_button = new QPushButton(this);
  cancel_button->setText("Cancel");
  connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(reject()));

  buttonLayout->addWidget(cancel_button);

  buttonLayout->addStretch();

  verticalLayout->addLayout(buttonLayout);

  connect(formatCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(format_changed(int)));
  connect(compressionTypeCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(comp_type_changed(int)));
  connect(vcodecCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(vcodec_changed(int)));
  connect(profile_box_, SIGNAL(currentIndexChanged(int)), this, SLOT(profile_changed(int)));
  connect(level_box_, SIGNAL(currentIndexChanged(int)), this, SLOT(level_changed(int)));
}


void ExportDialog::setupForMpeg2()
{
  assert(profile_box_);
  assert(level_box_);

  const int profile_default = 1;
  const int level_default = 1;

  format_vcodecs.append(AV_CODEC_ID_MPEG2VIDEO);

  format_acodecs.append(AV_CODEC_ID_AC3);
  format_acodecs.append(AV_CODEC_ID_MP2);
  format_acodecs.append(AV_CODEC_ID_MP3);
  format_acodecs.append(AV_CODEC_ID_PCM_S16LE);

  profile_box_->clear();
  profile_box_->addItems(MPEG2_PROFILES);
  profile_box_->setCurrentIndex(profile_default);

  level_box_->clear();
  level_box_->addItems(MPEG2_LEVELS);
  level_box_->setCurrentIndex(level_default);


  profile_box_->setEnabled(true);
  level_box_->setEnabled(true);
}


void ExportDialog::constrainMpeg2()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(b_frame_box_);
  Q_ASSERT(sequence_);

  int max_width = 0;
  int max_height = 0;
  int max_bitrate = 0;
  int max_b_frames = 0;
  QStringList rates = FRAME_RATES;


  const auto profile = profile_box_->currentText();
  if (profile == MPEG2_SIMPLE_PROFILE) {
    max_b_frames = 0;
  } else {
    max_b_frames = std::numeric_limits<int>::max();
  }

  const auto level = level_box_->currentText();
  if (level == MPEG2_LOW_LEVEL) {
    max_height = MPEG2_LL_CONSTRAINTS.max_height;
    max_width =  MPEG2_LL_CONSTRAINTS.max_width;
    max_bitrate = MPEG2_LL_CONSTRAINTS.max_bitrate;
    rates = MPEG2_LOW_LEVEL_FRATES;
  } else if (level == MPEG2_MAIN_LEVEL) {
    max_height = MPEG2_ML_CONSTRAINTS.max_height;
    max_width = MPEG2_ML_CONSTRAINTS.max_width;
    max_bitrate = MPEG2_ML_CONSTRAINTS.max_bitrate;
    rates = MPEG2_MAIN_LEVEL_FRATES;
  } else if (level == MPEG2_HIGH1440_LEVEL) {
    max_height = MPEG2_H14_CONSTRAINTS.max_height;
    max_width = MPEG2_H14_CONSTRAINTS.max_width;
    max_bitrate = MPEG2_H14_CONSTRAINTS.max_bitrate;
    rates = MPEG2_HIGH1440_LEVEL_FRATES;
  } else if (level == MPEG2_HIGH_LEVEL) {
    max_height = MPEG2_HL_CONSTRAINTS.max_height;
    max_width = MPEG2_HL_CONSTRAINTS.max_width;
    max_bitrate = MPEG2_HL_CONSTRAINTS.max_bitrate;
    rates = MPEG2_HIGH_LEVEL_FRATES;
  } else {
    qCritical() << "Unknown MPEG2 level =" << level;
    return;
  }

  heightSpinbox->setMaximum(max_height);
  widthSpinbox->setMaximum(max_width);
  videobitrateSpinbox->setMaximum(max_bitrate);
  b_frame_box_->setMaximum(max_b_frames);
  framerate_box_->clear();
  framerate_box_->addItems(rates);


  heightSpinbox->setValue(sequence_->height());
  widthSpinbox->setValue(sequence_->width());
  const int ix = framerate_box_->findText(QString::number(sequence_->frameRate(), 'g', 4));
  framerate_box_->setCurrentIndex(ix);
}

void ExportDialog::setupForMpeg4()
{
  assert(profile_box_);
  assert(level_box_);

  format_vcodecs.append(AV_CODEC_ID_MPEG4);
  format_vcodecs.append(AV_CODEC_ID_H264);

  format_acodecs.append(AV_CODEC_ID_AAC);
  format_acodecs.append(AV_CODEC_ID_AC3);
  format_acodecs.append(AV_CODEC_ID_MP2);
  format_acodecs.append(AV_CODEC_ID_MP3);
}

void ExportDialog::setupForH264()
{
  Q_ASSERT(profile_box_);
  const int profile_default = 2;
  const auto level_default = "4.0";
  profile_box_->clear();
  profile_box_->addItems(H264_PROFILES);
  profile_box_->setCurrentIndex(profile_default);

  level_box_->clear();

  for (auto lvl = MIN_H264_LEVEL; lvl < MAX_H264_LEVEL; ++lvl) {
    for (auto sub = 0; sub <= 2; ++sub) {
      auto level = QString("%1.%2").arg(QString::number(lvl)).arg(QString::number(sub));
      level_box_->addItem(level);
    }
  }

  auto ix = level_box_->findText(level_default);
  level_box_->setCurrentIndex(ix);
  profile_box_->setEnabled(true);
  level_box_->setEnabled(true);
}


void ExportDialog::constrainH264()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(sequence_);

  int max_width = 0;
  int max_height = 0;
  int max_bitrate = 0;


  const auto profile = profile_box_->currentText();
  //TODO:

  const auto level = level_box_->currentText();
  //TODO: using CRF
  //  if (level == "2.0") {
  //    max_bitrate = H264_20_CONSTRAINTS.max_bitrate;
  //  } else if (level == "2.1") {
  //    max_bitrate = H264_21_CONSTRAINTS.max_bitrate;
  //  } else if (level == "2.2") {
  //    max_bitrate = H264_22_CONSTRAINTS.max_bitrate;
  //  } else if (level == "3.0") {
  //    max_bitrate = H264_30_CONSTRAINTS.max_bitrate;
  //  } else if (level == "3.1") {
  //    max_bitrate = H264_31_CONSTRAINTS.max_bitrate;
  //  } else if (level == "3.2") {
  //    max_bitrate = H264_32_CONSTRAINTS.max_bitrate;
  //  } else if (level == "4.0") {
  //    max_bitrate = H264_40_CONSTRAINTS.max_bitrate;
  //  } else if (level == "4.1") {
  //    max_bitrate = H264_41_CONSTRAINTS.max_bitrate;
  //  } else if (level == "4.2") {
  //    max_bitrate = H264_42_CONSTRAINTS.max_bitrate;
  //  } else if (level == "5.0") {
  //    max_bitrate = H264_50_CONSTRAINTS.max_bitrate;
  //  } else if (level == "5.1") {
  //    max_bitrate = H264_51_CONSTRAINTS.max_bitrate;
  //  } else if (level == "5.2") {
  //    max_bitrate = H264_52_CONSTRAINTS.max_bitrate;
  //  }

  //  videobitrateSpinbox->setMaximum(max_bitrate);
}


void ExportDialog::unconstrain()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(b_frame_box_);



  constexpr auto max_val = std::numeric_limits<int>::max();
  heightSpinbox->setMaximum(max_val);
  widthSpinbox->setMaximum(max_val);
  videobitrateSpinbox->setMaximum(max_val);
  b_frame_box_->setMaximum(max_val);

}


std::string ExportDialog::getCodecLongName(const AVCodecID codec) const
{
  auto codec_info = avcodec_find_encoder(codec);
  if (codec_info == nullptr) {
    return "";
  }

  return codec_info->long_name;
}
