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
#ifndef VIEWER_H
#define VIEWER_H

#include <QDockWidget>
#include <QTimer>
#include <QIcon>
#include <QPushButton>
#include <QLabel>


#include "project/sequence.h"
#include "project/media.h"
#include "ui/markerdockwidget.h"

class Timeline;
class ViewerWidget;

class TimelineHeader;
class ResizableScrollBar;
class ViewerContainer;
class LabelSlider;

bool frame_rate_is_droppable(const double rate);
long timecode_to_frame(const QString& s, int view, double frame_rate);
QString frame_to_timecode(long frame, const int view, const double frame_rate);

class Viewer : public QDockWidget, public ui::MarkerDockWidget
{
    Q_OBJECT

  public:
    explicit Viewer(QWidget *parent = nullptr);

    bool is_focused();
    bool is_main_sequence();
    void set_main_sequence();
    void set_media(const MediaPtr& m);
    void reset();
    void compose();
    void set_playpause_icon(bool play);
    void update_playhead_timecode(long p);
    void update_end_timecode();
    void update_header_zoom();
    void update_viewer();
    void clear_in();
    void clear_out();
    void clear_inout_point();
    void toggle_enable_inout();
    void set_in_point();
    void set_out_point();
    void set_zoom(bool in);
    void set_panel_name(const QString& name);

    // playback functions
    void seek(long p);
    void play();
    void pause();
    bool playing;
    long playhead_start;
    qint64 start_msecs;
    QTimer playback_updater;
    bool just_played;

    void cue_recording(long start, long end, int track);
    void uncue_recording();
    bool is_recording_cued();
    long recording_start;
    long recording_end;
    int recording_track;

    void reset_all_audio();
    void update_parents(bool reload_fx = false);

    ViewerWidget* viewer_widget;

    void resizeEvent(QResizeEvent *event);

    MediaPtr getMedia();
    SequencePtr getSequence();
    /**
     * Create a new marker of an object in the widget
     */
    virtual void setMarker() const override;

  public slots:
    void play_wake();
    void go_to_start();
    void go_to_in();
    void previous_frame();
    void toggle_play();
    void next_frame();
    void go_to_out();
    void go_to_end();
    void close_media();

  private slots:
    void update_playhead();
    void timer_update();
    void recording_flasher_update();
    void resize_move(double d);

  private:
    friend class ViewerTest;

    void update_window_title();
    void clean_created_seq();
    void set_sequence(bool main, SequencePtr s);
    void set_zoom_value(double d);
    void set_sb_max();

    long get_seq_in();
    long get_seq_out();

    void setup_ui();

    SequencePtr seq;
    MediaPtr media;

    bool main_sequence;
    bool created_sequence;
    long cached_end_frame = 0;
    QString panel_name;
    double minimum_zoom;

    QIcon playIcon;

    TimelineHeader* headers;
    ResizableScrollBar* horizontal_bar;
    ViewerContainer* viewer_container;
    LabelSlider* currentTimecode;
    QLabel* endTimecode;

    QPushButton* btnSkipToStart;
    QPushButton* btnRewind;
    QPushButton* btnPlay;
    QPushButton* btnFastForward;
    QPushButton* btnSkipToEnd;

    bool cue_recording_internal;
    QTimer recording_flasher;

    long previous_playhead;
    SequencePtr createFootageSequence(const MediaPtr& mda) const;
};

#endif // VIEWER_H
