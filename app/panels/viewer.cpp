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
#include "viewer.h"

#include <QtMath>
#include <QAudioOutput>
#include <QPainter>
#include <QStringList>
#include <QTimer>
#include <QHBoxLayout>
#include <QPushButton>

#include "playback/audio.h"
#include "timeline.h"
#include "panels/panelmanager.h"
#include "io/config.h"
#include "project/sequence.h"
#include "project/clip.h"
#include "project/footage.h"
#include "project/media.h"
#include "project/undo.h"
#include "ui/audiomonitor.h"
#include "playback/playback.h"
#include "ui/viewerwidget.h"
#include "ui/viewercontainer.h"
#include "ui/labelslider.h"
#include "ui/timelineheader.h"
#include "ui/resizablescrollbar.h"
#include "debug.h"


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

using panels::PanelManager;

namespace {
  const int FRAMES_IN_ONE_MINUTE = 1798; // 1800 - 2
  const int FRAMES_IN_TEN_MINUTES = 17978; // (FRAMES_IN_ONE_MINUTE * 10) - 2

  const int RECORD_FLASHER_INTERVAL = 500;

  const int MEDIA_WIDTH = 1980;
  const int MEDIA_HEIGHT = 1080;
  const int MEDIA_AUDIO_FREQUENCY = 48000;
  const int MEDIA_FRAME_RATE = 30;

  const char* const PANEL_NAME = "Viewer";
  const char* const PANEL_TITLE_FORMAT = "%1: %2";

  const QColor PAUSE_COLOR(128,192,128); // RGB
}

Viewer::Viewer(QWidget *parent) :
  QDockWidget(parent),
  playing(false),
  just_played(false),
  seq(nullptr),
  media(nullptr),
  created_sequence(false),
  minimum_zoom(1.0),
  cue_recording_internal(false)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  setup_ui();

  headers->viewer = this;
  headers->snapping = false;
  headers->show_text(false);
  viewer_container->viewer = this;
  viewer_widget = viewer_container->child;
  viewer_widget->viewer = this;
  set_media(nullptr);

  currentTimecode->setEnabled(false);
  currentTimecode->set_minimum_value(0);
  currentTimecode->set_default_value(qSNaN());
  currentTimecode->set_value(0, false);
  currentTimecode->set_display_type(SliderType::FRAMENUMBER);
  connect(currentTimecode, SIGNAL(valueChanged()), this, SLOT(update_playhead()));

  recording_flasher.setInterval(RECORD_FLASHER_INTERVAL);

  connect(&playback_updater, SIGNAL(timeout()), this, SLOT(timer_update()));
  connect(&recording_flasher, SIGNAL(timeout()), this, SLOT(recording_flasher_update()));
  connect(horizontal_bar, SIGNAL(valueChanged(int)), headers, SLOT(set_scroll(int)));
  connect(horizontal_bar, SIGNAL(valueChanged(int)), viewer_widget, SLOT(set_waveform_scroll(int)));
  connect(horizontal_bar, SIGNAL(resize_move(double)), this, SLOT(resize_move(double)));

  update_playhead_timecode(0);
  update_end_timecode();
}

Viewer::~Viewer() {}

bool Viewer::is_focused() {
  return headers->hasFocus()
      || viewer_widget->hasFocus()
      || btnSkipToStart->hasFocus()
      || btnRewind->hasFocus()
      || btnPlay->hasFocus()
      || btnFastForward->hasFocus()
      || btnSkipToEnd->hasFocus();
}

bool Viewer::is_main_sequence() {
  return main_sequence;
}

void Viewer::set_main_sequence() {
  clean_created_seq();
  set_sequence(true, global::sequence);
}

void Viewer::reset_all_audio() {
  // reset all clip audio
  if (seq != nullptr) {
    audio_ibuffer_frame = seq->playhead_;
    audio_ibuffer_timecode = (double) audio_ibuffer_frame / seq->frameRate();

    for (int i=0;i<seq->clips_.size();i++) {
      ClipPtr c = seq->clips_.at(i);
      if (c != nullptr) {
        c->resetAudio();
      }
    }
  }
  clear_audio_ibuffer();
}

long timecode_to_frame(const QString& s, int view, double frame_rate) {
  QList<QString> list = s.split(QRegExp("[:;]"));

  if (view == TIMECODE_FRAMES || (list.size() == 1 && view != TIMECODE_MILLISECONDS)) {
    return s.toLong();
  }

  int frRound = qRound(frame_rate);
  int hours, minutes, seconds, frames;

  if (view == TIMECODE_MILLISECONDS) {
    long milliseconds = s.toLong();

    hours = milliseconds/3600000;
    milliseconds -= (hours*3600000);
    minutes = milliseconds/60000;
    milliseconds -= (minutes*60000);
    seconds = milliseconds/1000;
    milliseconds -= (seconds*1000);
    frames = qRound64((milliseconds*0.001)*frame_rate);

    seconds = qRound64(seconds * frame_rate);
    minutes = qRound64(minutes * frame_rate * 60);
    hours = qRound64(hours * frame_rate * 3600);
  } else {
    hours = ((list.size() > 0) ? list.at(0).toInt() : 0) * frRound * 3600;
    minutes = ((list.size() > 1) ? list.at(1).toInt() : 0) * frRound * 60;
    seconds = ((list.size() > 2) ? list.at(2).toInt() : 0) * frRound;
    frames = (list.size() > 3) ? list.at(3).toInt() : 0;
  }

  int f = (frames + seconds + minutes + hours);

  if ((view == TIMECODE_DROP || view == TIMECODE_MILLISECONDS) && frame_rate_is_droppable(frame_rate)) {
    // return drop
    int d;
    int m;

    int dropFrames = round(frame_rate * .066666); //Number of frames to drop on the minute marks is the nearest integer to 6% of the framerate
    int framesPer10Minutes = round(frame_rate * 60 * 10); //Number of frames per ten minutes
    int framesPerMinute = (round(frame_rate)*60)-  dropFrames; //Number of frames per minute is the round of the framerate * 60 minus the number of dropped frames

    d = f / framesPer10Minutes;
    f -= dropFrames*9*d;

    m = f % framesPer10Minutes;

    if (m > dropFrames) {
      f -= (dropFrames * ((m - dropFrames) / framesPerMinute));
    }
  }

  // return non-drop
  return f;
}

QString frame_to_timecode(long f, int view, double frame_rate) {
  if (view == TIMECODE_FRAMES) {
    return QString::number(f);
  }

  // return timecode
  int hours = 0;
  int mins = 0;
  int secs = 0;
  int frames = 0;
  QString token = ":";

  if ((view == TIMECODE_DROP || view == TIMECODE_MILLISECONDS) && frame_rate_is_droppable(frame_rate)) {
    //CONVERT A FRAME NUMBER TO DROP FRAME TIMECODE
    //Code by David Heidelberger, adapted from Andrew Duncan, further adapted for Olive by Olive Team
    //Given an int called framenumber and a double called framerate
    //Framerate should be 29.97, 59.94, or 23.976, otherwise the calculations will be off.

    int d;
    int m;

    int dropFrames = round(frame_rate * .066666); //Number of frames to drop on the minute marks is the nearest integer to 6% of the framerate
    int framesPerHour = round(frame_rate*60*60); //Number of frqRound64ames in an hour
    int framesPer24Hours = framesPerHour*24; //Number of frames in a day - timecode rolls over after 24 hours
    int framesPer10Minutes = round(frame_rate * 60 * 10); //Number of frames per ten minutes
    int framesPerMinute = (round(frame_rate)*60)-  dropFrames; //Number of frames per minute is the round of the framerate * 60 minus the number of dropped frames

    //If framenumber is greater than 24 hrs, next operation will rollover clock
    f = f % framesPer24Hours; //% is the modulus operator, which returns a remainder. a % b = the remainder of a/b

    d = f / framesPer10Minutes; // \ means integer division, which is a/b without a remainder. Some languages you could use floor(a/b)
    m = f % framesPer10Minutes;

    //In the original post, the next line read m>1, which only worked for 29.97. Jean-Baptiste Mardelle correctly pointed out that m should be compared to dropFrames.
    if (m > dropFrames) {
      f = f + (dropFrames*9*d) + dropFrames * ((m - dropFrames) / framesPerMinute);
    } else {
      f = f + dropFrames*9*d;
    }

    int frRound = qRound(frame_rate);
    frames = f % frRound;
    secs = (f / frRound) % 60;
    mins = ((f / frRound) / 60) % 60;
    hours = (((f / frRound) / 60) / 60);

    token = ";";
  } else {
    // non-drop timecode

    int int_fps = qRound(frame_rate);
    hours = f / (3600 * int_fps);
    mins = f / (60*int_fps) % 60;
    secs = f/int_fps % 60;
    frames = f%int_fps;
  }
  if (view == TIMECODE_MILLISECONDS) {
    return QString::number((hours*3600000)+(mins*60000)+(secs*1000)+qCeil(frames*1000/frame_rate));
  }
  return QString(QString::number(hours).rightJustified(2, '0') +
                 ":" + QString::number(mins).rightJustified(2, '0') +
                 ":" + QString::number(secs).rightJustified(2, '0') +
                 token + QString::number(frames).rightJustified(2, '0')
                 );
}

bool frame_rate_is_droppable(float rate) {
  return (qFuzzyCompare(rate, 23.976f) || qFuzzyCompare(rate, 29.97f) || qFuzzyCompare(rate, 59.94f));
}

void Viewer::seek(long p) {
  if (seq == nullptr) {
      qWarning() << "Null Sequence instance";
      return;
  }
  pause();
  seq->playhead_ = p;
  bool update_fx = false;
  if (main_sequence) {
    PanelManager::timeLine().scroll_to_frame(p);
    PanelManager::fxControls().scroll_to_frame(p);
    if (e_config.seek_also_selects) {
      PanelManager::timeLine().select_from_playhead();
      update_fx = true;
    }
  }
  reset_all_audio();
  audio_scrub = true;
  update_parents(update_fx);
}

void Viewer::go_to_start() {
  if (seq != nullptr) seek(0);
}

void Viewer::go_to_end() {
  if (seq != nullptr) seek(seq->endFrame());
}

void Viewer::close_media() {
  set_media(nullptr);
}

void Viewer::go_to_in() {
  if (seq != nullptr) {
    if (seq->workarea_.using_ && seq->workarea_.enabled_) {
      seek(seq->workarea_.in_);
    } else {
      go_to_start();
    }
  }
}

void Viewer::previous_frame() {
  if (seq != nullptr && seq->playhead_ > 0) seek(seq->playhead_-1);
}

void Viewer::next_frame() {
  if (seq != nullptr) seek(seq->playhead_+1);
}

void Viewer::go_to_out() {
  if (seq != nullptr) {
    if (seq->workarea_.using_ && seq->workarea_.enabled_) {
      seek(seq->workarea_.out_);
    } else {
      go_to_end();
    }
  }
}

void Viewer::cue_recording(long start, long end, int track)
{
  recording_start = start;
  recording_end = end;
  recording_track = track;
  PanelManager::sequenceViewer().seek(recording_start); //FIXME:
  cue_recording_internal = true;
  recording_flasher.start();
}

void Viewer::uncue_recording() {
  cue_recording_internal = false;
  recording_flasher.stop();
  btnPlay->setStyleSheet(QString());
}

bool Viewer::is_recording_cued() {
  return cue_recording_internal;
}

void Viewer::toggle_play() {
  if (playing) {
    pause();
  } else {
    play();
  }
}

void Viewer::play()
{
  if (PanelManager::sequenceViewer().playing) {
    PanelManager::sequenceViewer().pause();
  }
  if (PanelManager::footageViewer().playing) {
    PanelManager::footageViewer().pause();
  }

  if (seq != nullptr) {
    if (!is_recording_cued()
        && seq->playhead_ >= get_seq_out()
        && (e_config.loop || !main_sequence)) {
      seek(get_seq_in());
    }

    reset_all_audio();
    if (is_recording_cued() && !start_recording()) {
      qCritical() << "Failed to record audio";
      return;
    }
    playhead_start = seq->playhead_;
    playing = true;
    just_played = true;
    set_playpause_icon(false);
    start_msecs = QDateTime::currentMSecsSinceEpoch();
    timer_update();
  }
}

void Viewer::play_wake() {
  if (just_played) {
    start_msecs = QDateTime::currentMSecsSinceEpoch();
    playback_updater.start();
    if (audio_thread != nullptr) audio_thread->notifyReceiver();
    just_played = false;
  }
}

void Viewer::pause() {
  playing = false;
  just_played = false;
  set_playpause_icon(true);
  playback_updater.stop();

  if (is_recording_cued()) {
    uncue_recording();

    if (recording) {
      stop_recording();

      // import audio
      QStringList file_list;
      file_list.append(get_recorded_audio_filename());
      PanelManager::projectViewer().process_file_list(file_list);

      // add it to the sequence
      auto clp = std::make_shared<Clip>(seq);
      auto mda = PanelManager::projectViewer().getImportedMedia(0);
      auto ftg = mda->object<Footage>();

      ftg->ready_lock.lock();

      clp->timeline_info.media = mda; // latest media
      clp->timeline_info.media_stream = 0;
      clp->timeline_info.in = recording_start;
      clp->timeline_info.out = recording_start + ftg->get_length_in_frames(seq->frameRate());
      clp->timeline_info.clip_in = 0;
      clp->timeline_info.track_ = recording_track;
      clp->timeline_info.color = PAUSE_COLOR;
      clp->timeline_info.name = mda->name();

      ftg->ready_lock.unlock();

      QVector<ClipPtr> add_clips;
      add_clips.append(clp);
      e_undo_stack.push(new AddClipCommand(seq, add_clips)); // add clip
    }
  }
}

void Viewer::update_playhead_timecode(long p) {
  currentTimecode->set_value(p, false);
}

void Viewer::update_end_timecode() {
  endTimecode->setText((seq == nullptr) ? frame_to_timecode(0, e_config.timecode_view, 30)
                                        : frame_to_timecode(seq->endFrame(), e_config.timecode_view, seq->frameRate()));
}

void Viewer::update_header_zoom()
{
  if (seq != nullptr) {
    long sequenceEndFrame = seq->endFrame();
    if (cached_end_frame != sequenceEndFrame) {
      minimum_zoom = (sequenceEndFrame > 0) ? ((double) headers->width() / (double) sequenceEndFrame) : 1;
      headers->update_zoom(qMax(headers->get_zoom(), minimum_zoom));
      set_sb_max();
      viewer_widget->waveform_zoom = headers->get_zoom();
    } else {
      headers->update();
    }
  }
}

void Viewer::update_parents(bool reload_fx) {
  if (main_sequence) {
    PanelManager::refreshPanels(reload_fx);
  } else {
    update_viewer();
  }
}

void Viewer::resizeEvent(QResizeEvent *) {
  if (seq != nullptr) {
    set_sb_max();
  }
}


MediaPtr Viewer::getMedia()
{
  return media;
}

SequencePtr Viewer::getSequence()
{
  return seq;
}

void Viewer::setMarker() const
{
  bool add_marker = !e_config.set_name_with_marker;
  QString marker_name;

  if (!add_marker) {
    std::tie(marker_name, add_marker) = this->getName();
  }

  if (add_marker) {
    // TODO: create a thumbnail for this
    e_undo_stack.push(new AddMarkerAction(media->object<Footage>(), seq->playhead_, marker_name));
  }
}

void Viewer::update_viewer() {
  update_header_zoom();
  viewer_widget->frame_update();
  if (seq != nullptr) {
    update_playhead_timecode(seq->playhead_);
  }
  update_end_timecode();
}

void Viewer::clear_in() {
  if (seq->workarea_.using_) {
    e_undo_stack.push(new SetTimelineInOutCommand(seq, true, 0, seq->workarea_.out_));
    update_parents();
  }
}

void Viewer::clear_out() {
  if (seq->workarea_.using_) {
    e_undo_stack.push(new SetTimelineInOutCommand(seq, true, seq->workarea_.in_, seq->endFrame()));
    update_parents();
  }
}

void Viewer::clear_inout_point() {
  if (seq->workarea_.using_) {
    e_undo_stack.push(new SetTimelineInOutCommand(seq, false, 0, 0));
    update_parents();
  }
}

void Viewer::toggle_enable_inout() {
  if (seq != nullptr && seq->workarea_.using_) {
    e_undo_stack.push(new SetBool(&seq->workarea_.enabled_, !seq->workarea_.enabled_));
    update_parents();
  }
}

void Viewer::set_in_point() {
  headers->set_in_point(seq->playhead_);
}

void Viewer::set_out_point() {
  headers->set_out_point(seq->playhead_);
}

void Viewer::set_zoom(bool in) {
  if (seq != nullptr) {
    set_zoom_value(in ? headers->get_zoom()*2 : qMax(minimum_zoom, headers->get_zoom()*0.5));
  }
}

void Viewer::set_panel_name(const QString &n) {
  panel_name = n;
  update_window_title();
}

void Viewer::update_window_title() {
  QString name;
  if (seq == nullptr) {
    name = tr("(none)");
  } else {
    name = seq->name();
  }
  setWindowTitle(QString("%1: %2").arg(panel_name, name));
}

void Viewer::set_zoom_value(double d) {
  headers->update_zoom(d);
  if (viewer_widget->waveform) {
    viewer_widget->waveform_zoom = d;
    viewer_widget->update();
  }
  if (seq != nullptr) {
    set_sb_max();
    if (!horizontal_bar->is_resizing())
      center_scroll_to_playhead(horizontal_bar, headers->get_zoom(), seq->playhead_);
  }
}

void Viewer::set_sb_max() {
  headers->set_scrollbar_max(horizontal_bar, seq->endFrame(), headers->width());
}

long Viewer::get_seq_in() {
  return (seq->workarea_.using_ && seq->workarea_.enabled_)
      ? seq->workarea_.in_
      : 0;
}

long Viewer::get_seq_out() {
  return (seq->workarea_.using_ && seq->workarea_.enabled_ && previous_playhead < seq->workarea_.out_)
      ? seq->workarea_.out_
      : seq->endFrame();
}

void Viewer::setup_ui() {
  QWidget* contents = new QWidget();

  QVBoxLayout* layout = new QVBoxLayout(contents);
  layout->setSpacing(0);
  layout->setMargin(0);

  viewer_container = new ViewerContainer(contents);
  viewer_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(viewer_container);

  headers = new TimelineHeader(contents);
  layout->addWidget(headers);

  horizontal_bar = new ResizableScrollBar(contents);
  horizontal_bar->setSingleStep(20);
  horizontal_bar->setOrientation(Qt::Horizontal);
  layout->addWidget(horizontal_bar);

  QWidget* lower_controls = new QWidget(contents);

  QHBoxLayout* lower_control_layout = new QHBoxLayout(lower_controls);
  lower_control_layout->setMargin(0);

  // current time code
  QWidget* current_timecode_container = new QWidget(lower_controls);
  QHBoxLayout* current_timecode_container_layout = new QHBoxLayout(current_timecode_container);
  current_timecode_container_layout->setSpacing(0);
  current_timecode_container_layout->setMargin(0);
  currentTimecode = new LabelSlider(current_timecode_container);
  current_timecode_container_layout->addWidget(currentTimecode);
  lower_control_layout->addWidget(current_timecode_container);

  QWidget* playback_controls = new QWidget(lower_controls);

  QHBoxLayout* playback_control_layout = new QHBoxLayout(playback_controls);
  playback_control_layout->setSpacing(0);
  playback_control_layout->setMargin(0);

  btnSkipToStart = new QPushButton(playback_controls);
  QIcon goToStartIcon;
  goToStartIcon.addFile(QStringLiteral(":/icons/prev.png"), QSize(), QIcon::Normal, QIcon::Off);
  goToStartIcon.addFile(QStringLiteral(":/icons/prev-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  btnSkipToStart->setIcon(goToStartIcon);
  connect(btnSkipToStart, SIGNAL(clicked(bool)), this, SLOT(go_to_in()));
  playback_control_layout->addWidget(btnSkipToStart);

  btnRewind = new QPushButton(playback_controls);
  QIcon rewindIcon;
  rewindIcon.addFile(QStringLiteral(":/icons/rew.png"), QSize(), QIcon::Normal, QIcon::Off);
  rewindIcon.addFile(QStringLiteral(":/icons/rew-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  btnRewind->setIcon(rewindIcon);
  connect(btnRewind, SIGNAL(clicked(bool)), this, SLOT(previous_frame()));
  playback_control_layout->addWidget(btnRewind);

  btnPlay = new QPushButton(playback_controls);
  playIcon.addFile(QStringLiteral(":/icons/play.png"), QSize(), QIcon::Normal, QIcon::On);
  playIcon.addFile(QStringLiteral(":/icons/play-disabled.png"), QSize(), QIcon::Disabled, QIcon::On);
  btnPlay->setIcon(playIcon);
  connect(btnPlay, SIGNAL(clicked(bool)), this, SLOT(toggle_play()));
  playback_control_layout->addWidget(btnPlay);

  btnFastForward = new QPushButton(playback_controls);
  QIcon ffIcon;
  ffIcon.addFile(QStringLiteral(":/icons/ff.png"), QSize(), QIcon::Normal, QIcon::On);
  ffIcon.addFile(QStringLiteral(":/icons/ff-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  btnFastForward->setIcon(ffIcon);
  connect(btnFastForward, SIGNAL(clicked(bool)), this, SLOT(next_frame()));
  playback_control_layout->addWidget(btnFastForward);

  btnSkipToEnd = new QPushButton(playback_controls);
  QIcon nextIcon;
  nextIcon.addFile(QStringLiteral(":/icons/next.png"), QSize(), QIcon::Normal, QIcon::Off);
  nextIcon.addFile(QStringLiteral(":/icons/next-disabled.png"), QSize(), QIcon::Disabled, QIcon::Off);
  btnSkipToEnd->setIcon(nextIcon);
  connect(btnSkipToEnd, SIGNAL(clicked(bool)), this, SLOT(go_to_out()));
  playback_control_layout->addWidget(btnSkipToEnd);

  lower_control_layout->addWidget(playback_controls);

  QWidget* end_timecode_container = new QWidget(lower_controls);

  QHBoxLayout* end_timecode_layout = new QHBoxLayout(end_timecode_container);
  end_timecode_layout->setSpacing(0);
  end_timecode_layout->setMargin(0);

  endTimecode = new QLabel(end_timecode_container);
  endTimecode->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

  end_timecode_layout->addWidget(endTimecode);

  lower_control_layout->addWidget(end_timecode_container);

  layout->addWidget(lower_controls);

  setWidget(contents);
}

SequencePtr Viewer::createFootageSequence(const MediaPtr& mda) const
{
  auto ftg = mda->object<Footage>();
  auto sqn = std::make_shared<Sequence>();
  sqn->markers_ = ftg->markers_;
  sqn->wrapper_sequence_ = true;
  sqn->setName(ftg->name());

  sqn->workarea_.using_ = ftg->using_inout;
  if (ftg->using_inout) {
    sqn->workarea_.in_ = ftg->in;
    sqn->workarea_.out_ = ftg->out;
  }

  sqn->setFrameRate(MEDIA_FRAME_RATE);

  if (!ftg->video_tracks.empty()) {
    const auto video_stream = ftg->video_tracks.front();
    sqn->setWidth(video_stream->video_width);
    sqn->setHeight(video_stream->video_height);
    if ( (video_stream->video_frame_rate > 0) && (!video_stream->infinite_length) ) { // not image?
      sqn->setFrameRate(video_stream->video_frame_rate * ftg->speed);
    }

    auto clp = std::make_shared<Clip>(sqn);
    clp->timeline_info.media        = mda;
    clp->timeline_info.media_stream = video_stream->file_index;
    clp->timeline_info.in           = 0;
    clp->timeline_info.out          = ftg->get_length_in_frames(sqn->frameRate());
    if (clp->timeline_info.out <= 0) {
      clp->timeline_info.out = 150;
    }
    clp->timeline_info.track_ = -1;
    clp->timeline_info.clip_in  = 0;
    clp->recalculateMaxLength();
    sqn->clips_.append(clp);
  } else {
    sqn->setWidth(MEDIA_WIDTH);
    sqn->setHeight(MEDIA_HEIGHT);
  }

  if (!ftg->audio_tracks.empty()) {
    const auto audio_stream = ftg->audio_tracks.front();
    sqn->setAudioFrequency(audio_stream->audio_frequency);

    auto clp = std::make_shared<Clip>(global::sequence);
    clp->timeline_info.media        = mda;
    clp->timeline_info.media_stream = audio_stream->file_index;
    clp->timeline_info.in           = 0;
    clp->timeline_info.out          = ftg->get_length_in_frames(sqn->frameRate());
    clp->timeline_info.track_       = 0;
    clp->timeline_info.clip_in      = 0;
    clp->recalculateMaxLength();
    sqn->clips_.append(clp);

    if (ftg->video_tracks.empty()) {
      viewer_widget->waveform         = true;
      viewer_widget->waveform_clip    = clp;
      viewer_widget->waveform_ms      = audio_stream;
      viewer_widget->frame_update();
    }
  } else {
    sqn->setAudioFrequency(MEDIA_AUDIO_FREQUENCY);
  }

  sqn->setAudioLayout(AV_CH_LAYOUT_STEREO);
  return sqn;
}

void Viewer::set_media(MediaPtr m) {
  main_sequence = false;
  if (media != nullptr) {
    if (media == m) {
      // prevent assigning shared_ptr to itself
      return;
    }
  }
  media = m;
  clean_created_seq();
  if (media != nullptr) {
    switch (media->type()) {
      case MediaType::FOOTAGE:
        seq = createFootageSequence(media);
        created_sequence = true;
        break;
      case MediaType::SEQUENCE:
        seq = media->object<Sequence>();
        break;
      default:
        qWarning() << "Unhandled media type" << static_cast<int>(media->type());
        break;
    }//switch
  }
  set_sequence(false, seq);
}

void Viewer::reset() {
  //TODO:
}

void Viewer::update_playhead() {
  seek(currentTimecode->value());
}

void Viewer::timer_update() {
  previous_playhead = seq->playhead_;

  seq->playhead_ = qRound(playhead_start + ((QDateTime::currentMSecsSinceEpoch()-start_msecs) * 0.001 * seq->frameRate()));
  if (e_config.seek_also_selects) PanelManager::timeLine().select_from_playhead();
  update_parents(e_config.seek_also_selects);

  long end_frame = get_seq_out();
  if (!recording
      && playing
      && seq->playhead_ >= end_frame
      && previous_playhead < end_frame) {
    if (!e_config.pause_at_out_point && e_config.loop) {
      seek(get_seq_in());
      play();
    } else if (e_config.pause_at_out_point || !main_sequence) {
      pause();
    }
  } else if (recording && recording_start != recording_end && seq->playhead_ >= recording_end) {
    pause();
  }
}

void Viewer::recording_flasher_update() {
  if (btnPlay->styleSheet().isEmpty()) {
    btnPlay->setStyleSheet("background: red;");
  } else {
    btnPlay->setStyleSheet(QString());
  }
}

void Viewer::resize_move(double d) {
  set_zoom_value(headers->get_zoom()*d);
}

void Viewer::clean_created_seq() {
  viewer_widget->waveform = false;

  if (created_sequence) {
    // TODO delete undo commands referencing this sequence to avoid crashes
    /*for (int i=0;i<undo_stack.count();i++) {
            undo_stack.command(i)
        }*/

    seq = nullptr;
    created_sequence = false;
  }
}

void Viewer::set_sequence(bool main, SequencePtr s) {
  pause();

  reset_all_audio();

  if (seq != nullptr) {
    seq->closeActiveClips();
  }

  main_sequence = main;
  seq = (main) ? global::sequence : s;

  bool null_sequence = (seq == nullptr);

  headers->setEnabled(!null_sequence);
  currentTimecode->setEnabled(!null_sequence);
  viewer_widget->setEnabled(!null_sequence);
  viewer_widget->setVisible(!null_sequence);
  btnSkipToStart->setEnabled(!null_sequence);
  btnRewind->setEnabled(!null_sequence);
  btnPlay->setEnabled(!null_sequence);
  btnFastForward->setEnabled(!null_sequence);
  btnSkipToEnd->setEnabled(!null_sequence);

  if (!null_sequence) {
    currentTimecode->set_frame_rate(seq->frameRate());

    playback_updater.setInterval(qFloor(1000 / seq->frameRate()));

    update_playhead_timecode(seq->playhead_);
    update_end_timecode();

    viewer_container->adjust();
  } else {
    update_playhead_timecode(0);
    update_end_timecode();
  }
  update_window_title();

  update_header_zoom();

  viewer_widget->frame_update();

  update();
}

void Viewer::set_playpause_icon(bool play) {
  btnPlay->setIcon(play ? playIcon : QIcon(":/icons/pause.png"));
}
