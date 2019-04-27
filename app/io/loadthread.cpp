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
#include "loadthread.h"

#include <QFile>
#include <QMessageBox>
#include <QTreeWidgetItem>

#include "panels/panelmanager.h"
#include "ui/mainwindow.h"
#include "project/footage.h"
#include "io/config.h"
#include "project/clip.h"
#include "project/sequence.h"
#include "project/transition.h"
#include "project/effect.h"
#include "playback/playback.h"
#include "io/previewgenerator.h"
#include "dialogs/loaddialog.h"
#include "project/media.h"
#include "effects/internal/voideffect.h"
#include "debug.h"

constexpr const char* const PROJ_FILE_EXT = ".nut";

using panels::PanelManager;

LoadThread::LoadThread(LoadDialog* l, const bool a)
  : ld(l),
    autorecovery(a),
    cancelled(false)
{
  qRegisterMetaType<ClipPtr>("ClipPtr");
  connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
  connect(this, SIGNAL(success()), this, SLOT(success_func()));
  connect(this, SIGNAL(error()), this, SLOT(error_func()));
}

void LoadThread::read_next(QXmlStreamReader &stream) {
  stream.readNext();
  update_current_element_count(stream);
}

void LoadThread::read_next_start_element(QXmlStreamReader &stream) {
  stream.readNextStartElement();
  update_current_element_count(stream);
}

void LoadThread::update_current_element_count(QXmlStreamReader &stream) {
  if (is_element(stream)) {
    current_element_count++;
    report_progress((current_element_count * 100) / total_element_count);
  }
}

bool LoadThread::is_element(QXmlStreamReader &stream) {
  return stream.isStartElement()
      && (stream.name() == "folder"
          || stream.name() == "footage"
          || stream.name() == "sequence"
          || stream.name() == "clip"
          || stream.name() == "effect");
}


MediaPtr LoadThread::find_loaded_folder_by_id(int id) {
//  if (id == 0) return nullptr;
//  for (int j=0;j<loaded_folders.size();j++) {
//    MediaPtr parent_item = loaded_folders.at(j);
//    if (parent_item->temp_id == id) {
//      return parent_item;
//    }
//  }
  return nullptr;
}

void LoadThread::run()
{
  mutex.lock();

  QFile file(project_url);
  if (!file.open(QIODevice::ReadOnly)) {
    qCritical() << "Could not open file";
    return;
  }

  QXmlStreamReader stream(&file);
  if (stream.readNextStartElement() && stream.name() == "project") {
    if (PanelManager::projectViewer().model().load(stream)) {
      qInfo() << "Successful load of project file" <<  file.fileName();
      emit success();
    } else {
      qCritical() << "Failed to read project file" << file.fileName();
      emit error();
    }
  } else {
    emit error();
    qWarning() << "Read failure" << file.fileName();
  }

  file.close();

  mutex.unlock();
}

void LoadThread::cancel() {
  waitCond.wakeAll();
  cancelled = true;
}

void LoadThread::error_func() {
  if (xml_error) {
    qCritical() << "Error parsing XML." << error_str;
    QMessageBox::critical(&MainWindow::instance(),
                          tr("XML Parsing Error"),
                          tr("Couldn't load '%1'. %2").arg(project_url, error_str),
                          QMessageBox::Ok);
  } else {
    QMessageBox::critical(&MainWindow::instance(),
                          tr("Project Load Error"),
                          tr("Error loading project: %1").arg(error_str),
                          QMessageBox::Ok);
  }
}

void LoadThread::success_func()
{
  if (autorecovery) {
    QString orig_filename = internal_proj_url;
    int insert_index = internal_proj_url.lastIndexOf(PROJ_FILE_EXT, -1, Qt::CaseInsensitive);
    if (insert_index == -1) insert_index = internal_proj_url.length();
    int counter = 1;
    while (QFileInfo::exists(orig_filename)) {
      orig_filename = internal_proj_url;
      QString recover_text = "recovered";
      if (counter > 1) {
        recover_text += " " + QString::number(counter);
      }
      orig_filename.insert(insert_index, " (" + recover_text + ")");
      counter++;
    }
    MainWindow::instance().updateTitle(orig_filename);
  } else {
    PanelManager::projectViewer().add_recent_project(project_url);
  }

  MainWindow::instance().setWindowModified(autorecovery);
  if (global::sequence != nullptr) {
    // global::sequence will be set in file load as required
    //TODO: think of a better way than this and playback.h - set_sequence()
    PanelManager::fxControls().clear_effects(true);
    PanelManager::sequenceViewer().set_main_sequence();
    PanelManager::timeLine().setSequence(global::sequence);
    PanelManager::timeLine().update_sequence();
    PanelManager::timeLine().setFocus();
  }
  PanelManager::refreshPanels(false);
}


void LoadThread::create_dual_transition(const TransitionData& td, ClipPtr  primary, ClipPtr  secondary, const EffectMeta& meta)
{
  //TODO: reassess
//  int transition_index = create_transition(primary, secondary, meta);
//  primary->sequence->transitions_.at(transition_index)->set_length(td.length);
//  if (td.otc != nullptr) {
//    td.otc->opening_transition = transition_index;
//  }
//  if (td.ctc != nullptr) {
//    td.ctc->closing_transition = transition_index;
//  }
//  waitCond.wakeAll();
}
