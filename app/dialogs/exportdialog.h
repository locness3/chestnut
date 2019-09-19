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

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>

#include "io/exportthread.h"
#include "project/sequence.h"

struct FormatCodecs
{
  QVector<int> video_;
  QVector<int> audio_;
};


class ExportDialog : public QDialog
{
    Q_OBJECT
  public:
    ExportDialog(SequencePtr seq, QWidget *parent = nullptr);
    virtual ~ExportDialog();

    ExportDialog(const ExportDialog& ) = delete;
    ExportDialog& operator=(const ExportDialog&) = delete;
    ExportDialog(const ExportDialog&& ) = delete;
    ExportDialog& operator=(const ExportDialog&&) = delete;

    QString export_error;

  private slots:
    void format_changed(int index);
    void export_action();
    void update_progress_bar(int value, qint64 remaining_ms);
    void cancel_render();
    void render_thread_finished();
    void vcodec_changed(int index);
    void acodecChanged(int index);
    void comp_type_changed(int index);
    void profile_changed(int index);
    void level_changed(int index);

  private:
    SequencePtr sequence_ {nullptr};
    QString output_dir_;

    FormatCodecs format_codecs_;
    QVector<QString> format_strings;
    QVector<QString> format_profiles;
    QVector<QString> format_levels;

    ExportThread* et;
    bool cancelled;

    QComboBox* rangeCombobox;
    QSpinBox* widthSpinbox;
    QDoubleSpinBox* videobitrateSpinbox;
    QLabel* videoBitrateLabel;
    QComboBox* framerate_box_ {nullptr};
    QComboBox* vcodecCombobox;
    QComboBox* acodecCombobox;
    QSpinBox* samplingRateSpinbox;
    QLabel* audio_bitrate_label_ {nullptr};
    QSpinBox* audiobitrateSpinbox;
    QProgressBar* progressBar;
    QComboBox* formatCombobox;
    QSpinBox* heightSpinbox;
    QPushButton* export_button;
    QPushButton* cancel_button;
    QPushButton* renderCancel;
    QGroupBox* videoGroupbox;
    QGroupBox* audioGroupbox;
    QLabel* compression_type_label_ {nullptr};
    QComboBox* compressionTypeCombobox {nullptr};
    QLabel* gop_length_label_ {nullptr};
    QSpinBox* gop_length_box_ {nullptr};
    QLabel* closed_gop_label_ {nullptr};
    QCheckBox* closed_gop_box_ {nullptr};
    QLabel* b_frame_label_ {nullptr};
    QSpinBox* b_frame_box_ {nullptr};
    QLabel* profile_box_label_ {nullptr};
    QComboBox* profile_box_ {nullptr};
    QLabel* level_box_label_ {nullptr};
    QComboBox* level_box_ {nullptr};
    bool subsampling_ {true};


    void setup_ui();
    void prep_ui_for_render(bool r);

    void setupForMpeg2Video();
    void constrainMpeg2Video();

    void setupForMpeg4();
    void constrainMPEG4();

    void setupForH264();
    void constrainH264();

    void setupForDNXHD();
    void constrainDNXHD();

    void setupForHuffYUV();

    void unconstrain();

    void setGOPWidgetsVisible(const bool visible);

    std::string getCodecLongName(const AVCodecID codec) const;

    void matchWidgetsToSequence();
};

#endif // EXPORTDIALOG_H
