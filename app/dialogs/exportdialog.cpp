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

constexpr auto MAX_WIDTH = 3840;
constexpr auto MAX_HEIGHT = 2160;


enum ExportFormats
{
  FORMAT_DNXHD,
  FORMAT_AC3,
  FORMAT_MP3,
  FORMAT_MPEG2,
  FORMAT_MPEG4,
  FORMAT_MKV,
  FORMAT_MOV,
  FORMAT_WAV,
  FORMAT_SIZE
};


static const FormatCodecs DNXHD_CODECS {{AV_CODEC_ID_DNXHD}, {AV_CODEC_ID_PCM_S16LE}};
static const FormatCodecs MPEG2_CODECS {{AV_CODEC_ID_MPEG2VIDEO},
                                        {AV_CODEC_ID_AC3,AV_CODEC_ID_MP2,AV_CODEC_ID_MP3,AV_CODEC_ID_PCM_S16LE}};
static const FormatCodecs MPEG4_CODECS {{AV_CODEC_ID_MPEG4, AV_CODEC_ID_H264},
                                        {AV_CODEC_ID_AAC, AV_CODEC_ID_AC3, AV_CODEC_ID_MP2, AV_CODEC_ID_MP3}};
static const FormatCodecs AC3_CODECS {{}, {AV_CODEC_ID_AC3, AV_CODEC_ID_EAC3}};
static const FormatCodecs MP3_CODECS {{}, {AV_CODEC_ID_MP3}};
static const FormatCodecs MKV_CODECS {{AV_CODEC_ID_MPEG4, AV_CODEC_ID_H264, AV_CODEC_ID_HUFFYUV},
                                      {AV_CODEC_ID_AAC, AV_CODEC_ID_AC3, AV_CODEC_ID_EAC3, AV_CODEC_ID_FLAC, AV_CODEC_ID_MP2,
                                      AV_CODEC_ID_MP3, AV_CODEC_ID_OPUS, AV_CODEC_ID_PCM_S16LE, AV_CODEC_ID_VORBIS, AV_CODEC_ID_WAVPACK}};

static const FormatCodecs MOV_CODECS {{AV_CODEC_ID_MPEG4, AV_CODEC_ID_H264, AV_CODEC_ID_DNXHD},
                                      {AV_CODEC_ID_AAC, AV_CODEC_ID_AC3, AV_CODEC_ID_MP2, AV_CODEC_ID_MP3, AV_CODEC_ID_PCM_S16LE}};

static const FormatCodecs WAV_CODECS {{}, {AV_CODEC_ID_PCM_S16LE}};


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
  format_strings[FORMAT_DNXHD] = "DNxHD";
  format_strings[FORMAT_AC3] = "Dolby Digital (AC3)";
  format_strings[FORMAT_MP3] = "MP3 Audio";
  format_strings[FORMAT_MPEG2] = "MPEG-2 Video";
  format_strings[FORMAT_MPEG4] = "MPEG-4 Video";
  format_strings[FORMAT_MKV] = "Matroska MKV";
  format_strings[FORMAT_MOV] = "QuickTime MOV";
  format_strings[FORMAT_WAV] = "WAVE Audio";

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
  Q_ASSERT(vcodecCombobox);
  Q_ASSERT(acodecCombobox);
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);

  vcodecCombobox->clear();
  acodecCombobox->clear();

  switch (index) {
    case FORMAT_DNXHD:
      format_codecs_ = DNXHD_CODECS;
      break;
    case FORMAT_AC3:
      format_codecs_ = AC3_CODECS;
      break;
    case FORMAT_MP3:
      format_codecs_ = MP3_CODECS;
      break;
    case FORMAT_MPEG2:
      format_codecs_ = MPEG2_CODECS;
      break;
    case FORMAT_MPEG4:
      format_codecs_ = MPEG4_CODECS;
      break;
    case FORMAT_MKV:
      format_codecs_ = MKV_CODECS;
      break;
    case FORMAT_MOV:
      format_codecs_ = MOV_CODECS;
      break;
    case FORMAT_WAV:
      format_codecs_ = WAV_CODECS;
      break;
    default:
      qCritical() << "Invalid format selection - this is a bug, please inform the developers";
  }

  AVCodec* codec_info;
  for (const auto& fmt: format_codecs_.video_) {
    codec_info = avcodec_find_encoder(static_cast<AVCodecID>(fmt));
    if (codec_info == nullptr) {
      vcodecCombobox->addItem("nullptr");
    } else {
      vcodecCombobox->addItem(codec_info->long_name);
    }
  }
  for (auto& acodec : format_codecs_.audio_) {
    codec_info = avcodec_find_encoder(static_cast<AVCodecID>(acodec));
    if (codec_info == nullptr) {
      acodecCombobox->addItem("nullptr");
    } else {
      acodecCombobox->addItem(codec_info->long_name);
    }
  }


  const auto video_enabled = !format_codecs_.video_.empty();
  const auto audio_enabled = !format_codecs_.audio_.empty();
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
          tr("Export width and height must both be divisible by 2."),
          QMessageBox::Ok
          );
    return;
  }

  QString ext;
  switch (formatCombobox->currentIndex()) {
    case FORMAT_DNXHD:
      ext = "mxf";
      break;
    case FORMAT_AC3:
      ext = "ac3";
      break;
    case FORMAT_MP3:
      ext = "mp3";
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
    case FORMAT_MKV:
      if (!videoGroupbox->isChecked()) {
        ext = "mka";
      } else {
        ext = "mkv";
      }
      break;
    case FORMAT_MOV:
      ext = "mov";
      break;
    case FORMAT_WAV:
      ext = "wav";
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
      et->video_params.codec = format_codecs_.video_.at(vcodecCombobox->currentIndex());
      et->video_params.width = widthSpinbox->value();
      et->video_params.height = heightSpinbox->value();
      bool ok;
      et->video_params.frame_rate = framerate_box_->currentText().toDouble(&ok);
      Q_ASSERT(ok);
      et->video_params.compression_type = static_cast<CompressionType>(compressionTypeCombobox->currentData().toInt());
      et->video_params.bitrate = videobitrateSpinbox->value();
      et->video_params.gop_length_ = gop_length_box_->value();
      et->video_params.closed_gop_ = closed_gop_box_->isChecked();
      et->video_params.b_frames_ = b_frame_box_->value();
      et->video_params.profile_ = profile_box_->currentText();
      et->video_params.level_ = level_box_->currentText();
      et->video_params.subsampling = subsampling_;
    }
    et->audio_params.enabled = audioGroupbox->isChecked();
    if (et->audio_params.enabled) {
      et->audio_params.codec = format_codecs_.audio_.at(acodecCombobox->currentIndex());
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

void ExportDialog::cancel_render()
{
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
  Q_ASSERT(videoBitrateLabel);
  Q_ASSERT(videobitrateSpinbox);
  Q_ASSERT(compression_type_label_);
  Q_ASSERT(compressionTypeCombobox);

  if (index < 0) {
    return;
  }

  videoBitrateLabel->setVisible(true);
  videobitrateSpinbox->setVisible(true);
  compression_type_label_->setVisible(true);
  compressionTypeCombobox->setVisible(true);
  compressionTypeCombobox->clear();

  if (!format_codecs_.video_.empty() && (format_codecs_.video_.at(index) == AV_CODEC_ID_H264)) {
    compressionTypeCombobox->setEnabled(true);
    compressionTypeCombobox->addItem(tr("Quality-based (Constant Rate Factor)"), static_cast<int>(CompressionType::CRF));
  } else {
    compressionTypeCombobox->addItem(tr("Constant Bitrate"), static_cast<int>(CompressionType::CBR));
    compressionTypeCombobox->setCurrentIndex(0);
    compressionTypeCombobox->setEnabled(false);
  }

  switch (format_codecs_.video_.at(index)) {
    case AV_CODEC_ID_H264:
      setupForH264();
      break;
    case AV_CODEC_ID_MPEG2VIDEO:
      setupForMpeg2Video();
      break;
    case AV_CODEC_ID_DNXHD:
      setupForDNXHD();
      break;
    case AV_CODEC_ID_MPEG4:
      setupForMpeg4();
      break;
    case AV_CODEC_ID_HUFFYUV:
      setupForHuffYUV();
      break;
  }
}


void ExportDialog::acodecChanged(int index)
{
  if (index < 0) {
    return;
  }

  Q_ASSERT(audio_bitrate_label_);
  Q_ASSERT(audiobitrateSpinbox);

  const auto codecs = format_codecs_.audio_;
  if (!codecs.empty() &&
      ( (codecs.at(index) == AV_CODEC_ID_WAVPACK)
        || (codecs.at(index) == AV_CODEC_ID_FLAC)) ) {
    audio_bitrate_label_->setVisible(false);
    audiobitrateSpinbox->setVisible(false);
  } else {
    audio_bitrate_label_->setVisible(true);
    audiobitrateSpinbox->setVisible(true);
  }
}

void ExportDialog::comp_type_changed(int)
{
  Q_ASSERT(compressionTypeCombobox);
  Q_ASSERT(videobitrateSpinbox);
  Q_ASSERT(videoBitrateLabel);
  Q_ASSERT(formatCombobox);


  auto locked_br = formatCombobox->currentText() == format_strings[FORMAT_DNXHD];

  videobitrateSpinbox->setToolTip("");

  if (!locked_br) {
    // Already constrained
    videobitrateSpinbox->setMinimum(0);
    videobitrateSpinbox->setMaximum(99.99);
  }
  const auto compressionType = static_cast<CompressionType>(compressionTypeCombobox->currentData().toInt());
  switch (compressionType) {
    case CompressionType::CBR:
    case CompressionType::TARGETBITRATE:
      videoBitrateLabel->setText(tr("Bitrate (Mbps):"));
      if (!locked_br) {
        videobitrateSpinbox->setValue(qMax(0.5, calculateBitrate(sequence_->height())));
      }
      break;
    case CompressionType::CRF:
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

  const auto ix = vcodecCombobox->currentIndex();
  switch (format_codecs_.video_.at(ix))  {
    case AV_CODEC_ID_MPEG2VIDEO:
      constrainMpeg2Video();
      break;
    case AV_CODEC_ID_H264:
      constrainH264();
      break;
    case AV_CODEC_ID_DNXHD:
      constrainDNXHD();
      break;
    case AV_CODEC_ID_MPEG4:
      constrainMPEG4();
      break;
    default:
      unconstrain();
  }
}

void ExportDialog::level_changed(int)
{
  profile_changed(-1);
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

  profile_box_label_ = new QLabel(tr("Profile"));
  videoGridLayout->addWidget(profile_box_label_, row, 0, 1, 1);
  profile_box_ = new QComboBox(videoGroupbox);
  videoGridLayout->addWidget(profile_box_, row, 1, 1, 1);
  row++;

  level_box_label_ = new QLabel(tr("Level"));
  videoGridLayout->addWidget(level_box_label_, row, 0, 1, 1);
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

  compression_type_label_ = new QLabel(tr("Compression Type:"));
  videoGridLayout->addWidget(compression_type_label_, row, 0, 1, 1);
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

  gop_length_label_ = new QLabel(tr("GOP Length:"));
  videoGridLayout->addWidget(gop_length_label_, row, 0, 1, 1);
  gop_length_box_ = new QSpinBox(videoGroupbox);
  gop_length_box_->setMinimum(1);
  gop_length_box_->setMaximum(std::numeric_limits<int>::max());
  videoGridLayout->addWidget(gop_length_box_, row, 1, 1, 1);
  row++;

  closed_gop_label_ = new QLabel(tr("Closed GOP:"));
  videoGridLayout->addWidget(closed_gop_label_, row, 0, 1, 1);
  closed_gop_box_ = new QCheckBox(videoGroupbox);
  closed_gop_box_->setCheckable(true);
  closed_gop_box_->setChecked(true);
  videoGridLayout->addWidget(closed_gop_box_, row, 1, 1, 1);
  row++;

  b_frame_label_ = new QLabel(tr("B-Frame count:"));
  videoGridLayout->addWidget(b_frame_label_, row, 0, 1, 1);
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

  audio_bitrate_label_ = new QLabel(tr("Bitrate (Kbps/CBR):"));
  audioGridLayout->addWidget(audio_bitrate_label_, row, 0, 1, 1);
  audiobitrateSpinbox = new QSpinBox(audioGroupbox);
  audiobitrateSpinbox->setMaximum(320);
  audiobitrateSpinbox->setValue(256);
  audioGridLayout->addWidget(audiobitrateSpinbox, row, 1, 1, 1);
  row++;



  verticalLayout->addWidget(audioGroupbox);

  auto spacer = new QSpacerItem(0,0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
  verticalLayout->addItem(spacer);

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
  connect(acodecCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(acodecChanged(int)));
  connect(profile_box_, SIGNAL(currentIndexChanged(int)), this, SLOT(profile_changed(int)));
  connect(level_box_, SIGNAL(currentIndexChanged(int)), this, SLOT(level_changed(int)));
}


void ExportDialog::setupForMpeg2Video()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);

  const int profile_default = 1;
  const int level_default = 1;

  profile_box_->clear();
  profile_box_->addItems(MPEG2_PROFILES);
  profile_box_->setCurrentIndex(profile_default);

  level_box_->clear();
  level_box_->addItems(MPEG2_LEVELS);
  level_box_->setCurrentIndex(level_default);

  profile_box_label_->setVisible(true);
  profile_box_->setVisible(true);
  level_box_label_->setVisible(true);
  level_box_->setVisible(true);

  setGOPWidgetsVisible(true);
}


void ExportDialog::constrainMpeg2Video()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(b_frame_box_);
  Q_ASSERT(sequence_);

  int max_b_frames = 0;

  const auto profile = profile_box_->currentText();
  if (profile == MPEG2_SIMPLE_PROFILE) {
    max_b_frames = 0;
  } else {
    max_b_frames = std::numeric_limits<int>::max();
  }

  LevelConstraints constraint;
  const auto level = level_box_->currentText();
  if (level == MPEG2_LOW_LEVEL) {
    constraint = MPEG2_LL_CONSTRAINTS;
  } else if (level == MPEG2_MAIN_LEVEL) {
    constraint = MPEG2_ML_CONSTRAINTS;
  } else if (level == MPEG2_HIGH1440_LEVEL) {
    constraint = MPEG2_H14_CONSTRAINTS;
  } else if (level == MPEG2_HIGH_LEVEL) {
    constraint = MPEG2_HL_CONSTRAINTS;
  } else {
    qCritical() << "Unknown MPEG2 level =" << level;
    return;
  }

  heightSpinbox->setMaximum(constraint.max_height);
  widthSpinbox->setMaximum(constraint.max_width);
  videobitrateSpinbox->setMaximum(constraint.max_bitrate);
  b_frame_box_->setMaximum(max_b_frames);
  framerate_box_->clear();
  framerate_box_->addItems(constraint.framerates);

  matchWidgetsToSequence();
}

void ExportDialog::setupForMpeg4()
{
  Q_ASSERT(profile_box_label_);
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_label_);
  Q_ASSERT(level_box_);

  profile_box_->clear();
  profile_box_->addItems(MPEG4_PROFILES);

  level_box_->clear();
  level_box_->addItems(MPEG4_LEVELS);

  profile_box_->setEnabled(true);
  level_box_->setEnabled(true);
  setGOPWidgetsVisible(true);

  profile_box_label_->setVisible(true);
  profile_box_->setVisible(true);
  level_box_label_->setVisible(true);
  level_box_->setVisible(true);
}


void ExportDialog::constrainMPEG4()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(sequence_);

  LevelConstraints constraint;
  const auto level = level_box_->currentText();
  if (level == MPEG4_SSTP_1_LEVEL) {
    constraint = MPEG4_SSTP_1_CONSTRAINTS;
  } else if (level == MPEG4_SSTP_2_LEVEL) {
    constraint = MPEG4_SSTP_2_CONSTRAINTS;
  } else if (level == MPEG4_SSTP_3_LEVEL) {
    constraint = MPEG4_SSTP_3_CONSTRAINTS;
  } else if (level == MPEG4_SSTP_4_LEVEL) {
    constraint = MPEG4_SSTP_4_CONSTRAINTS;
  } else if (level == MPEG4_SSTP_5_LEVEL) {
    constraint = MPEG4_SSTP_5_CONSTRAINTS;
  } else if (level == MPEG4_SSTP_6_LEVEL) {
    constraint = MPEG4_SSTP_6_CONSTRAINTS;
  }

  widthSpinbox->setMaximum(constraint.max_width);
  heightSpinbox->setMaximum(constraint.max_height);
  if (constraint.max_bitrate > 0) {
    const auto mb_brate = constraint.max_bitrate / 1E6;
    videobitrateSpinbox->setMaximum(mb_brate);
  }
  framerate_box_->clear();
  framerate_box_->addItems(constraint.framerates);

  matchWidgetsToSequence();
}

void ExportDialog::setupForH264()
{
  Q_ASSERT(profile_box_label_);
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_label_);
  Q_ASSERT(level_box_);

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
  setGOPWidgetsVisible(true);

  profile_box_label_->setVisible(true);
  profile_box_->setVisible(true);
  level_box_label_->setVisible(true);
  level_box_->setVisible(true);
}


void ExportDialog::constrainH264()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(widthSpinbox);
  Q_ASSERT(sequence_);

  widthSpinbox->setMaximum(MAX_WIDTH);
  heightSpinbox->setMaximum(MAX_HEIGHT);

  framerate_box_->clear();
  framerate_box_->addItems(ALL_FRATES);

  matchWidgetsToSequence();

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


void ExportDialog::setupForDNXHD()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(gop_length_box_);
  Q_ASSERT(b_frame_box_);
  Q_ASSERT(closed_gop_box_);
  Q_ASSERT(level_box_label_);
  Q_ASSERT(level_box_);

  profile_box_->clear();
  profile_box_->addItems(DNXHD_PROFILES);
  profile_box_->setEnabled(true);

  profile_box_label_->setVisible(true);
  profile_box_->setVisible(true);
  level_box_label_->setVisible(false);
  level_box_->setVisible(false);

  setGOPWidgetsVisible(false);
}

void ExportDialog::constrainDNXHD()
{
  Q_ASSERT(profile_box_);
  Q_ASSERT(widthSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(videobitrateSpinbox);

  auto profile = profile_box_->currentText();
  LevelConstraints constraint;
  if (profile == "440x") {
    constraint = DNXHD_440X_CONSTRAINTS;
  } else if (profile == "440") {
    constraint = DNXHD_440_CONSTRAINTS;
  } else if (profile == "365x") {
    constraint = DNXHD_365X_CONSTRAINTS;
  } else if (profile == "365") {
    constraint = DNXHD_365_CONSTRAINTS;
  } else if (profile == "350x") {
    constraint = DNXHD_350X_CONSTRAINTS;
  } else if (profile == "350") {
    constraint = DNXHD_350_CONSTRAINTS;
  } else if (profile == "220x") {
    constraint = DNXHD_220X_CONSTRAINTS;
  } else if (profile == "220") {
    constraint = DNXHD_220_CONSTRAINTS;
  } else if (profile == "185x") {
    constraint = DNXHD_185X_CONSTRAINTS;
  } else if (profile == "185") {
    constraint = DNXHD_185_CONSTRAINTS;
  } else if (profile == "145") {
    constraint = DNXHD_145_CONSTRAINTS;
  } else if (profile == "100") {
    constraint = DNXHD_100_CONSTRAINTS;
  } else if (profile == "90") {
    constraint = DNXHD_90_CONSTRAINTS;
  } else {
    return;
  }

  widthSpinbox->setMaximum(constraint.max_width);
  widthSpinbox->setMinimum(constraint.max_width);
  heightSpinbox->setMaximum(constraint.max_height);
  heightSpinbox->setMinimum(constraint.max_height);
  const auto mb_brate = constraint.max_bitrate / 1E6;
  videobitrateSpinbox->setMaximum(mb_brate);
  videobitrateSpinbox->setMinimum(mb_brate);
  framerate_box_->clear();
  framerate_box_->addItems(constraint.framerates);
  subsampling_ = constraint.subsampling;
}


void ExportDialog::setupForHuffYUV()
{
  Q_ASSERT(profile_box_label_);
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_label_);
  Q_ASSERT(level_box_);
  Q_ASSERT(videoBitrateLabel);
  Q_ASSERT(videobitrateSpinbox);
  Q_ASSERT(compression_type_label_);
  Q_ASSERT(compressionTypeCombobox);
  Q_ASSERT(widthSpinbox);
  Q_ASSERT(heightSpinbox);

  const bool visibility = false;
  setGOPWidgetsVisible(visibility);
  profile_box_label_->setVisible(visibility);
  profile_box_->setVisible(visibility);
  level_box_label_->setVisible(visibility);
  level_box_->setVisible(visibility);
  videoBitrateLabel->setVisible(visibility);
  videobitrateSpinbox->setVisible(visibility);
  compression_type_label_->setVisible(visibility);
  compressionTypeCombobox->setVisible(visibility);

  widthSpinbox->setMaximum(MAX_WIDTH);
  heightSpinbox->setMaximum(MAX_HEIGHT);

  matchWidgetsToSequence();
}

void ExportDialog::unconstrain()
{
  Q_ASSERT(profile_box_label_);
  Q_ASSERT(profile_box_);
  Q_ASSERT(level_box_label_);
  Q_ASSERT(level_box_);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(b_frame_box_);
  Q_ASSERT(framerate_box_);
  Q_ASSERT(sequence_);

  constexpr auto max_val = std::numeric_limits<int>::max();
  widthSpinbox->setMaximum(MAX_WIDTH);
  heightSpinbox->setMaximum(MAX_HEIGHT);
  videobitrateSpinbox->setMaximum(max_val);
  b_frame_box_->setMaximum(max_val);

  framerate_box_->clear();
  framerate_box_->addItems(ALL_FRATES);

  profile_box_label_->setVisible(false);
  profile_box_->setVisible(false);
  level_box_label_->setVisible(false);
  level_box_->setVisible(false);

  setGOPWidgetsVisible(true);

  matchWidgetsToSequence();
}


void ExportDialog::setGOPWidgetsVisible(const bool visible)
{
  Q_ASSERT(gop_length_label_);
  Q_ASSERT(gop_length_box_);
  Q_ASSERT(b_frame_label_);
  Q_ASSERT(b_frame_box_);
  Q_ASSERT(closed_gop_label_);
  Q_ASSERT(closed_gop_box_);

  gop_length_label_->setVisible(visible);
  gop_length_box_->setVisible(visible);
  b_frame_label_->setVisible(visible);
  b_frame_box_->setVisible(visible);
  closed_gop_label_->setVisible(visible);
  closed_gop_box_->setVisible(visible);
}


std::string ExportDialog::getCodecLongName(const AVCodecID codec) const
{
  auto codec_info = avcodec_find_encoder(codec);
  if (codec_info == nullptr) {
    return "";
  }

  return codec_info->long_name;
}


void ExportDialog::matchWidgetsToSequence()
{
  Q_ASSERT(sequence_);
  Q_ASSERT(widthSpinbox);
  Q_ASSERT(heightSpinbox);
  Q_ASSERT(framerate_box_);


  auto dar = static_cast<double>(sequence_->width()) / sequence_->height();
  widthSpinbox->setValue(sequence_->width());
  auto height = qRound(widthSpinbox->value() / dar);
  if (height % 2) {
    height++;
  }

  heightSpinbox->setValue(height) ;
  auto frate = QString::number(sequence_->frameRate(), 'g', 4);
  auto ix = framerate_box_->findText(frate);
  if (ix >= 0) {
    framerate_box_->setCurrentIndex(ix);
  }

  const auto compressionType = static_cast<CompressionType>(compressionTypeCombobox->currentData().toInt());
  if (compressionType == CompressionType::CBR) {
    videobitrateSpinbox->setValue(calculateBitrate(height));
  }
}


double ExportDialog::calculateBitrate(const int height)
{
  // No idea where these values have originally come from
  return qRound((0.01528 * height) - 4.5);
}
