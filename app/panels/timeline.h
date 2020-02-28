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


#include "ui/timelinewidget.h"
#include "ui/markerdockwidget.h"
#include "project/selection.h"
#include "project/sequence.h"
#include "project/media.h"
#include "project/effect.h"
#include "ui/Forms/timelinetrackarea.h"
#include "ui/blankscrollarea.h"

constexpr int TRACK_DEFAULT_HEIGHT = 40;


enum class AddObjectType {
  TITLE = 0,
  SOLID,
  BARS,
  TONE,
  NOISE,
  AUDIO,
  ADJUSTMENT_LAYER
};


class SourceTable;
class ViewerWidget;
class ComboAction;
class Effect;
class Transition;
class TimelineHeader;
class ResizableScrollBar;
class AudioMonitor;

class Clip;
class FootageStream;

namespace chestnut::project
{
  class Footage;
}

int getScreenPointFromFrame(double zoom, int64_t frame);
long getFrameFromScreenPoint(double zoom, int x);
bool selection_contains_transition(const Selection& s, ClipPtr c, int type);
void ripple_clips(ComboAction *ca, chestnut::project::SequencePtr  s, long point, long length,
                  const QVector<int>& ignore = QVector<int>());

struct Ghost {
    ClipWPtr clip_;
    long in = 0;
    long out = 0;
    int track = 0;
    long clip_in = 0;

    long old_in = 0;
    long old_out = 0;
    int old_track = 0;
    long old_clip_in = 0;

    // importing variables
    chestnut::project::MediaPtr media = nullptr;
    int media_stream = 0;

    // other variables
    long ghost_length = 0;
    long media_length = 0;
    bool trim_in = false;
    bool trimming = false;

    // transition trimming
    TransitionWPtr transition;
};


//FIXME: far too many methods and members
class Timeline : public QDockWidget, public ui::MarkerDockWidget
{
    Q_OBJECT
  public:
    explicit Timeline(QWidget *parent = nullptr);
    virtual ~Timeline() override = default;

    Timeline(const Timeline& ) = delete;
    Timeline(const Timeline&& ) = delete;
    Timeline& operator=(const Timeline&) = delete;
    Timeline& operator=(const Timeline&&) = delete;

    bool setSequence(const chestnut::project::SequencePtr& seq);
    bool focused();
    void set_zoom(bool in);
    void copy(bool del);
    void paste(bool insert);
    bool split_all_clips_at_point(ComboAction *ca, const long point);
    void clean_up_selections(QVector<Selection>& areas);
    void deselect_area(long in, long out, int track);
    void delete_areas(ComboAction *ca, QVector<Selection>& areas);
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
    /**
     * @brief Nudge selected clips by a specified amount + direction.
     * @param pos
     */
    void nudgeClips(const int pos);

    void createGhostsFromMedia(chestnut::project::SequencePtr& seq, const long entry_point,
                               QVector<chestnut::project::MediaPtr> &media_list);
    void addClipsFromGhosts(ComboAction *ca, const chestnut::project::SequencePtr& seq);

    int getTimelineScreenPointFromFrame(int64_t frame);
    long getTimelineFrameFromScreenPoint(int x);
    int getDisplayScreenPointFromFrame(long frame);
    long getDisplayFrameFromScreenPoint(int x);

    int get_snap_range();
    bool snap_to_point(long point, long* l);
    bool snap_to_timeline(long* l, bool use_playhead, bool use_markers, bool use_workarea);
    virtual void setMarker() const override;

    void update_effect_controls();

    int get_track_height_size(bool video);
    int calculate_track_height(const int track, const int height=-1);

    void delete_selection(QVector<Selection> &selections, bool ripple);
    void select_all();

    void scroll_to_frame(int64_t frame);
    void select_from_playhead();

    // shared information
    TimelineToolType tool;
    long cursor_frame;
    int cursor_track;
    double zoom;
    bool zoom_just_changed;
    long drag_frame_start{};
    int drag_track_start{};
    bool showing_all;
    double old_zoom{};

    QVector<int> video_track_heights;
    QVector<int> audio_track_heights;

    // snapping
    bool snapping;
    bool snapped;
    long snap_point;

    // selecting functions
    bool selecting;
    int selection_offset{};
    bool rect_select_init;
    bool rect_select_proc;
    int rect_select_x{};
    int rect_select_y{};
    int rect_select_w{};
    int rect_select_h{};

    // moving
    bool moving_init;
    bool moving_proc;
    QVector<Ghost> ghosts;
    bool video_ghosts{};
    bool audio_ghosts{};
    bool move_insert;

    // FIXME: these variables are only ever used in timelinewidget
    // trimming
    ClipWPtr trim_target;
    bool trim_in_point;
    int transition_select{};

    // splitting
    bool splitting;
    QSet<int> split_tracks;

    // importing
    bool importing;
    bool importing_files;

    // creating variables
    bool creating;
    AddObjectType creating_object;

    // transition variables
    bool transition_tool_init;
    bool transition_tool_proc;
    ClipPtr transition_tool_pre_clip{nullptr};
    ClipPtr transition_tool_post_clip{nullptr};
    int transition_tool_type{};
    EffectMeta transition_tool_meta{};
    int transition_tool_side{};

    // hand tool variables
    bool hand_moving;
    int drag_x_start{};
    int drag_y_start{};

    bool block_repaints;

    TimelineHeader* headers{};
    AudioMonitor* audio_monitor{};
    ResizableScrollBar* horizontalScrollBar{};

    QPushButton* toolArrowButton{};
    QPushButton* toolEditButton{};
    QPushButton* toolRippleButton{};
    QPushButton* toolRazorButton{};
    QPushButton* toolSlipButton{};
    QPushButton* toolSlideButton{};
    QPushButton* toolHandButton{};
    QPushButton* toolTransitionButton{};
    QPushButton* snappingButton{};

  protected:
    void resizeEvent(QResizeEvent *event) override;
  public slots:
    void repaint_timeline();
    void toggle_show_all();
    void deselect();
    void split_at_playhead();
    void unlinkClips();
    void linkClips();

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
    void trackEnabled(const bool enabled, const int track_number);
    void trackLocked(const bool locked, const int track_number);

  signals:
    void newSequenceLoaded(const chestnut::project::SequencePtr& new_sequence);

  private:
    friend class TimelineTest;
    QVector<QPushButton*> tool_buttons;
    long last_frame;
    int scroll;

    int default_track_height;

    QWidget* timeline_area{};
    TimelineWidget* video_area{};
    TimelineWidget* audio_area{};
    QWidget* editAreas{};
    QScrollBar* videoScrollbar{};
    ui::BlankScrollArea* video_scroll_{};
    ui::BlankScrollArea* audio_scroll_{};
    QScrollBar* audioScrollbar{};
    QPushButton* zoomInButton{};
    QPushButton* zoomOutButton{};
    QPushButton* recordButton{};
    QPushButton* addButton{};
    chestnut::project::SequencePtr sequence_{};

    TimelineTrackArea* header_track_area_{nullptr};
    TimelineTrackArea* video_track_area_{nullptr};
    TimelineTrackArea* audio_track_area_{nullptr};

    void set_zoom_value(double v);
    void decheck_tool_buttons(QObject* sender);
    void set_tool(int tool);
    void set_sb_max();
    void setup_ui();
    std::vector<ClipPtr> selectedClips();
    std::vector<Selection> selections();
    void pasteClip(const QVector<project::SequenceItemPtr>& items, const bool insert, const chestnut::project::SequencePtr& seq);
    ClipPtr split_clip(ComboAction& ca, const ClipPtr& pre_clip, const long frame) const;
    void clipLinkage(const bool link);
    void updateTrackAreas();
    QWidget* createToolButtonsWidget(QWidget* parent);
    QWidget* createVideoAreaWidget(QWidget* parent);
    QWidget* createAudioAreaWidget(QWidget* parent);
    void changeTrackHeight(QVector<int>& tracks, const int value) const;
};

#endif // TIMELINE_H
