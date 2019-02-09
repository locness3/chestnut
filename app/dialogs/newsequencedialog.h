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
#ifndef NEWSEQUENCEDIALOG_H
#define NEWSEQUENCEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>

#include "project/sequence.h"
#include "project/media.h"

class Project;

class NewSequenceDialog : public QDialog
{
    Q_OBJECT

  public:
    NewSequenceDialog(QWidget *parent = nullptr, MediaPtr existing = nullptr);
    virtual ~NewSequenceDialog();

    NewSequenceDialog(const NewSequenceDialog& ) = delete;
    NewSequenceDialog& operator=(const NewSequenceDialog&) = delete;

    void set_sequence_name(const QString& s);

  private slots:
    void create();
    void preset_changed(int index);

  private:
    SequencePtr existing_sequence;
    MediaPtr existing_item;

    void setup_ui();

    QComboBox* preset_combobox;
    QSpinBox* height_numeric;
    QSpinBox* width_numeric;
    QComboBox* par_combobox;
    QComboBox* interlacing_combobox;
    QComboBox* frame_rate_combobox;
    QComboBox* audio_frequency_combobox;
    QLineEdit* sequence_name_edit;
};

#endif // NEWSEQUENCEDIALOG_H
