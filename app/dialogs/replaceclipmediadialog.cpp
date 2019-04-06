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
#include "replaceclipmediadialog.h"

#include <QVBoxLayout>
#include <QTreeView>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>

#include "panels/project.h"
#include "ui/sourcetable.h"
#include "project/sequence.h"
#include "project/clip.h"
#include "playback/playback.h"
#include "project/footage.h"
#include "project/undo.h"
#include "project/media.h"


namespace{
  const int DIALOG_WIDTH = 300;
  const int DIALOG_HEIGHT = 400;
}

ReplaceClipMediaDialog::ReplaceClipMediaDialog(QWidget *parent, MediaPtr old_media) :
  QDialog(parent),
  media_(old_media)
{
  setWindowTitle(tr("Replace clips using \"%1\"").arg(old_media->name()));

  resize(DIALOG_WIDTH, DIALOG_HEIGHT);

  QVBoxLayout* layout = new QVBoxLayout();

  layout->addWidget(new QLabel(tr("Select which media you want to replace this media's clips with:")));

  tree = new QTreeView();

  layout->addWidget(tree);

  use_same_media_in_points = new QCheckBox(tr("Keep the same media in-points"));
  use_same_media_in_points->setChecked(true);
  layout->addWidget(use_same_media_in_points);

  QHBoxLayout* buttons = new QHBoxLayout();

  buttons->addStretch();

  QPushButton* replace_button = new QPushButton(tr("Replace"));
  connect(replace_button, SIGNAL(clicked(bool)), this, SLOT(replace()));
  buttons->addWidget(replace_button);

  QPushButton* cancel_button = new QPushButton(tr("Cancel"));
  connect(cancel_button, SIGNAL(clicked(bool)), this, SLOT(close()));
  buttons->addWidget(cancel_button);

  buttons->addStretch();

  layout->addLayout(buttons);

  setLayout(layout);

  tree->setModel(&Project::model());
}

void ReplaceClipMediaDialog::replace() {
  QModelIndexList selected_items = tree->selectionModel()->selectedRows();
  if (selected_items.size() != 1) {
    QMessageBox::critical(
          this,
          tr("No media selected"),
          tr("Please select a media to replace with or click 'Cancel'."),
          QMessageBox::Ok
          );
  } else {
    auto new_item = Project::model().getItem(selected_items.front());
    if (media_ == new_item) {
      QMessageBox::critical(
            this,
            tr("Same media selected"),
            tr("You selected the same media that you're replacing. Please select a different one or click 'Cancel'."),
            QMessageBox::Ok
            );
    } else if (new_item->type() == MediaType::FOLDER) {
      QMessageBox::critical(
            this,
            tr("Folder selected"),
            tr("You cannot replace footage with a folder."),
            QMessageBox::Ok
            );
    } else {
      if (new_item->type() == MediaType::SEQUENCE && global::sequence == new_item->object<Sequence>()) {
        QMessageBox::critical(
              this,
              tr("Active sequence selected"),
              tr("You cannot insert a sequence into itself."),
              QMessageBox::Ok
              );
      } else {
        ReplaceClipMediaCommand* rcmc = new ReplaceClipMediaCommand(
                                          media_,
                                          new_item,
                                          use_same_media_in_points->isChecked()
                                          );

        for (int i=0;i<global::sequence->clips_.size();i++) {
          ClipPtr c = global::sequence->clips_.at(i);
          if (c != nullptr && c->timeline_info.media == media_) {
            rcmc->clips.append(c);
          }
        }

        e_undo_stack.push(rcmc);

        close();
      }

    }
  }
}
