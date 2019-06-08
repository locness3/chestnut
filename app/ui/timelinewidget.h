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
#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QTimer>
#include <QWidget>
#include <QScrollBar>
#include <QPainter>

#include "project/sequence.h"
#include "project/media.h"
#include "project/undo.h"

constexpr int GHOST_THICKNESS = 2;
constexpr int CLIP_TEXT_PADDING = 3;

constexpr int TRACK_MIN_HEIGHT = 30;
constexpr int TRACK_HEIGHT_INCREMENT = 10;


enum class TimelineToolType {
  POINTER = 0,
  EDIT,
  RAZOR,
  RIPPLE,
  ROLLING,
  SLIP,
  SLIDE,
  HAND,
  ZOOM,
  MENU,
  TRANSITION
};

class Clip;
class FootageStream;
class Timeline;
class TimelineAction;
class SetSelectionsCommand;
struct Ghost;

bool same_sign(int a, int b);
void draw_waveform(ClipPtr& clip, const project::FootageStreamPtr& ms, const long media_length, QPainter& p,
                   const QRect& clip_rect, const int waveform_start, const int waveform_limit, const double zoom);

class TimelineWidget : public QWidget {
    Q_OBJECT
  public:
    /**
     * @brief TimelineWidget
     * @param displays_video    true==this widget is displaying video tracks
     * @param parent
     */
    TimelineWidget(const bool displays_video, QWidget *parent = nullptr);

    TimelineWidget(const TimelineWidget& ) = delete;
    TimelineWidget& operator=(const TimelineWidget&) = delete;

    QScrollBar* scrollBar{nullptr};
    bool bottom_align{false};
  protected:
    void paintEvent(QPaintEvent*) override;

    void resizeEvent(QResizeEvent *event) override;

    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent* event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;
  private:
    int track_resize_mouse_cache{};
    int track_resize_old_value{};
    bool track_resizing{false};
    int track_target{};

    QVector<ClipPtr> pre_clips;
    QVector<ClipPtr> post_clips;

    SequencePtr self_created_sequence;

    // used for "right click ripple"
    int64_t rc_ripple_min{};
    int64_t rc_ripple_max{};
    MediaPtr rc_reveal_media;

    QTimer tooltip_timer;
    ClipWPtr tooltip_clip_;

    int scroll{};

    SetSelectionsCommand* selection_command{};
    bool displays_video_;

    void init_ghosts();
    void update_ghosts(const QPoint& mouse_pos, bool lock_frame);
    bool is_track_visible(int track);
    int getTrackFromScreenPoint(int y);
    int getScreenPointFromTrack(int track);
    int getClipIndexFromCoords(long frame, int track);

    ClipPtr getClipFromCoords(const long frame, const int track) const;
    bool splitClipEvent(const long frame, const QSet<int>& tracks);

    void mouseMoveSplitEvent(const bool alt_pressed, Timeline& time_line) const;
    
    void paintSplitEvent(QPainter& painter, Timeline& time_line);

    void mousePressCreatingEvent(Timeline& time_line);

    bool applyTransition(ComboAction* ca);

    
    void paintGhosts(QPainter& painter);
    
    void paintSelections(QPainter& painter, Timeline& time_line);

    void processMove(ComboAction* ca, const bool ctrl_pressed, const bool alt_pressed, QVector<ClipPtr>& moved, Timeline& time_line);

    /**
     * @brief         Clear an area around a clip before moving it and its linked clips
     * @param ca
     * @param c
     * @param g       The area to be cleared
     * @param front   true==clear in front of the clip, false==clear behind
     */
    void makeRoomForClipLinked(ComboAction& ca, const ClipPtr& c, const Ghost& g, const bool front);

    void rippleMove(ComboAction& ca, const Ghost& first_ghost);
    
    void duplicateClips(ComboAction& ca);

    void moveClipSetup(ComboAction& ca);

    void moveClips(ComboAction& ca, QVector<ClipPtr>& moved, const QVector<Ghost>& ghosts);

    void moveClip(ComboAction& ca, const ClipPtr& c, const Ghost& g);

    /**
     * @brief             Moving the mouse across the timeline with the pointer/ripple/slip tools
     * @param pos
     * @param time_line
     * @param seq
     */
    void mousingOverEvent(const QPoint& pos, Timeline& time_line, const SequencePtr& seq);

    void setupMovement(Timeline& time_line, const SequencePtr& sqn);

    /**
     * @brief             Obtain a tracks lower and upper limits, vertically, in this area
     * @param track
     * @param time_line
     * @return            tuple<lower,upper> limits
     */
    std::tuple<int,int> trackVerticalLimits(const int track, Timeline& time_line) const;

    /**
     * @brief               Identify if the mouse cursor is in an area, vertically, where a transition could be painted
     * @param pos           Mouse position relative to this widgets dimensions
     * @param time_line
     * @param track_number
     * @return              true==in transition area
     */
    bool inTransitionArea(const QPoint& pos, Timeline& time_line, const int track_number) const;

    /*
     * Draw the features that enbody a track in the timeline
     */
    void paintTrack(QPainter& painter, const int track, const bool video);
    
public slots:
    void setScroll(int);

  private slots:
    void reveal_media();
    void right_click_ripple();
    void show_context_menu(const QPoint& pos);
    void toggle_autoscale();
    void tooltip_timer_timeout();
    void rename_clip();
    void show_stabilizer_diag();
    void open_sequence_properties();
};

#endif // TIMELINEWIDGET_H
