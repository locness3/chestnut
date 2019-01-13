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
#ifndef TIMELINE_H
#define TIMELINE_H

#include <QDockWidget>
#include <QVector>
#include <QTime>
#include <QPushButton>
#include <QScrollBar>


#include "ui/timelinetools.h"
#include "project/selection.h"
#include "project/sequence.h"
#include "project/media.h"

#define TRACK_DEFAULT_HEIGHT 40

#define ADD_OBJ_TITLE 0
#define ADD_OBJ_SOLID 1
#define ADD_OBJ_BARS 2
#define ADD_OBJ_TONE 3
#define ADD_OBJ_NOISE 4
#define ADD_OBJ_AUDIO 5

class SourceTable;
class ViewerWidget;
class ComboAction;
class Effect;
class Transition;
class TimelineHeader;
class TimelineWidget;
class ResizableScrollBar;
class AudioMonitor;
struct EffectMeta;

class Clip;
class Footage;
struct FootageStream;

long refactor_frame_number(long framenumber, double source_frame_rate, double target_frame_rate);
int getScreenPointFromFrame(double zoom, long frame);
long getFrameFromScreenPoint(double zoom, int x);
bool selection_contains_transition(const Selection& s, ClipPtr c, int type);
void move_clip(ComboAction *ca, ClipPtr c, long iin, long iout, long iclip_in, int itrack, bool verify_transitions = true, bool relative = false);
void ripple_clips(ComboAction *ca, SequencePtr  s, long point, long length, const QVector<int>& ignore = QVector<int>());

struct Ghost {
    int clip = 0;
    long in = 0;
    long out = 0;
    int track = 0;
    long clip_in = 0;

    long old_in = 0;
    long old_out = 0;
    int old_track = 0;
    long old_clip_in = 0;

	// importing variables
    MediaPtr media = nullptr;
    int media_stream = 0;

	// other variables
    long ghost_length = 0;
    long media_length = 0;
    bool trim_in = false;
    bool trimming = false;

	// transition trimming
    Transition* transition = nullptr;
};

class Timeline : public QDockWidget
{
	Q_OBJECT
public:
	explicit Timeline(QWidget *parent = 0);
	~Timeline();

	bool focused();
	void set_zoom(bool in);
	void copy(bool del);
	void paste(bool insert);
    ClipPtr split_clip(ComboAction* ca, int p, long frame);
    ClipPtr split_clip(ComboAction* ca, int p, long frame, long post_in);
	bool split_selection(ComboAction* ca);
	bool split_all_clips_at_point(ComboAction *ca, long point);
	bool split_clip_and_relink(ComboAction* ca, int clip, long frame, bool relink);
	void clean_up_selections(QVector<Selection>& areas);
	void deselect_area(long in, long out, int track);
	void delete_areas_and_relink(ComboAction *ca, QVector<Selection>& areas);
    void relink_clips_using_ids(QVector<int>& old_clips, QVector<ClipPtr>& new_clips);
	void update_sequence();
	void increase_track_height();
	void decrease_track_height();
	void add_transition();
	QVector<int> get_tracks_of_linked_clips(int i);
	bool has_clip_been_split(int c);
	void ripple_to_in_point(bool in, bool ripple);
	void delete_in_out(bool ripple);
	void previous_cut();
	void next_cut();

    void create_ghosts_from_media(SequencePtr seq, long entry_point, QVector<MediaPtr> &media_list);
    void add_clips_from_ghosts(ComboAction *ca, SequencePtr s);

	int getTimelineScreenPointFromFrame(long frame);
	long getTimelineFrameFromScreenPoint(int x);
	int getDisplayScreenPointFromFrame(long frame);
	long getDisplayFrameFromScreenPoint(int x);

	int get_snap_range();
	bool snap_to_point(long point, long* l);
	bool snap_to_timeline(long* l, bool use_playhead, bool use_markers, bool use_workarea);
	void set_marker();

	// shared information
	int tool;
	long cursor_frame;
	int cursor_track;
	double zoom;
	bool zoom_just_changed;
	long drag_frame_start;
	int drag_track_start;
	void update_effect_controls();
	bool showing_all;
	double old_zoom;

	QVector<int> video_track_heights;
	QVector<int> audio_track_heights;
	int get_track_height_size(bool video);
	int calculate_track_height(int track, int height);

	// snapping
	bool snapping;
	bool snapped;
	long snap_point;

	// selecting functions
	bool selecting;
	int selection_offset;
    bool is_clip_selected(ClipPtr clip, bool containing);
	void delete_selection(QVector<Selection> &selections, bool ripple);
	void select_all();
	bool rect_select_init;
	bool rect_select_proc;
	int rect_select_x;
	int rect_select_y;
	int rect_select_w;
	int rect_select_h;

	// moving
	bool moving_init;
	bool moving_proc;
	QVector<Ghost> ghosts;
	bool video_ghosts;
	bool audio_ghosts;
	bool move_insert;

	// trimming
	int trim_target;
	bool trim_in_point;
	int transition_select;

	// splitting
	bool splitting;
	QVector<int> split_tracks;
	QVector<int> split_cache;

	// importing
	bool importing;
	bool importing_files;

	// creating variables
	bool creating;
	int creating_object;

	// transition variables
	bool transition_tool_init;
	bool transition_tool_proc;
	int transition_tool_pre_clip;
	int transition_tool_post_clip;
	int transition_tool_type;
	const EffectMeta* transition_tool_meta;
	int transition_tool_side;

	// hand tool variables
	bool hand_moving;
	int drag_x_start;
	int drag_y_start;

	bool block_repaints;

	TimelineHeader* headers;
	AudioMonitor* audio_monitor;
	ResizableScrollBar* horizontalScrollBar;

	QPushButton* toolArrowButton;
	QPushButton* toolEditButton;
	QPushButton* toolRippleButton;
	QPushButton* toolRazorButton;
	QPushButton* toolSlipButton;
	QPushButton* toolSlideButton;
	QPushButton* toolHandButton;
	QPushButton* toolTransitionButton;
	QPushButton* snappingButton;

	void scroll_to_frame(long frame);

	void resizeEvent(QResizeEvent *event);
public slots:
	void repaint_timeline();
	void toggle_show_all();
	void deselect();
	void toggle_links();
	void split_at_playhead();

private slots:
	void zoom_in();
	void zoom_out();
	void snapping_clicked(bool checked);
	void add_btn_click();
	void add_menu_item(QAction*);
	void setScroll(int);
	void record_btn_click();
	void transition_tool_click();
	void transition_menu_select(QAction*);
	void resize_move(double d);
	void set_tool();

private:
	void set_zoom_value(double v);
	QVector<QPushButton*> tool_buttons;
	void decheck_tool_buttons(QObject* sender);
	void set_tool(int tool);
	long last_frame;
	int scroll;
	void set_sb_max();

	void setup_ui();

	int default_track_height;

	QWidget* timeline_area;
	TimelineWidget* video_area;
	TimelineWidget* audio_area;
	QWidget* editAreas;
	QScrollBar* videoScrollbar;
	QScrollBar* audioScrollbar;
	QPushButton* zoomInButton;
	QPushButton* zoomOutButton;
	QPushButton* recordButton;
	QPushButton* addButton;
};

#endif // TIMELINE_H
