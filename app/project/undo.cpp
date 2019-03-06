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
#include "undo.h"

#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QCheckBox>

#include "project/clip.h"
#include "project/sequence.h"
#include "panels/panels.h"
#include "panels/project.h"
#include "panels/effectcontrols.h"
#include "panels/viewer.h"
#include "panels/timeline.h"
#include "playback/playback.h"
#include "ui/sourcetable.h"
#include "project/effect.h"
#include "project/transition.h"
#include "project/footage.h"
#include "ui/labelslider.h"
#include "ui/viewerwidget.h"
#include "project/marker.h"
#include "ui/mainwindow.h"
#include "io/clipboard.h"
#include "project/media.h"
#include "debug.h"

QUndoStack e_undo_stack;

ComboAction::ComboAction() {}

ComboAction::~ComboAction()
{
  for (int i=0;i<commands.size();i++) {
    delete commands.at(i);
  }
}

void ComboAction::undo() {
  for (int i=commands.size()-1;i>=0;i--) {
    commands.at(i)->undo();
  }
  for (int i=0;i<post_commands.size();i++) {
    post_commands.at(i)->undo();
  }
}

void ComboAction::redo() {
  for (int i=0;i<commands.size();i++) {
    commands.at(i)->redo();
  }
  for (int i=0;i<post_commands.size();i++) {
    post_commands.at(i)->redo();
  }
}

void ComboAction::append(QUndoCommand* u) {
  commands.append(u);
}

void ComboAction::appendPost(QUndoCommand* u) {
  post_commands.append(u);
}

MoveClipAction::MoveClipAction(ClipPtr c, const long iin, const long iout, const long iclip_in, const int itrack, const bool irelative)
  : clip(c),
    old_in(c->timeline_info.in),
    old_out(c->timeline_info.out),
    old_clip_in(c->timeline_info.clip_in),
    old_track(c->timeline_info.track_),
    new_in(iin),
    new_out(iout),
    new_clip_in(iclip_in),
    new_track(itrack),
    relative(irelative),
    old_project_changed(global::mainWindow->isWindowModified())
{}

void MoveClipAction::undo() {
  if (relative) {
    clip->timeline_info.in -= new_in;
    clip->timeline_info.out -= new_out;
    clip->timeline_info.clip_in -= new_clip_in;
    clip->timeline_info.track_ -= new_track;
  } else {
    clip->timeline_info.in = old_in;
    clip->timeline_info.out = old_out;
    clip->timeline_info.clip_in = old_clip_in;
    clip->timeline_info.track_  = old_track ;
  }

  global::mainWindow->setWindowModified(old_project_changed);
}

void MoveClipAction::redo() {
  if (relative) {
    clip->timeline_info.in += new_in;
    clip->timeline_info.out += new_out;
    clip->timeline_info.clip_in += new_clip_in;
    clip->timeline_info.track_ += new_track;
  } else {
    clip->timeline_info.in = new_in;
    clip->timeline_info.out = new_out;
    clip->timeline_info.clip_in = new_clip_in;
    clip->timeline_info.track_ = new_track;
  }

  global::mainWindow->setWindowModified(true);
}

DeleteClipAction::DeleteClipAction(SequencePtr s, const int clip) :
  seq(s),
  index(clip),
  opening_transition(-1),
  closing_transition(-1),
  old_project_changed(global::mainWindow->isWindowModified())
{}

DeleteClipAction::~DeleteClipAction() {

}

void DeleteClipAction::undo() {
  // restore ref to clip
  seq->clips_[index] = ref;

  // restore shared transitions
  if (opening_transition > -1) {
    seq->transitions_.at(opening_transition)->secondary_clip = seq->transitions_.at(opening_transition)->parent_clip;
    seq->transitions_.at(opening_transition)->parent_clip = ref;
    ref->opening_transition = opening_transition;
    opening_transition = -1;
  }
  if (closing_transition > -1) {
    seq->transitions_.at(closing_transition)->secondary_clip = ref;
    ref->closing_transition = closing_transition;
    closing_transition = -1;
  }

  // restore links to this clip
  for (int i=linkClipIndex.size()-1;i>=0;i--) {
    seq->clips_.at(linkClipIndex.at(i))->linked.insert(linkLinkIndex.at(i), index);
  }

  ref = nullptr;

  global::mainWindow->setWindowModified(old_project_changed);
}

void DeleteClipAction::redo() {
  // remove ref to clip
  ref = seq->clips_.at(index);
  ref->close(true);

  seq->clips_[index] = nullptr;

  // save shared transitions
  if (ref->opening_transition > -1 && !ref->openingTransition()->secondary_clip.expired()) {
    opening_transition = ref->opening_transition;
    ref->openingTransition()->parent_clip = ref->openingTransition()->secondary_clip.lock();
    ref->openingTransition()->secondary_clip.reset();
    ref->opening_transition = -1;
  }
  if (ref->closing_transition > -1 && !ref->closingTransition()->secondary_clip.expired()) {
    closing_transition = ref->closing_transition;
    ref->closingTransition()->secondary_clip.reset();
    ref->closing_transition = -1;
  }

  // delete link to this clip
  linkClipIndex.clear();
  linkLinkIndex.clear();
  for (int i=0;i<seq->clips_.size();i++) {
    ClipPtr   c = seq->clips_.at(i);
    if (c != nullptr) {
      for (int j=0;j<c->linked.size();j++) {
        if (c->linked.at(j) == index) {
          linkClipIndex.append(i);
          linkLinkIndex.append(j);
          c->linked.removeAt(j);
        }
      }
    }
  }

  global::mainWindow->setWindowModified(true);
}

ChangeSequenceAction::ChangeSequenceAction(SequencePtr s) :
  new_sequence(s)
{}

void ChangeSequenceAction::undo() {
  set_sequence(old_sequence);
}

void ChangeSequenceAction::redo() {
  old_sequence = global::sequence;
  set_sequence(new_sequence);
}

SetTimelineInOutCommand::SetTimelineInOutCommand(SequencePtr s, const bool enabled, const long in, const long out) :
  seq(s),
  new_enabled(enabled),
  new_in(in),
  new_out(out),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetTimelineInOutCommand::undo() {
  seq->workarea_.using_ = old_enabled;
  seq->workarea_.enabled_ = old_workarea_enabled;
  seq->workarea_.in_ = old_in;
  seq->workarea_.out_ = old_out;

  // footage viewer functions
  if (seq->wrapper_sequence_) {
    FootagePtr  m = seq->clips_.at(0)->timeline_info.media->object<Footage>();
    m->using_inout = old_enabled;
    m->in = old_in;
    m->out = old_out;
  }

  global::mainWindow->setWindowModified(old_project_changed);
}

void SetTimelineInOutCommand::redo() {
  old_enabled = seq->workarea_.using_;
  old_workarea_enabled = seq->workarea_.enabled_;
  old_in = seq->workarea_.in_;
  old_out = seq->workarea_.out_;

  if (!seq->workarea_.using_) seq->workarea_.enabled_ = true;
  seq->workarea_.using_ = new_enabled;
  seq->workarea_.in_ = new_in;
  seq->workarea_.out_ = new_out;

  // footage viewer functions
  if (seq->wrapper_sequence_) {
    FootagePtr  m = seq->clips_.at(0)->timeline_info.media->object<Footage>();
    m->using_inout = new_enabled;
    m->in = new_in;
    m->out = new_out;
  }

  global::mainWindow->setWindowModified(true);
}

AddEffectCommand::AddEffectCommand(ClipPtr   c, EffectPtr e, const EffectMeta *m, const int insert_pos) :
  clip(c),
  meta(m),
  ref(e),
  pos(insert_pos),
  done(false),
  old_project_changed(global::mainWindow->isWindowModified())
{}

AddEffectCommand::~AddEffectCommand() {

}

void AddEffectCommand::undo() {
  clip->effects.last()->close();
  if (pos < 0) {
    clip->effects.removeLast();
  } else {
    clip->effects.removeAt(pos);
  }
  done = false;
  global::mainWindow->setWindowModified(old_project_changed);
}

void AddEffectCommand::redo() {
  if (ref == nullptr) {
    ref = create_effect(clip, meta);
  }
  if (pos < 0) {
    clip->effects.append(ref);
  } else {
    clip->effects.insert(pos, ref);
  }
  done = true;
  global::mainWindow->setWindowModified(true);
}

AddTransitionCommand::AddTransitionCommand(ClipPtr   c, ClipPtr s, TransitionPtr copy, const EffectMeta *itransition, const int itype, const int ilength) :
  clip(c),
  secondary(s),
  transition_to_copy(copy),
  transition(itransition),
  type(itype),
  length(ilength),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void AddTransitionCommand::undo() {
  clip->sequence->hardDeleteTransition(clip, type);
  if (secondary != nullptr) secondary->sequence->hardDeleteTransition(secondary, (type == TA_OPENING_TRANSITION) ? TA_CLOSING_TRANSITION : TA_OPENING_TRANSITION);

  if (type == TA_OPENING_TRANSITION) {
    clip->opening_transition = old_ptransition;
    if (secondary != nullptr) secondary->closing_transition = old_stransition;
  } else {
    clip->closing_transition = old_ptransition;
    if (secondary != nullptr) secondary->opening_transition = old_stransition;
  }

  global::mainWindow->setWindowModified(old_project_changed);
}

void AddTransitionCommand::redo() {
  if (type == TA_OPENING_TRANSITION) {
    old_ptransition = clip->opening_transition;
    clip->opening_transition = (transition_to_copy == nullptr) ? create_transition(clip, secondary, transition) : transition_to_copy->copy(clip, nullptr);
    if (secondary != nullptr) {
      old_stransition = secondary->closing_transition;
      secondary->closing_transition = clip->opening_transition;
    }
    if (length > 0) {
      clip->openingTransition()->set_length(length);
    }
  } else {
    old_ptransition = clip->closing_transition;
    clip->closing_transition = (transition_to_copy == nullptr) ? create_transition(clip, secondary, transition) : transition_to_copy->copy(clip, nullptr);
    if (secondary != nullptr) {
      old_stransition = secondary->opening_transition;
      secondary->opening_transition = clip->closing_transition;
    }
    if (length > 0) {
      clip->closingTransition()->set_length(length);
    }
  }
  global::mainWindow->setWindowModified(true);
}

ModifyTransitionCommand::ModifyTransitionCommand(ClipPtr c, const int itype, const long ilength) :
  clip(c),
  type(itype),
  new_length(ilength),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void ModifyTransitionCommand::undo() {
  TransitionPtr t = (type == TA_OPENING_TRANSITION) ? clip->openingTransition() : clip->closingTransition();
  t->set_length(old_length);
  global::mainWindow->setWindowModified(old_project_changed);
}

void ModifyTransitionCommand::redo() {
  TransitionPtr t = (type == TA_OPENING_TRANSITION) ? clip->openingTransition() : clip->closingTransition();
  old_length = t->get_true_length();
  t->set_length(new_length);
  global::mainWindow->setWindowModified(true);
}

DeleteTransitionCommand::DeleteTransitionCommand(SequencePtr s, const int transition_index) :
  seq(s),
  index(transition_index),
  transition(nullptr),
  otc(nullptr),
  ctc(nullptr),
  old_project_changed(global::mainWindow->isWindowModified())
{}

DeleteTransitionCommand::~DeleteTransitionCommand() {
}

void DeleteTransitionCommand::undo() {
  seq->transitions_[index] = transition;

  if (otc != nullptr) {
    otc->opening_transition = index;
  }
  if (ctc != nullptr) {
    ctc->closing_transition = index;
  }

  transition = nullptr;
  global::mainWindow->setWindowModified(old_project_changed);
}

void DeleteTransitionCommand::redo() {
  for (int i=0;i<seq->clips_.size();i++) {
    ClipPtr   c = seq->clips_.at(i);
    if (c != nullptr) {
      if (c->opening_transition == index) {
        otc = c;
        c->opening_transition = -1;
      }
      if (c->closing_transition == index) {
        ctc = c;
        c->closing_transition = -1;
      }
    }
  }

  transition = seq->transitions_.at(index);
  seq->transitions_[index] = nullptr;

  global::mainWindow->setWindowModified(true);
}

NewSequenceCommand::NewSequenceCommand(MediaPtr s, MediaPtr iparent, const bool modified) :
  seq_(s),
  parent_(iparent),
  done_(false),
  old_project_changed_(modified)
{
  if (parent_ == nullptr) {
    parent_ = project_model.root();
  }
}

NewSequenceCommand::~NewSequenceCommand()
{

}

void NewSequenceCommand::undo()
{
  project_model.removeChild(parent_, seq_);

  done_ = false;
  global::mainWindow->setWindowModified(old_project_changed_);
}

void NewSequenceCommand::redo()
{
  project_model.appendChild(parent_, seq_);

  done_ = true;
  global::mainWindow->setWindowModified(true);
}

AddMediaCommand::AddMediaCommand(MediaPtr iitem, MediaPtr iparent) :
  item(iitem),
  parent(iparent),
  done(false),
  old_project_changed(global::mainWindow->isWindowModified())
{}

AddMediaCommand::~AddMediaCommand() {

}

void AddMediaCommand::undo() {
  project_model.removeChild(parent, item);
  done = false;
  global::mainWindow->setWindowModified(old_project_changed);
}

void AddMediaCommand::redo() {
  project_model.appendChild(parent, item);

  done = true;
  global::mainWindow->setWindowModified(true);
}

DeleteMediaCommand::DeleteMediaCommand(MediaPtr i) :
  item(i),
  parent(i->parentItem()),
  old_project_changed(global::mainWindow->isWindowModified())
{}

DeleteMediaCommand::~DeleteMediaCommand() {

}

void DeleteMediaCommand::undo() {
  project_model.appendChild(parent, item);

  global::mainWindow->setWindowModified(old_project_changed);
  done = false;
}

void DeleteMediaCommand::redo() {
  project_model.removeChild(parent, item);

  global::mainWindow->setWindowModified(true);
  done = true;
}

AddClipCommand::AddClipCommand(SequencePtr s, QVector<ClipPtr  >& add) :
  seq(s),
  clips(add),
  old_project_changed(global::mainWindow->isWindowModified())
{}

AddClipCommand::~AddClipCommand() {

}

void AddClipCommand::undo() {
  e_panel_effect_controls->clear_effects(true);
  for (int i=0;i<clips.size();i++) {
    ClipPtr   c = seq->clips_.last();
    e_panel_timeline->deselect_area(c->timeline_info.in, c->timeline_info.out, c->timeline_info.track_);
    undone_clips.prepend(c);
    c->close(true);
    seq->clips_.removeLast();
  }
  global::mainWindow->setWindowModified(old_project_changed);
}

void AddClipCommand::redo() {
  if (undone_clips.size() > 0) {
    for (int i=0;i<undone_clips.size();i++) {
      seq->clips_.append(undone_clips.at(i));
    }
    undone_clips.clear();
  } else {
    int linkOffset = seq->clips_.size();
    for (int i=0;i<clips.size();i++) {
      ClipPtr   original = clips.at(i);
      ClipPtr   copy = original->copy(seq);
      copy->linked.resize(original->linked.size());
      for (int j=0;j<original->linked.size();j++) {
        copy->linked[j] = original->linked.at(j) + linkOffset;
      }
      if (original->opening_transition > -1) copy->opening_transition = original->openingTransition()->copy(copy, nullptr);
      if (original->closing_transition > -1) copy->closing_transition = original->closingTransition()->copy(copy, nullptr);
      seq->clips_.append(copy);
    }
  }
  global::mainWindow->setWindowModified(true);
}

LinkCommand::LinkCommand()
  : link(true),
    old_project_changed(global::mainWindow->isWindowModified())
{

}

void LinkCommand::undo() {
  for (int i=0;i<clips.size();i++) {
    ClipPtr   c = s->clips_.at(clips.at(i));
    if (link) {
      c->linked.clear();
    } else {
      c->linked = old_links.at(i);
    }
  }
  global::mainWindow->setWindowModified(old_project_changed);
}

void LinkCommand::redo() {
  old_links.clear();
  for (int i=0;i<clips.size();i++) {
    dout << clips.at(i);
    ClipPtr   c = s->clips_.at(clips.at(i));
    if (link) {
      for (int j=0;j<clips.size();j++) {
        if (i != j) {
          c->linked.append(clips.at(j));
        }
      }
    } else {
      old_links.append(c->linked);
      c->linked.clear();
    }
  }
  global::mainWindow->setWindowModified(true);
}

CheckboxCommand::CheckboxCommand(QCheckBox* b) : box(b), checked(box->isChecked()), done(true), old_project_changed(global::mainWindow->isWindowModified()) {}

CheckboxCommand::~CheckboxCommand() {}

void CheckboxCommand::undo() {
  box->setChecked(!checked);
  done = false;
  global::mainWindow->setWindowModified(old_project_changed);
}

void CheckboxCommand::redo() {
  if (!done) {
    box->setChecked(checked);
  }
  global::mainWindow->setWindowModified(true);
}

ReplaceMediaCommand::ReplaceMediaCommand(MediaPtr i, QString s) :
  item(i),
  new_filename(s),
  old_project_changed(global::mainWindow->isWindowModified())
{
  old_filename = item->object<Footage>()->url;
}

void ReplaceMediaCommand::replace(QString& filename) {
  // close any clips currently using this media
  QVector<MediaPtr> all_sequences = e_panel_project->list_all_project_sequences();
  for (int i=0;i<all_sequences.size();i++) {
    SequencePtr s = all_sequences.at(i)->object<Sequence>();
    for (int j=0;j<s->clips_.size();j++) {
      ClipPtr   c = s->clips_.at(j);
      if (c != nullptr && c->timeline_info.media == item && c->is_open) {
        c->close(true);
        c->replaced = true;
      }
    }
  }

  // replace media
  QStringList files;
  files.append(filename);
  item->object<Footage>()->ready_lock.lock();
  e_panel_project->process_file_list(files, false, item, nullptr);
}

void ReplaceMediaCommand::undo() {
  replace(old_filename);

  global::mainWindow->setWindowModified(old_project_changed);
}

void ReplaceMediaCommand::redo() {
  replace(new_filename);

  global::mainWindow->setWindowModified(true);
}

ReplaceClipMediaCommand::ReplaceClipMediaCommand(MediaPtr a, MediaPtr b, const bool e) :
  old_media(a),
  new_media(b),
  preserve_clip_ins(e),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void ReplaceClipMediaCommand::replace(bool undo) {
  if (!undo) {
    old_clip_ins.clear();
  }

  for (int i=0;i<clips.size();i++) {
    ClipPtr c = clips.at(i);
    c->close(true);

    if (undo) {
      if (!preserve_clip_ins) {
        c->timeline_info.clip_in = old_clip_ins.at(i);
      }

      c->timeline_info.media = old_media;
    } else {
      if (!preserve_clip_ins) {
        old_clip_ins.append(c->timeline_info.clip_in);
        c->timeline_info.clip_in = 0;
      }

      c->timeline_info.media = new_media;
    }

    c->replaced = true;
    c->refresh();
  }
}

void ReplaceClipMediaCommand::undo() {
  replace(true);

  global::mainWindow->setWindowModified(old_project_changed);
}

void ReplaceClipMediaCommand::redo() {
  replace(false);

  update_ui(true);
  global::mainWindow->setWindowModified(true);
}

EffectDeleteCommand::EffectDeleteCommand()
  : done(false),
    old_project_changed(global::mainWindow->isWindowModified())
{

}

EffectDeleteCommand::~EffectDeleteCommand() {

}

void EffectDeleteCommand::undo() {
  for (int i=0;i<clips.size();i++) {
    ClipPtr   c = clips.at(i);
    c->effects.insert(fx.at(i), deleted_objects.at(i));
  }
  e_panel_effect_controls->reload_clips();
  done = false;
  global::mainWindow->setWindowModified(old_project_changed);
}

void EffectDeleteCommand::redo() {
  deleted_objects.clear();
  for (int i=0;i<clips.size();i++) {
    ClipPtr   c = clips.at(i);
    int fx_id = fx.at(i) - i;
    EffectPtr e = c->effects.at(fx_id);
    e->close();
    deleted_objects.append(e);
    c->effects.removeAt(fx_id);
  }
  e_panel_effect_controls->reload_clips();
  done = true;
  global::mainWindow->setWindowModified(true);
}

MediaMove::MediaMove() : old_project_changed(global::mainWindow->isWindowModified()) {}

void MediaMove::undo() {
  for (int i=0;i<items.size();i++) {
    project_model.moveChild(items.at(i), froms.at(i));
  }
  global::mainWindow->setWindowModified(old_project_changed);
}

void MediaMove::redo() {
  if (to == nullptr) to = project_model.root();
  froms.resize(items.size());
  for (int i=0;i<items.size();i++) {
    MediaWPtr parent = items.at(i)->parentItem();
    if (!parent.expired()) {
      MediaPtr parPtr = parent.lock();
      froms[i] = parPtr;
      project_model.moveChild(items.at(i), to);
    }
  }
  global::mainWindow->setWindowModified(true);
}

MediaRename::MediaRename(MediaPtr iitem, const QString& ito) :
  old_project_changed(global::mainWindow->isWindowModified()),
  item(iitem),
  from(iitem->name()),
  to(ito)
{

}

void MediaRename::undo() {
  item->setName(from);
  global::mainWindow->setWindowModified(old_project_changed);
}

void MediaRename::redo() {
  item->setName(to);
  global::mainWindow->setWindowModified(true);
}

KeyframeDelete::KeyframeDelete(EffectField* ifield, const int iindex) :
  field(ifield),
  index(iindex),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void KeyframeDelete::undo() {
  field->keyframes.insert(index, deleted_key);
  global::mainWindow->setWindowModified(old_project_changed);
}

void KeyframeDelete::redo() {
  deleted_key = field->keyframes.at(index);
  field->keyframes.removeAt(index);
  global::mainWindow->setWindowModified(true);
}

EffectFieldUndo::EffectFieldUndo(EffectField* f) :
  field(f),
  done(true),
  old_project_changed(global::mainWindow->isWindowModified())
{
  old_val = field->get_previous_data();
  new_val = field->get_current_data();
}

void EffectFieldUndo::undo() {
  field->set_current_data(old_val);
  done = false;
  global::mainWindow->setWindowModified(old_project_changed);
}

void EffectFieldUndo::redo() {
  if (!done) {
    field->set_current_data(new_val);
  }
  global::mainWindow->setWindowModified(true);
}

SetAutoscaleAction::SetAutoscaleAction() :
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetAutoscaleAction::undo() {
  for (int i=0;i<clips.size();i++) {
    clips.at(i)->timeline_info.autoscale = !clips.at(i)->timeline_info.autoscale;
  }
  e_panel_sequence_viewer->viewer_widget->update();
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetAutoscaleAction::redo() {
  for (int i=0;i<clips.size();i++) {
    clips.at(i)->timeline_info.autoscale = !clips.at(i)->timeline_info.autoscale;
  }
  e_panel_sequence_viewer->viewer_widget->update();
  global::mainWindow->setWindowModified(true);
}

AddMarkerAction::AddMarkerAction(SequencePtr s, const long t, QString n) :
  seq(s),
  time(t),
  name(n),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void AddMarkerAction::undo() {
  if (index == -1) {
    seq->markers_.removeLast();
  } else {
    seq->markers_[index].name = old_name;
  }

  global::mainWindow->setWindowModified(old_project_changed);
}

void AddMarkerAction::redo() {
  index = -1;
  for (int i=0;i<seq->markers_.size();i++) {
    if (seq->markers_.at(i).frame == time) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    Marker m;
    m.frame = time;
    seq->markers_.append(m);
  } else {
    old_name = seq->markers_.at(index).name;
    seq->markers_[index].name = name;
  }

  global::mainWindow->setWindowModified(true);
}

MoveMarkerAction::MoveMarkerAction(Marker* m, const long o, const long n) :
  marker(m),
  old_time(o),
  new_time(n),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void MoveMarkerAction::undo() {
  marker->frame = old_time;
  global::mainWindow->setWindowModified(old_project_changed);
}

void MoveMarkerAction::redo() {
  marker->frame = new_time;
  global::mainWindow->setWindowModified(true);
}

DeleteMarkerAction::DeleteMarkerAction(SequencePtr s) :
  seq(s),
  sorted(false),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void DeleteMarkerAction::undo() {
  for (int i=markers.size()-1;i>=0;i--) {
    seq->markers_.insert(markers.at(i), copies.at(i));
  }
  global::mainWindow->setWindowModified(old_project_changed);
}

void DeleteMarkerAction::redo() {
  for (int i=0;i<markers.size();i++) {
    // correct future removals
    if (!sorted) {
      copies.append(seq->markers_.at(markers.at(i)));
      for (int j=i+1;j<markers.size();j++) {
        if (markers.at(j) > markers.at(i)) {
          markers[j]--;
        }
      }
    }
    seq->markers_.removeAt(markers.at(i));
  }
  sorted = true;
  global::mainWindow->setWindowModified(true);
}

SetSpeedAction::SetSpeedAction(ClipPtr   c, double speed) :
  clip(c),
  old_speed(c->timeline_info.speed),
  new_speed(speed),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetSpeedAction::undo() {
  clip->timeline_info.speed = old_speed;
  clip->recalculateMaxLength();
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetSpeedAction::redo() {
  clip->timeline_info.speed = new_speed;
  clip->recalculateMaxLength();
  global::mainWindow->setWindowModified(true);
}

SetBool::SetBool(bool* b, const bool setting) :
  boolean(b),
  old_setting(*b),
  new_setting(setting),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetBool::undo() {
  *boolean = old_setting;
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetBool::redo() {
  *boolean = new_setting;
  global::mainWindow->setWindowModified(true);
}

SetSelectionsCommand::SetSelectionsCommand(SequencePtr s) :
  seq(s),
  done(true),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetSelectionsCommand::undo() {
  seq->selections_  = old_data;
  done = false;
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetSelectionsCommand::redo() {
  if (!done) {
    seq->selections_  = new_data;
    done = true;
  }
  global::mainWindow->setWindowModified(true);
}

SetEnableCommand::SetEnableCommand(ClipPtr c, const bool enable) :
  clip(c),
  old_val(c->timeline_info.enabled),
  new_val(enable),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetEnableCommand::undo() {
  clip->timeline_info.enabled = old_val;
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetEnableCommand::redo() {
  clip->timeline_info.enabled = new_val;
  global::mainWindow->setWindowModified(true);
}

EditSequenceCommand::EditSequenceCommand(MediaPtr i, SequencePtr s) :
  item(i),
  seq(s),
  old_project_changed(global::mainWindow->isWindowModified()),
  old_name(s->name()),
  old_width(s->width()),
  old_height(s->height()),
  old_frame_rate(s->frameRate()),
  old_audio_frequency(s->audioFrequency()),
  old_audio_layout(s->audioLayout())
{}

void EditSequenceCommand::undo() {
  seq->setName(old_name);
  seq->setWidth(old_width);
  seq->setHeight(old_height);
  seq->setFrameRate(old_frame_rate);
  seq->setAudioFrequency(old_audio_frequency);
  seq->setAudioLayout(old_audio_layout);
  update();

  global::mainWindow->setWindowModified(old_project_changed);
}

void EditSequenceCommand::redo() {
  seq->setName(name);
  seq->setWidth(width);
  seq->setHeight(height);
  seq->setFrameRate(frame_rate);
  seq->setAudioFrequency(audio_frequency);
  seq->setAudioLayout(audio_layout);
  update();

  global::mainWindow->setWindowModified(true);
}

void EditSequenceCommand::update() {
  // update tooltip
  item->setSequence(seq);

  for (int i=0;i<seq->clips_.size();i++) {
    if (seq->clips_.at(i) != nullptr) seq->clips_.at(i)->refresh();
  }

  if (global::sequence == seq) {
    set_sequence(seq);
  }
}

SetInt::SetInt(int* pointer, const int new_value) :
  p(pointer),
  oldval(*pointer),
  newval(new_value),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetInt::undo() {
  *p = oldval;
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetInt::redo() {
  *p = newval;
  global::mainWindow->setWindowModified(true);
}

SetString::SetString(QString* pointer, QString new_value) :
  p(pointer),
  oldval(*pointer),
  newval(new_value),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetString::undo() {
  *p = oldval;
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetString::redo() {
  *p = newval;
  global::mainWindow->setWindowModified(true);
}

void CloseAllClipsCommand::undo() {
  redo();
}

void CloseAllClipsCommand::redo() {
  if (global::sequence != nullptr) {
    global::sequence->closeActiveClips();
  }
}

UpdateFootageTooltip::UpdateFootageTooltip(MediaPtr i) :
  item(i)
{}

void UpdateFootageTooltip::undo() {
  redo();
}

void UpdateFootageTooltip::redo() {
  item->updateTooltip();
}

MoveEffectCommand::MoveEffectCommand() :
  old_project_changed(global::mainWindow->isWindowModified())
{}

void MoveEffectCommand::undo() {
  clip->effects.move(to, from);
  global::mainWindow->setWindowModified(old_project_changed);
}

void MoveEffectCommand::redo() {
  clip->effects.move(from, to);
  global::mainWindow->setWindowModified(true);
}

RemoveClipsFromClipboard::RemoveClipsFromClipboard(int index) :
  pos(index),
  old_project_changed(global::mainWindow->isWindowModified()),
  done(false)
{}

RemoveClipsFromClipboard::~RemoveClipsFromClipboard() {

}

void RemoveClipsFromClipboard::undo() {
  e_clipboard.insert(pos, clip);
  done = false;
}

void RemoveClipsFromClipboard::redo() {
  clip = std::dynamic_pointer_cast<Clip>(e_clipboard.at(pos));
  e_clipboard.removeAt(pos);
  done = true;
}

RenameClipCommand::RenameClipCommand() :
  old_project_changed(global::mainWindow->isWindowModified())
{}

void RenameClipCommand::undo() {
  for (int i=0;i<clips.size();i++) {
    clips.at(i)->setName(old_names.at(i));
  }
}

void RenameClipCommand::redo() {
  old_names.resize(clips.size());
  for (int i=0;i<clips.size();i++) {
    old_names[i] = clips.at(i)->name();
    clips.at(i)->setName(new_name);
  }
}

//SetPointer::SetPointer(void **pointer, void *data) :
//  old_changed(global::mainWindow->isWindowModified()),
//  p(pointer),
//  new_data(data)
//{

//}

//void SetPointer::undo() {
//  *p = old_data;
//  global::mainWindow->setWindowModified(old_changed);
//}

//void SetPointer::redo() {
//  old_data = *p;
//  *p = new_data;
//  global::mainWindow->setWindowModified(true);
//}

void ReloadEffectsCommand::undo() {
  redo();
}

void ReloadEffectsCommand::redo() {
  e_panel_effect_controls->reload_clips();
}

RippleAction::RippleAction(SequencePtr is, const long ipoint, const long ilength, const QVector<int> &iignore) :
  s(is),
  point(ipoint),
  length(ilength),
  ignore(iignore)
{}

void RippleAction::undo() {
  ca->undo();
  delete ca;
}

void RippleAction::redo() {
  ca = new ComboAction();
  for (int i=0;i<s->clips_.size();i++) {
    if (!ignore.contains(i)) {
      ClipPtr   c = s->clips_.at(i);
      if (c != nullptr) {
        if (c->timeline_info.in >= point) {
          move_clip(ca, c, length, length, 0, 0, true, true);
        }
      }
    }
  }
  ca->redo();
}

SetDouble::SetDouble(double* pointer, double old_value, double new_value) :
  p(pointer),
  oldval(old_value),
  newval(new_value),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetDouble::undo() {
  *p = oldval;
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetDouble::redo() {
  *p = newval;
  global::mainWindow->setWindowModified(true);
}

SetQVariant::SetQVariant(QVariant *itarget, const QVariant &iold, const QVariant &inew) :
  target(itarget),
  old_val(iold),
  new_val(inew)
{}

void SetQVariant::undo() {
  *target = old_val;
}

void SetQVariant::redo() {
  *target = new_val;
}

SetLong::SetLong(long *pointer, const long old_value, const long new_value) :
  p(pointer),
  oldval(old_value),
  newval(new_value),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void SetLong::undo() {
  *p = oldval;
  global::mainWindow->setWindowModified(old_project_changed);
}

void SetLong::redo() {
  *p = newval;
  global::mainWindow->setWindowModified(true);
}

KeyframeFieldSet::KeyframeFieldSet(EffectField* ifield, const int ii) :
  field(ifield),
  index(ii),
  key(ifield->keyframes.at(ii)),
  done(true),
  old_project_changed(global::mainWindow->isWindowModified())
{}

void KeyframeFieldSet::undo() {
  field->keyframes.removeAt(index);
  global::mainWindow->setWindowModified(old_project_changed);
  done = false;
}

void KeyframeFieldSet::redo() {
  if (!done) {
    field->keyframes.insert(index, key);
    global::mainWindow->setWindowModified(true);
  }
  done = true;
}

SetKeyframing::SetKeyframing(EffectRow *irow, const bool ib) :
  row(irow),
  b(ib)
{}

void SetKeyframing::undo() {
  row->setKeyframing(!b);
}

void SetKeyframing::redo() {
  row->setKeyframing(b);
}

RefreshClips::RefreshClips(MediaPtr m) :
  media(m)
{}

void RefreshClips::undo() {
  redo();
}

void RefreshClips::redo() {
  // close any clips currently using this media
  QVector<MediaPtr> all_sequences = e_panel_project->list_all_project_sequences();
  for (int i=0;i<all_sequences.size();i++) {
    SequencePtr s = all_sequences.at(i)->object<Sequence>();
    for (int j=0;j<s->clips_.size();j++) {
      ClipPtr   c = s->clips_.at(j);
      if (c != nullptr && c->timeline_info.media == media) {
        c->replaced = true;
        c->refresh();
      }
    }
  }
}
