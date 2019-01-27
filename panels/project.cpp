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
#include "project.h"
#include "project/footage.h"

#include "panels/panels.h"
#include "panels/timeline.h"
#include "panels/viewer.h"
#include "playback/playback.h"
#include "project/effect.h"
#include "project/transition.h"
#include "panels/timeline.h"
#include "project/sequence.h"
#include "project/clip.h"
#include "io/previewgenerator.h"
#include "project/undo.h"
#include "mainwindow.h"
#include "io/config.h"
#include "dialogs/replaceclipmediadialog.h"
#include "panels/effectcontrols.h"
#include "dialogs/newsequencedialog.h"
#include "dialogs/mediapropertiesdialog.h"
#include "dialogs/loaddialog.h"
#include "io/clipboard.h"
#include "project/media.h"
#include "ui/sourcetable.h"
#include "ui/sourceiconview.h"
#include "project/sourcescommon.h"
#include "project/projectfilter.h"
#include "debug.h"

#include <QApplication>
#include <QFileDialog>
#include <QString>
#include <QVariant>
#include <QCharRef>
#include <QMessageBox>
#include <QDropEvent>
#include <QMimeData>
#include <QPushButton>
#include <QInputDialog>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QMenu>
#include <memory>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}


ProjectModel project_model;

QString autorecovery_filename;
QString project_url = "";
QStringList recent_projects;
QString recent_proj_file;

namespace
{
const int       MAXIMUM_RECENT_PROJECTS             = 10;
const int       SEQUENCE_DEFAULT_WIDTH              = 1920;
const int       SEQUENCE_DEFAULT_HEIGHT             = 1080;
const double    SEQUENCE_DEFAULT_FRAMERATE          = 29.97;
const int       SEQUENCE_DEFAULT_AUDIO_FREQUENCY    = 48000;
const int       SEQUENCE_DEFAULT_LAYOUT             = 3;

const int       THROBBER_INTERVAL                   = 20; //ms
const int       THROBBER_LIMIT                      = 20;
const int       THROBBER_SIZE                       = 50;
}

Project::Project(QWidget *parent) :
    QDockWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QWidget* dockWidgetContents = new QWidget();
    QVBoxLayout* verticalLayout = new QVBoxLayout(dockWidgetContents);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(0);

    setWidget(dockWidgetContents);

    sources_common = new SourcesCommon(this);

    sorter = new ProjectFilter(this);
    sorter->setSourceModel(&project_model);

    // optional toolbar
    toolbar_widget = new QWidget();
    toolbar_widget->setVisible(e_config.show_project_toolbar);
    QHBoxLayout* toolbar = new QHBoxLayout();
    toolbar->setMargin(0);
    toolbar->setSpacing(0);
    toolbar_widget->setLayout(toolbar);

    QPushButton* toolbar_new = new QPushButton("New");
    toolbar_new->setIcon(QIcon(":/icons/tri-down.png"));
    toolbar_new->setIconSize(QSize(8, 8));
    toolbar_new->setToolTip("New");
    connect(toolbar_new, SIGNAL(clicked(bool)), this, SLOT(make_new_menu()));
    toolbar->addWidget(toolbar_new);

    QPushButton* toolbar_open = new QPushButton("Open");
    toolbar_open->setToolTip("Open Project");
    connect(toolbar_open, SIGNAL(clicked(bool)), global::mainWindow, SLOT(open_project()));
    toolbar->addWidget(toolbar_open);

    QPushButton* toolbar_save = new QPushButton("Save");
    toolbar_save->setToolTip("Save Project");
    connect(toolbar_save, SIGNAL(clicked(bool)), global::mainWindow, SLOT(save_project()));
    toolbar->addWidget(toolbar_save);

    QPushButton* toolbar_undo = new QPushButton("Undo");
    toolbar_undo->setToolTip("Undo");
    connect(toolbar_undo, SIGNAL(clicked(bool)), global::mainWindow, SLOT(undo()));
    toolbar->addWidget(toolbar_undo);

    QPushButton* toolbar_redo = new QPushButton("Redo");
    toolbar_redo->setToolTip("Redo");
    connect(toolbar_redo, SIGNAL(clicked(bool)), global::mainWindow, SLOT(redo()));
    toolbar->addWidget(toolbar_redo);

    toolbar->addStretch();

    QPushButton* toolbar_tree_view = new QPushButton("Tree View");
    toolbar_tree_view->setToolTip("Tree View");
    connect(toolbar_tree_view, SIGNAL(clicked(bool)), this, SLOT(set_tree_view()));
    toolbar->addWidget(toolbar_tree_view);

    QPushButton* toolbar_icon_view = new QPushButton("Icon View");
    toolbar_icon_view->setToolTip("Icon View");
    connect(toolbar_icon_view, SIGNAL(clicked(bool)), this, SLOT(set_icon_view()));
    toolbar->addWidget(toolbar_icon_view);

    verticalLayout->addWidget(toolbar_widget);

    // tree view
    tree_view = new SourceTable(dockWidgetContents);
    tree_view->project_parent = this;
    tree_view->setModel(sorter);
    verticalLayout->addWidget(tree_view);

    // icon view
    icon_view_container = new QWidget();

    QVBoxLayout* icon_view_container_layout = new QVBoxLayout();
    icon_view_container_layout->setMargin(0);
    icon_view_container_layout->setSpacing(0);
    icon_view_container->setLayout(icon_view_container_layout);

    QHBoxLayout* icon_view_controls = new QHBoxLayout();
    icon_view_controls->setMargin(0);
    icon_view_controls->setSpacing(0);

    QIcon directory_up_button;
    directory_up_button.addFile(":/icons/dirup.png", QSize(), QIcon::Normal);
    directory_up_button.addFile(":/icons/dirup-disabled.png", QSize(), QIcon::Disabled);

    directory_up = new QPushButton();
    directory_up->setIcon(directory_up_button);
    directory_up->setEnabled(false);
    icon_view_controls->addWidget(directory_up);

    icon_view_controls->addStretch();

    QSlider* icon_size_slider = new QSlider(Qt::Horizontal);
    icon_size_slider->setMinimum(16);
    icon_size_slider->setMaximum(120);
    icon_view_controls->addWidget(icon_size_slider);
    connect(icon_size_slider, SIGNAL(valueChanged(int)), this, SLOT(set_icon_view_size(int)));

    icon_view_container_layout->addLayout(icon_view_controls);

    icon_view = new SourceIconView(dockWidgetContents);
    icon_view->project_parent = this;
    icon_view->setModel(sorter);
    icon_view->setIconSize(QSize(100, 100));
    icon_view->setViewMode(QListView::IconMode);
    icon_view->setUniformItemSizes(true);
    icon_view_container_layout->addWidget(icon_view);

    icon_size_slider->setValue(icon_view->iconSize().height());

    verticalLayout->addWidget(icon_view_container);

    connect(directory_up, SIGNAL(clicked(bool)), this, SLOT(go_up_dir()));
    connect(icon_view, SIGNAL(changed_root()), this, SLOT(set_up_dir_enabled()));

    //retranslateUi(Project);
    setWindowTitle(tr("Project"));

    update_view_type();
}

Project::~Project() {
    delete sorter;
    delete sources_common;
}

QString Project::get_next_sequence_name(QString start) {
    if (start.isEmpty()) start = tr("Sequence");

    int n = 1;
    bool found = true;
    QString name;
    while (found) {
        found = false;
        name = start + " ";
        if (n < 10) {
            name += "0";
        }
        name += QString::number(n);
        for (int i=0;i<project_model.childCount();i++) {
            if (QString::compare(project_model.child(i)->get_name(), name, Qt::CaseInsensitive) == 0) {
                found = true;
                n++;
                break;
            }
        }
    }
    return name;
}

SequencePtr create_sequence_from_media(QVector<MediaPtr>& media_list) {
    SequencePtr  s = std::make_shared<Sequence>();

    s->setName(e_panel_project->get_next_sequence_name());

    s->setWidth(SEQUENCE_DEFAULT_HEIGHT);
    s->setHeight(SEQUENCE_DEFAULT_WIDTH);
    s->setFrameRate(SEQUENCE_DEFAULT_FRAMERATE);
    s->setAudioFrequency(SEQUENCE_DEFAULT_AUDIO_FREQUENCY);
    s->setAudioLayout(SEQUENCE_DEFAULT_LAYOUT);

    bool got_video_values = false;
    bool got_audio_values = false;
    for (MediaPtr mda: media_list){
        if (mda == nullptr) {
            qCritical() << "Null MediaPtr";
            continue;
        }
        switch (mda->get_type()) {
        case MEDIA_TYPE_FOOTAGE:
        {
            FootagePtr mediaFootage = mda->get_object<Footage>();
            if (mediaFootage->ready) {
                if (!got_video_values) {
                    for (int j=0;j<mediaFootage->video_tracks.size();j++) {
                        const FootageStream& ms = mediaFootage->video_tracks.at(j);
                        s->setWidth(ms.video_width);
                        s->setHeight(ms.video_height);
                        if (!qFuzzyCompare(ms.video_frame_rate, 0.0)) {
                            s->setFrameRate(ms.video_frame_rate * mediaFootage->speed);

                            if (ms.video_interlacing != VIDEO_PROGRESSIVE) {
                                s->setFrameRate(s->getFrameRate() * 2);
                            }

                            // only break with a decent frame rate, otherwise there may be a better candidate
                            got_video_values = true;
                            break;
                        }
                    }
                }
                if (!got_audio_values && mediaFootage->audio_tracks.size() > 0) {
                    const FootageStream& ms = mediaFootage->audio_tracks.at(0);
                    s->setAudioFrequency(ms.audio_frequency);
                    got_audio_values = true;
                }
            }
        }
            break;
        case MEDIA_TYPE_SEQUENCE:
        {
            SequencePtr  seq = mda->get_object<Sequence>();
            if (seq != nullptr) {
                s->setWidth(seq->getWidth());
                s->setHeight(seq->getHeight());
                s->setFrameRate(seq->getFrameRate());
                s->setAudioFrequency(seq->getAudioFrequency());
                s->setAudioLayout(seq->getAudioLayout());

                got_video_values = true;
                got_audio_values = true;
            }
        }
            break;
        default:
            qWarning() << "Unknown media type" << mda->get_type();
        }//switch
        if (got_video_values && got_audio_values) break;
    }

    return s;
}

void Project::duplicate_selected() {
    QModelIndexList items = get_current_selected();
    bool duped = false;
    ComboAction* ca = new ComboAction();
    for (int j=0;j<items.size();j++) {
        MediaPtr i = item_to_media(items.at(j));
        if (i->get_type() == MEDIA_TYPE_SEQUENCE) {
            new_sequence(ca, SequencePtr(i->get_object<Sequence>()->copy()), false, item_to_media(items.at(j).parent()));
            duped = true;
        }
    }
    if (duped) {
        e_undo_stack.push(ca);
    } else {
        delete ca;
    }
}

void Project::replace_selected_file() {
    QModelIndexList selected_items = get_current_selected();
    if (selected_items.size() == 1) {
        MediaPtr item = item_to_media(selected_items.at(0));
        if (item->get_type() == MEDIA_TYPE_FOOTAGE) {
            replace_media(item, nullptr);
        }
    }
}

void Project::replace_media(MediaPtr item, QString filename) {
    if (filename.isEmpty()) {
        filename = QFileDialog::getOpenFileName(
                    this,
                    tr("Replace '%1'").arg(item->get_name()),
                    "",
                    tr("All Files") + " (*)");
    }
    if (!filename.isEmpty()) {
        ReplaceMediaCommand* rmc = new ReplaceMediaCommand(item, filename);
        e_undo_stack.push(rmc);
    }
}

void Project::replace_clip_media() {
    if (global::sequence == nullptr) {
        QMessageBox::critical(this,
                              tr("No active sequence"),
                              tr("No sequence is active, please open the sequence you want to replace clips from."),
                              QMessageBox::Ok);
    } else {
        QModelIndexList selected_items = get_current_selected();
        if (selected_items.size() == 1) {
            MediaPtr item = item_to_media(selected_items.at(0));
            if (item->get_type() == MEDIA_TYPE_SEQUENCE && global::sequence == item->get_object<Sequence>()) {
                QMessageBox::critical(this,
                                      tr("Active sequence selected"),
                                      tr("You cannot insert a sequence into itself, so no clips of this media would be in this sequence."),
                                      QMessageBox::Ok);
            } else {
                ReplaceClipMediaDialog dialog(this, item);
                dialog.exec();
            }
        }
    }
}

void Project::open_properties() {
    QModelIndexList selected_items = get_current_selected();
    if (selected_items.size() == 1) {
        MediaPtr item = item_to_media(selected_items.at(0));
        switch (item->get_type()) {
        case MEDIA_TYPE_FOOTAGE:
        {
            MediaPropertiesDialog mpd(this, item);
            mpd.exec();
        }
            break;
        case MEDIA_TYPE_SEQUENCE:
        {
            NewSequenceDialog nsd(this, item);
            nsd.exec();
        }
            break;
        default:
        {
            // fall back to renaming
            QString new_name = QInputDialog::getText(this,
                                                     tr("Rename '%1'").arg(item->get_name()),
                                                     tr("Enter new name:"),
                                                     QLineEdit::Normal,
                                                     item->get_name());
            if (!new_name.isEmpty()) {
                MediaRename* mr = new MediaRename(item, new_name);
                e_undo_stack.push(mr);
            }
        }
        }
    }
}

MediaPtr Project::new_sequence(ComboAction *ca, SequencePtr s, bool open, MediaPtr parent) {
    if (parent == nullptr) parent = project_model.get_root();
    MediaPtr item = std::make_shared<Media>(parent);
    item->set_sequence(s);

    if (ca != nullptr) {
        ca->append(new NewSequenceCommand(item, parent));
        if (open) ca->append(new ChangeSequenceAction(s));
    } else {
        if (parent == project_model.get_root()) {
            project_model.appendChild(parent, item);
        } else {
            parent->appendChild(item);
        }
        if (open) set_sequence(s);
    }
    return item;
}

QString Project::get_file_name_from_path(const QString& path) {
    return QFileInfo(path).fileName();
}


QString Project::get_file_ext_from_path(const QString &path) {
    return QFileInfo(path).suffix();
}

/*MediaPtr Project::new_item() {
    MediaPtr item = new Media(0);
    //item->setFlags(item->flags() | Qt::ItemIsEditable);
    return item;
}*/

bool Project::is_focused() {
    return tree_view->hasFocus() || icon_view->hasFocus();
}

MediaPtr Project::new_folder(const QString &name) {
    MediaPtr item = std::make_shared<Media>();
    item->set_folder();
    item->set_name(name);
    return item;
}

MediaPtr Project::item_to_media(const QModelIndex &index) {
    if (sorter != nullptr) {
        const auto src = sorter->mapToSource(index);
        return project_model.get(src);
    }

    return MediaPtr();
}

void Project::get_all_media_from_table(QVector<MediaPtr>& items, QVector<MediaPtr>& list, int search_type) {
    for (int i=0;i<items.size();i++) {
        MediaPtr item = items.at(i);
        if (item->get_type() == MEDIA_TYPE_FOLDER) {
            QVector<MediaPtr> children;
            for (int j=0;j<item->childCount();j++) {
                children.append(item->child(j));
            }
            get_all_media_from_table(children, list, search_type);
        } else if (search_type == item->get_type() || search_type == -1) {
            list.append(item);
        }
    }
}

bool delete_clips_in_clipboard_with_media(ComboAction* ca, MediaPtr m) {
    int delete_count = 0;
    if (e_clipboard_type == CLIPBOARD_TYPE_CLIP) {
        for (int i=0;i<e_clipboard.size();i++) {
            ClipPtr c = std::dynamic_pointer_cast<Clip>(e_clipboard.at(i));
            if (c->timeline_info.media == m) {
                ca->append(new RemoveClipsFromClipboard(i-delete_count));
                delete_count++;
            }
        }
    }
    return (delete_count > 0);
}

void Project::delete_selected_media() {
    auto ca = new ComboAction();
    auto selected_items = get_current_selected();
    QVector<MediaPtr> items;
    for (auto idx : selected_items) {
        auto mda = item_to_media(idx);
        if (mda == nullptr) {
            qCritical() << "Null Media Ptr";
            continue;
        }
        items.append(mda);
    }
    auto remove = true;
    auto redraw = false;

    // check if media is in use
    QVector<MediaPtr> parents;
    QVector<MediaPtr> sequence_items;
    QVector<MediaPtr> all_top_level_items;
    for (auto i=0;i<project_model.childCount();i++) {
        all_top_level_items.append(project_model.child(i));
    }
    get_all_media_from_table(all_top_level_items, sequence_items, MEDIA_TYPE_SEQUENCE); // find all sequences in project
    if (sequence_items.size() > 0) {
        QVector<MediaPtr> media_items;
        get_all_media_from_table(items, media_items, MEDIA_TYPE_FOOTAGE);
        auto abort = false;
        for (auto i=0; (i<media_items.size()) && (!abort); ++i) {
            auto item = media_items.at(i);
            bool confirm_delete = false;
            auto skip = false;
            for (auto j=0; j<sequence_items.size() && (!abort) && (!skip); ++j) {
                auto seq = sequence_items.at(j)->get_object<Sequence>();
                for (auto k=0; (k<seq->clips.size()) && (!abort) && (!skip); ++k) {
                    auto c = seq->clips.at(k);
                    if ( (c != nullptr) && (c->timeline_info.media == item) ) {
                        if (!confirm_delete) {
                            auto ftg = item->get_object<Footage>();
                            // we found a reference, so we know we'll need to ask if the user wants to delete it
                            QMessageBox confirm(this);
                            confirm.setWindowTitle(tr("Delete media in use?"));
                            confirm.setText(tr("The media '%1' is currently used in '%2'. Deleting it will remove all instances in the sequence."
                                               "Are you sure you want to do this?").arg(ftg ->getName(), seq->getName()));
                            auto yes_button = confirm.addButton(QMessageBox::Yes);
                            QAbstractButton* skip_button = nullptr;
                            if (items.size() > 1) skip_button = confirm.addButton("Skip", QMessageBox::NoRole);
                            auto abort_button = confirm.addButton(QMessageBox::Cancel);
                            confirm.exec();
                            if (confirm.clickedButton() == yes_button) {
                                // remove all clips referencing this media
                                confirm_delete = true;
                                redraw = true;
                            } else if (confirm.clickedButton() == skip_button) {
                                // remove media item and any folders containing it from the remove list
                                auto parent = item;
                                while (parent) {
                                    parents.append(parent);
                                    // re-add item's siblings
                                    for (int m=0; m < parent->childCount();m++) {
                                        auto child = parent->child(m);
                                        bool found = false;
                                        for (int n=0; n<items.size(); n++) {
                                            if (items.at(n) == child) {
                                                found = true;
                                                break;
                                            }
                                        }
                                        if (!found) {
                                            items.append(child);
                                        }
                                    }

                                    parent = parent->parentItem();
                                }//while

                                skip = true;
                            } else if (confirm.clickedButton() == abort_button) {
                                // break out of loop
                                abort = true;
                                remove = false;
                            } else {
                                // TODO: anything expected to be done here?
                            }
                        }
                        if (confirm_delete) {
                            ca->append(new DeleteClipAction(seq, k));
                        }
                    }
                }//for
            }//for
            if (confirm_delete) {
                delete_clips_in_clipboard_with_media(ca, item);
            }
        }//for
    }

    // remove
    if (remove) {
        e_panel_effect_controls->clear_effects(true);
        if (global::sequence != nullptr) global::sequence->selections.clear();

        // remove media and parents
        for (int m=0; m < parents.size(); m++) {
            for (int l=0; l < items.size(); l++) {
                if (auto parPtr = parents.at(m)) {
                    if (items.at(l) == parPtr) {
                        items.removeAt(l);
                        l--;
                    }
                }
            }//for
        }//for

        for (auto item : items) {
            if (!item) {
                continue;
            }
            ca->append(new DeleteMediaCommand(item));

            if (item->get_type() == MEDIA_TYPE_SEQUENCE) {
                redraw = true;

                auto s = item->get_object<Sequence>();

                if (s == global::sequence) {
                    ca->append(new ChangeSequenceAction(nullptr));
                }

                if (s == e_panel_footage_viewer->getSequence()) {
                    e_panel_footage_viewer->set_media(nullptr);
                }
            } else if (item->get_type() == MEDIA_TYPE_FOOTAGE) {
                if (e_panel_footage_viewer->getSequence()) {
                    for (auto clp : e_panel_footage_viewer->getSequence()->clips) {
                        if (clp) {
                            if (clp->timeline_info.media == item) {
                                // Media viewer is displaying the clip for deletion, so clear it
                                e_panel_footage_viewer->set_media(nullptr); //TODO: create a clear()
                                break;
                            }
                        }
                    }//for
                }
            }
        } //for
        e_undo_stack.push(ca);

        // redraw clips
        if (redraw) {
            update_ui(true);
        }
    } else {
        delete ca;
    }
}

void Project::start_preview_generator(MediaPtr item, bool replacing) {
    // set up throbber animation
    MediaThrobber* throbber = new MediaThrobber(item);
    throbber->moveToThread(QApplication::instance()->thread());
    item->throbber = throbber;
    QMetaObject::invokeMethod(throbber, "start", Qt::QueuedConnection);

    PreviewGeneratorUPtr pg = std::make_unique<PreviewGenerator>(item, item->get_object<Footage>(), replacing); //FIXME: leak
    connect(pg.get(), SIGNAL(set_icon(int, bool)), throbber, SLOT(stop(int, bool)));
    pg->start(QThread::LowPriority);
    item->get_object<Footage>()->preview_gen = std::move(pg);
}

void Project::process_file_list(QStringList& files, bool recursive, MediaPtr replace, MediaPtr parent) {
    bool imported = false;

    QVector<QString> image_sequence_urls;
    QVector<bool> image_sequence_importassequence;
    QStringList image_sequence_formats = e_config.img_seq_formats.split("|");

    if (!recursive) {
        last_imported_media.clear();
    }

    bool create_undo_action = (!recursive && replace == nullptr);
    ComboAction* ca = nullptr;
    if (create_undo_action) {
        ca = new ComboAction();
    }

    for (QString fileName: files) {
        if (QFileInfo(fileName).isDir()) {
            QString folder_name = get_file_name_from_path(fileName);
            MediaPtr folder = new_folder(folder_name);

            QDir directory(fileName);
            directory.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);

            QFileInfoList subdir_files = directory.entryInfoList();
            QStringList subdir_filenames;

            for (int j=0; j<subdir_files.size(); j++) {
                subdir_filenames.append(subdir_files.at(j).filePath());
            }

            process_file_list(subdir_filenames, true, nullptr, folder);

            if (create_undo_action) {
                ca->append(new AddMediaCommand(folder, parent));
            } else {
                project_model.appendChild(parent, folder);
            }

            imported = true;
        } else if (!fileName.isEmpty()) {
            bool skip = false;
            /* Heuristic to determine whether file is part of an image sequence */
            // check file extension (assume it's not a
            int lastcharindex = fileName.lastIndexOf(".");
            bool found = true;
            if ( (lastcharindex != -1) && (lastcharindex > fileName.lastIndexOf('/')) ) {
                // img sequence check
                const QString ext(get_file_ext_from_path(fileName));
                found = image_sequence_formats.contains(ext);
            } else {
                lastcharindex = fileName.length();
            }

            if (lastcharindex == 0) {
                lastcharindex++;
            }

            if (found && fileName[lastcharindex-1].isDigit()) {
                bool is_img_sequence = false;

                // how many digits are in the filename?
                int digit_count = 0;
                int digit_test = lastcharindex-1;
                while (fileName[digit_test].isDigit()) {
                    digit_count++;
                    digit_test--;
                }

                // retrieve number from filename
                digit_test++;
                int file_number = fileName.mid(digit_test, digit_count).toInt();

                // Check if there are files with the same filename but just different numbers
                if (QFileInfo::exists(QString(fileName.left(digit_test) + QString("%1").arg(file_number-1, digit_count, 10, QChar('0')) + fileName.mid(lastcharindex)))
                        || QFileInfo::exists(QString(fileName.left(digit_test) + QString("%1").arg(file_number+1, digit_count, 10, QChar('0')) + fileName.mid(lastcharindex)))) {
                    is_img_sequence = true;
                }

                if (is_img_sequence) {
                    // get the URL that we would pass to FFmpeg to force it to read the image as a sequence
                    QString new_filename = fileName.left(digit_test) + "%" + QString("%1").arg(digit_count, 2, 10, QChar('0')) + "d" + fileName.mid(lastcharindex);

                    // add image sequence url to a vector in case the user imported several files that
                    // we're interpreting as a possible sequence
                    found = false;
                    for (int i=0;i<image_sequence_urls.size();i++) {
                        if (image_sequence_urls.at(i) == new_filename) {
                            // either SKIP if we're importing as a sequence, or leave it if we aren't
                            if (image_sequence_importassequence.at(i)) {
                                skip = true;
                            }
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        image_sequence_urls.append(new_filename);
                        if (QMessageBox::question(this,
                                                  tr("Image sequence detected"),
                                                  tr("The file '%1' appears to be part of an image sequence. Would you like to import it as such?").arg(fileName),
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::Yes) == QMessageBox::Yes) {
                            fileName = new_filename;
                            image_sequence_importassequence.append(true);
                        } else {
                            image_sequence_importassequence.append(false);
                        }
                    }
                }
            }

            if (!skip) {
                MediaPtr item;
                FootagePtr ftg;

                if (replace != nullptr) {
                    item = replace;
                    ftg = replace->get_object<Footage>();
                    ftg->reset();
                } else {
                    item = std::make_shared<Media>(parent);
                    ftg = std::make_shared<Footage>();
                }

                ftg->using_inout = false;
                ftg->url = fileName;
                ftg->setName(get_file_name_from_path(fileName));

                item->set_footage(ftg);

                last_imported_media.append(item);

                if (replace == nullptr) {
                    if (create_undo_action) {
                        ca->append(new AddMediaCommand(item, parent));
                    } else {
                        parent->appendChild(item);
                        //                        project_model.appendChild(parent, item);
                    }
                }

                imported = true;
            }
        }
    }
    if (create_undo_action) {
        if (imported) {
            e_undo_stack.push(ca);

            for (MediaPtr mda : last_imported_media){
                // generate waveform/thumbnail in another thread
                start_preview_generator(mda, replace != nullptr);
            }
        } else {
            delete ca;
        }
    }
}

MediaPtr Project::get_selected_folder() {
    // if one item is selected and it's a folder, return it
    QModelIndexList selected_items = get_current_selected();
    if (selected_items.size() == 1) {
        MediaPtr m = item_to_media(selected_items.front());
        if (m != nullptr) {
            if (m->get_type() == MEDIA_TYPE_FOLDER) {
                return m;
            }
        } else {
            qCritical() << "Null Media Ptr";
        }
    }
    return nullptr;
}

bool Project::reveal_media(MediaPtr media, QModelIndex parent) {
    for (int i=0;i<project_model.rowCount(parent);i++) {
        const QModelIndex& item = project_model.index(i, 0, parent);
        MediaPtr m = project_model.getItem(item);

        if (m->get_type() == MEDIA_TYPE_FOLDER) {
            if (reveal_media(media, item)) return true;
        } else if (m == media) {
            // expand all folders leading to this media
            QModelIndex sorted_index = sorter->mapFromSource(item);

            QModelIndex hierarchy = sorted_index.parent();

            if (e_config.project_view_type == PROJECT_VIEW_TREE) {
                while (hierarchy.isValid()) {
                    tree_view->setExpanded(hierarchy, true);
                    hierarchy = hierarchy.parent();
                }

                // select item
                tree_view->selectionModel()->select(sorted_index, QItemSelectionModel::Select);
            } else if (e_config.project_view_type == PROJECT_VIEW_ICON) {
                icon_view->setRootIndex(hierarchy);
                icon_view->selectionModel()->select(sorted_index, QItemSelectionModel::Select);
                set_up_dir_enabled();
            }

            return true;
        }
    }

    return false;
}

void Project::import_dialog() {
    QFileDialog fd(this, tr("Import media..."), "", tr("All Files") + " (*)");
    fd.setFileMode(QFileDialog::ExistingFiles);

    if (fd.exec()) {
        QStringList files = fd.selectedFiles();
        process_file_list(files, false, nullptr, get_selected_folder());
    }
}

void Project::delete_clips_using_selected_media() {
    if (global::sequence == nullptr) {
        QMessageBox::critical(this,
                              tr("No active sequence"),
                              tr("No sequence is active, please open the sequence you want to delete clips from."),
                              QMessageBox::Ok);
    } else {
        ComboAction* ca = new ComboAction();
        bool deleted = false;
        QModelIndexList items = get_current_selected();
        for (int i=0;i<global::sequence->clips.size();i++) {
            ClipPtr c = global::sequence->clips.at(i);
            if (c != nullptr) {
                for (int j=0;j<items.size();j++) {
                    MediaPtr m = item_to_media(items.at(j));
                    if (c->timeline_info.media == m) {
                        ca->append(new DeleteClipAction(global::sequence, i));
                        deleted = true;
                    }
                }
            }
        }
        for (int j=0;j<items.size();j++) {
            MediaPtr m = item_to_media(items.at(j));
            if (delete_clips_in_clipboard_with_media(ca, m)) deleted = true;
        }
        if (deleted) {
            e_undo_stack.push(ca);
            update_ui(true);
        } else {
            delete ca;
        }
    }
}

void Project::clear() {
    // clear effects cache
    e_panel_effect_controls->clear_effects(true);

    // delete sequences first because it's important to close all the clips before deleting the media
    QVector<MediaPtr> sequences = list_all_project_sequences();
    //TODO: are we clearing the right things?
    for (int i=0;i<sequences.size();i++) {
        MediaPtr const tmp = sequences.at(i);
        if (tmp != nullptr) {
            tmp->clear_object();
        }
    }

    // delete everything else
    project_model.clear();
}

void Project::new_project() {
    // clear existing project
    set_sequence(nullptr);
    e_panel_footage_viewer->set_media(nullptr);
    clear();
    global::mainWindow->setWindowModified(false);
}

void Project::load_project(bool autorecovery) {
    new_project();

    LoadDialog ld(this, autorecovery);
    ld.exec();
}

void Project::save_folder(QXmlStreamWriter& stream, int type, bool set_ids_only, const QModelIndex& parent) {
    for (int i=0;i<project_model.rowCount(parent);i++) {
        const QModelIndex& item = project_model.index(i, 0, parent);
        MediaPtr mda = project_model.getItem(item);
        if (mda == nullptr) {
            qCritical() << "Null Media Ptr";
            continue;
        }

        if (type == mda->get_type()) {
            if (mda->get_type() == MEDIA_TYPE_FOLDER) {
                if (set_ids_only) {
                    mda->temp_id = folder_id; // saves a temporary ID for matching in the project file
                    folder_id++;
                } else {
                    // if we're saving folders, save the folder
                    stream.writeStartElement("folder");
                    stream.writeAttribute("name", mda->get_name());
                    stream.writeAttribute("id", QString::number(mda->temp_id));
                    if (!item.parent().isValid()) {
                        stream.writeAttribute("parent", "0");
                    } else {
                        stream.writeAttribute("parent", QString::number(project_model.getItem(item.parent())->temp_id));
                    }
                    stream.writeEndElement();
                }
                // save_folder(stream, item, type, set_ids_only);
            } else {
                int folder;
                if (auto parPtr = mda->parentItem()) {
                    folder = parPtr->temp_id;
                } else {
                    folder = 0;
                }
                if (type == MEDIA_TYPE_FOOTAGE) {
                    auto f = mda->get_object<Footage>();
                    f->save_id = media_id;
                    stream.writeStartElement("footage");
                    stream.writeAttribute("id", QString::number(media_id));
                    stream.writeAttribute("folder", QString::number(folder));
                    stream.writeAttribute("name", f->getName());
                    stream.writeAttribute("url", proj_dir.relativeFilePath(f->url));
                    stream.writeAttribute("duration", QString::number(f->length));
                    stream.writeAttribute("using_inout", QString::number(f->using_inout));
                    stream.writeAttribute("in", QString::number(f->in));
                    stream.writeAttribute("out", QString::number(f->out));
                    stream.writeAttribute("speed", QString::number(f->speed));
                    for (int j=0;j<f->video_tracks.size();j++) {
                        const FootageStream& ms = f->video_tracks.at(j);
                        stream.writeStartElement("video");
                        stream.writeAttribute("id", QString::number(ms.file_index));
                        stream.writeAttribute("width", QString::number(ms.video_width));
                        stream.writeAttribute("height", QString::number(ms.video_height));
                        stream.writeAttribute("framerate", QString::number(ms.video_frame_rate, 'f', 10));
                        stream.writeAttribute("infinite", QString::number(ms.infinite_length));
                        stream.writeEndElement();
                    }
                    for (int j=0;j<f->audio_tracks.size();j++) {
                        const FootageStream& ms = f->audio_tracks.at(j);
                        stream.writeStartElement("audio");
                        stream.writeAttribute("id", QString::number(ms.file_index));
                        stream.writeAttribute("channels", QString::number(ms.audio_channels));
                        stream.writeAttribute("layout", QString::number(ms.audio_layout));
                        stream.writeAttribute("frequency", QString::number(ms.audio_frequency));
                        stream.writeEndElement();
                    }
                    stream.writeEndElement();
                    media_id++;
                } else if (type == MEDIA_TYPE_SEQUENCE) {
                    SequencePtr  s = mda->get_object<Sequence>();
                    if (set_ids_only) {
                        s->save_id = sequence_id;
                        sequence_id++;
                    } else {
                        stream.writeStartElement("sequence");
                        stream.writeAttribute("id", QString::number(s->save_id));
                        stream.writeAttribute("folder", QString::number(folder));
                        stream.writeAttribute("name", s->getName());
                        stream.writeAttribute("width", QString::number(s->getWidth()));
                        stream.writeAttribute("height", QString::number(s->getHeight()));
                        stream.writeAttribute("framerate", QString::number(s->getFrameRate(), 'f', 10));
                        stream.writeAttribute("afreq", QString::number(s->getAudioFrequency()));
                        stream.writeAttribute("alayout", QString::number(s->getAudioLayout()));
                        if (s == global::sequence) {
                            stream.writeAttribute("open", "1");
                        }
                        stream.writeAttribute("workarea", QString::number(s->using_workarea));
                        stream.writeAttribute("workareaEnabled", QString::number(s->enable_workarea));
                        stream.writeAttribute("workareaIn", QString::number(s->workarea_in));
                        stream.writeAttribute("workareaOut", QString::number(s->workarea_out));

                        for (int j=0;j<s->transitions.size();j++) {
                            auto t = s->transitions.at(j);
                            if (t != nullptr) {
                                stream.writeStartElement("transition");
                                stream.writeAttribute("id", QString::number(j));
                                stream.writeAttribute("length", QString::number(t->get_true_length()));
                                t->save(stream);
                                stream.writeEndElement(); // transition
                            }
                        }

                        for (int j=0;j<s->clips.size();j++) {
                            auto c = s->clips.at(j);
                            if (c != nullptr) {
                                stream.writeStartElement("clip"); // clip
                                stream.writeAttribute("id", QString::number(j));
                                stream.writeAttribute("enabled", QString::number(c->timeline_info.enabled));
                                stream.writeAttribute("name", c->timeline_info.name);
                                stream.writeAttribute("clipin", QString::number(c->timeline_info.clip_in));
                                stream.writeAttribute("in", QString::number(c->timeline_info.in));
                                stream.writeAttribute("out", QString::number(c->timeline_info.out));
                                stream.writeAttribute("track", QString::number(c->timeline_info.track));
                                stream.writeAttribute("opening", QString::number(c->opening_transition));
                                stream.writeAttribute("closing", QString::number(c->closing_transition));

                                stream.writeAttribute("r", QString::number(c->timeline_info.color.red()));
                                stream.writeAttribute("g", QString::number(c->timeline_info.color.green()));
                                stream.writeAttribute("b", QString::number(c->timeline_info.color.blue()));

                                stream.writeAttribute("autoscale", QString::number(c->timeline_info.autoscale));
                                stream.writeAttribute("speed", QString::number(c->timeline_info.speed, 'f', 10));
                                stream.writeAttribute("maintainpitch", QString::number(c->timeline_info.maintain_audio_pitch));
                                stream.writeAttribute("reverse", QString::number(c->timeline_info.reverse));

                                if (c->timeline_info.media != nullptr) {
                                    stream.writeAttribute("type", QString::number(c->timeline_info.media->get_type()));
                                    switch (c->timeline_info.media->get_type()) {
                                    case MEDIA_TYPE_FOOTAGE:
                                        stream.writeAttribute("media", QString::number(c->timeline_info.media->get_object<Footage>()->save_id));
                                        stream.writeAttribute("stream", QString::number(c->timeline_info.media_stream));
                                        break;
                                    case MEDIA_TYPE_SEQUENCE:
                                        stream.writeAttribute("sequence", QString::number(c->timeline_info.media->get_object<Sequence>()->save_id));
                                        break;
                                    }
                                }

                                stream.writeStartElement("linked"); // linked
                                for (int k=0;k<c->linked.size();k++) {
                                    stream.writeStartElement("link"); // link
                                    stream.writeAttribute("id", QString::number(c->linked.at(k)));
                                    stream.writeEndElement(); // link
                                }
                                stream.writeEndElement(); // linked

                                for (int k=0;k<c->effects.size();k++) {
                                    stream.writeStartElement("effect"); // effect
                                    c->effects.at(k)->save(stream);
                                    stream.writeEndElement(); // effect
                                }

                                stream.writeEndElement(); // clip
                            }
                        }
                        for (int j=0;j<s->markers.size();j++) {
                            stream.writeStartElement("marker");
                            stream.writeAttribute("frame", QString::number(s->markers.at(j).frame));
                            stream.writeAttribute("name", s->markers.at(j).name);
                            stream.writeEndElement();
                        }
                        stream.writeEndElement();
                    }
                }
            }
        }

        if (mda->get_type() == MEDIA_TYPE_FOLDER) {
            save_folder(stream, type, set_ids_only, item);
        }
    }
}

void Project::save_project(bool autorecovery) {
    folder_id = 1;
    media_id = 1;
    sequence_id = 1;

    QFile file(autorecovery ? autorecovery_filename : project_url);
    if (!file.open(QIODevice::WriteOnly/* | QIODevice::Text*/)) {
        qCritical() << "Could not open file";
        return;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument(); // doc

    stream.writeStartElement("project"); // project

    stream.writeTextElement("version", QString::number(SAVE_VERSION));

    stream.writeTextElement("url", project_url);
    proj_dir = QFileInfo(project_url).absoluteDir();

    save_folder(stream, MEDIA_TYPE_FOLDER, true);

    stream.writeStartElement("folders"); // folders
    save_folder(stream, MEDIA_TYPE_FOLDER, false);
    stream.writeEndElement(); // folders

    stream.writeStartElement("media"); // media
    save_folder(stream, MEDIA_TYPE_FOOTAGE, false);
    stream.writeEndElement(); // media

    save_folder(stream, MEDIA_TYPE_SEQUENCE, true);

    stream.writeStartElement("sequences"); // sequences
    save_folder(stream, MEDIA_TYPE_SEQUENCE, false);
    stream.writeEndElement();// sequences

    stream.writeEndElement(); // project

    stream.writeEndDocument(); // doc

    file.close();

    if (!autorecovery) {
        add_recent_project(project_url);
        global::mainWindow->setWindowModified(false);
    }
}

void Project::update_view_type() {
    tree_view->setVisible(e_config.project_view_type == PROJECT_VIEW_TREE);
    icon_view_container->setVisible(e_config.project_view_type == PROJECT_VIEW_ICON);

    switch (e_config.project_view_type) {
    case PROJECT_VIEW_TREE:
        sources_common->view = tree_view;
        break;
    case PROJECT_VIEW_ICON:
        sources_common->view = icon_view;
        break;
    }
}

void Project::set_icon_view() {
    e_config.project_view_type = PROJECT_VIEW_ICON;
    update_view_type();
}

void Project::set_tree_view() {
    e_config.project_view_type = PROJECT_VIEW_TREE;
    update_view_type();
}

void Project::save_recent_projects() {
    // save to file
    QFile f(recent_proj_file);
    if (f.open(QFile::WriteOnly | QFile::Truncate | QFile::Text)) {
        QTextStream out(&f);
        for (int i=0;i<recent_projects.size();i++) {
            if (i > 0) {
                out << "\n";
            }
            out << recent_projects.at(i);
        }
        f.close();
    } else {
        qWarning() << "Could not save recent projects";
    }
}

void Project::clear_recent_projects() {
    recent_projects.clear();
    save_recent_projects();
}

void Project::set_icon_view_size(int s) {
    icon_view->setIconSize(QSize(s, s));
}

void Project::set_up_dir_enabled() {
    directory_up->setEnabled(icon_view->rootIndex().isValid());
}

void Project::go_up_dir() {
    icon_view->setRootIndex(icon_view->rootIndex().parent());
    set_up_dir_enabled();
}

void Project::make_new_menu() {
    QMenu new_menu(this);
    global::mainWindow->make_new_menu(&new_menu);
    new_menu.exec(QCursor::pos());
}

void Project::add_recent_project(QString url) {
    bool found = false;
    for (int i=0;i<recent_projects.size();i++) {
        if (url == recent_projects.at(i)) {
            found = true;
            recent_projects.move(i, 0);
            break;
        }
    }
    if (!found) {
        recent_projects.insert(0, url);
        if (recent_projects.size() > MAXIMUM_RECENT_PROJECTS) {
            recent_projects.removeLast();
        }
    }
    save_recent_projects();
}

/**
 * @brief           Get a Media item that was imported
 * @param index     Location in store
 * @return          ptr or null
 */
MediaPtr Project::getImportedMedia(const int index)
{
    MediaPtr item;

    if (last_imported_media.size() >= index){
        item = last_imported_media.at(index);
    } else {
        qWarning() << "No Media item at location" << index;
    }

    return item;
}

/**
 * @brief Get the size of the Import media list
 * @return  integer
 */
int Project::getMediaSize() const {
    return last_imported_media.size();
}

void Project::list_all_sequences_worker(QVector<MediaPtr>& list, MediaPtr parent) {
    for (int i=0; i<project_model.childCount(parent); i++) {
        MediaPtr item = project_model.child(i, parent);
        if (item != nullptr) {
            switch (item->get_type()) {
            case MEDIA_TYPE_SEQUENCE:
                list.append(item);
                break;
            case MEDIA_TYPE_FOLDER:
                list_all_sequences_worker(list, item);
                break;
            default:
                qWarning() << "Unknown media type" << item->get_type();
                break;
            }
        } else {
            //TODO:
        }
    }//for
}

QVector<MediaPtr> Project::list_all_project_sequences() {
    QVector<MediaPtr> list;
    list_all_sequences_worker(list, nullptr);
    return list;
}

QModelIndexList Project::get_current_selected() {
    if (e_config.project_view_type == PROJECT_VIEW_TREE) {
        return e_panel_project->tree_view->selectionModel()->selectedRows();
    }
    return e_panel_project->icon_view->selectionModel()->selectedIndexes();
}


MediaThrobber::MediaThrobber(MediaPtr i) :
    pixmap(":/icons/throbber.png"),
    animation(0),
    item(i),
    animator(nullptr)
{
    animator = std::make_unique<QTimer>(this);
}

void MediaThrobber::start() {
    // set up throbber
    animation_update();
    animator->setInterval(THROBBER_INTERVAL);
    connect(animator.get(), SIGNAL(timeout()), this, SLOT(animation_update()));
    animator->start();
}

void MediaThrobber::animation_update() {
    if (animation == THROBBER_LIMIT) {
        animation = 0;
    }
    project_model.set_icon(item, QIcon(pixmap.copy(THROBBER_SIZE*animation, 0, THROBBER_SIZE, THROBBER_SIZE)));
    animation++;
}

void MediaThrobber::stop(const int icon_type, const bool replace) {
    if (animator.get() != nullptr) {
        animator->stop();
    }

    QIcon icon;
    switch (icon_type) {
    case ICON_TYPE_VIDEO:
        icon.addFile(":/icons/videosource.png");
        break;
    case ICON_TYPE_AUDIO:
        icon.addFile(":/icons/audiosource.png");
        break;
    case ICON_TYPE_IMAGE:
        icon.addFile(":/icons/imagesource.png");
        break;
    case ICON_TYPE_ERROR:
        icon.addFile(":/icons/error.png");
        break;
    default:
        //TODO:
        break;
    }//switch
    project_model.set_icon(item,icon);

    // refresh all clips
    QVector<MediaPtr> sequences = e_panel_project->list_all_project_sequences();
    for (MediaPtr sqn : sequences) {
        SequencePtr s = sqn->get_object<Sequence>();
        for (ClipPtr clp: s->clips) {
            if (clp != nullptr) {
                clp->refresh();
            }
        }
    }

    // redraw clips
    update_ui(replace);

    e_panel_project->tree_view->viewport()->update();
    item->throbber = nullptr;
    deleteLater();
}
