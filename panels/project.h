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
#ifndef PROJECT_H
#define PROJECT_H

#include <QDockWidget>
#include <QVector>
#include <QTimer>
#include <QDir>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QSortFilterProxyModel>

#include "project/projectmodel.h"
#include "project/sequence.h"
#include "project/media.h"

class Footage;

class Clip;
class Timeline;
class Viewer;
class SourceTable;
class ComboAction;
class SourceIconView;
class QPushButton;
class SourcesCommon;

#define LOAD_TYPE_VERSION 69
#define LOAD_TYPE_URL 70

extern QString autorecovery_filename;
extern QString project_url;
extern QStringList recent_projects;
extern QString recent_proj_file;

extern ProjectModel project_model;

SequencePtr  create_sequence_from_media(QVector<MediaPtr > &media_list);

QString get_channel_layout_name(int channels, uint64_t layout);
QString get_interlacing_name(int interlacing);

class Project : public QDockWidget {
	Q_OBJECT
public:
	explicit Project(QWidget *parent = 0);
    virtual ~Project();
	bool is_focused();
	void clear();
    MediaPtr new_sequence(ComboAction *ca, SequencePtr  s, bool open, MediaPtr parent);
	QString get_next_sequence_name(QString start = 0);
    void process_file_list(QStringList& files, bool recursive = false, MediaPtr replace = nullptr, MediaPtr parent = nullptr);
    void replace_media(MediaPtr item, QString filename);
    MediaPtr get_selected_folder();
    bool reveal_media(MediaPtr media, QModelIndex parent = QModelIndex());
	void add_recent_project(QString url);

	void new_project();
	void load_project(bool autorecovery);
	void save_project(bool autorecovery);

    MediaPtr new_folder(QString name);
    MediaPtr item_to_media(const QModelIndex& index);

	void save_recent_projects();

    QVector<MediaPtr> list_all_project_sequences();

	SourceTable* tree_view;
	SourceIconView* icon_view;
	SourcesCommon* sources_common;

	QSortFilterProxyModel* sorter;

    QVector<MediaPtr> last_imported_media; //TODO: feel a map is required so ProjectModel can use the key in QModelIndex

	QModelIndexList get_current_selected();

    void start_preview_generator(MediaPtr item, bool replacing);
    void get_all_media_from_table(QVector<MediaPtr> &items, QVector<MediaPtr> &list, int type = -1);

	QWidget* toolbar_widget;
public slots:
	void import_dialog();
	void delete_selected_media();
	void duplicate_selected();
	void delete_clips_using_selected_media();
	void replace_selected_file();
	void replace_clip_media();
	void open_properties();
private:
	void save_folder(QXmlStreamWriter& stream, int type, bool set_ids_only, const QModelIndex &parent = QModelIndex());
	int folder_id;
	int media_id;
	int sequence_id;
    void list_all_sequences_worker(QVector<MediaPtr > *list, MediaPtr parent);
	QString get_file_name_from_path(const QString &path);
	QDir proj_dir;
	QWidget* icon_view_container;
	QPushButton* directory_up;
private slots:
	void update_view_type();
	void set_icon_view();
	void set_tree_view();
	void clear_recent_projects();
	void set_icon_view_size(int);
	void set_up_dir_enabled();
	void go_up_dir();
	void make_new_menu();
};

class MediaThrobber : public QObject {
	Q_OBJECT
public:
    MediaThrobber(MediaPtr);
public slots:
	void start();
	void stop(int, bool replace);
private slots:
	void animation_update();
private:
	QPixmap pixmap;
	int animation;
    MediaPtr item;
	QTimer* animator;
};

#endif // PROJECT_H
