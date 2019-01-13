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
#ifndef UNDO_H
#define UNDO_H

#include <QUndoStack>
#include <QUndoCommand>
#include <QVector>
#include <QVariant>
#include <QModelIndex>
#include <QCheckBox>

#include "project/marker.h"
#include "project/selection.h"
#include "project/effectfield.h"
#include "project/sequence.h"
#include "project/effect.h"
#include "project/clip.h"
#include "project/media.h"

class LabelSlider;
class SourceTable;
class EffectRow;
class EffectField;
class Transition;
class EffectGizmo;

class Footage;
struct EffectMeta;


extern QUndoStack e_undo_stack;

class ComboAction : public QUndoCommand {
public:
    ComboAction();
    virtual ~ComboAction();
    virtual void undo();
    virtual void redo();
    void append(QUndoCommand* u);
    void appendPost(QUndoCommand* u);
private:
    QVector<QUndoCommand*> commands;
    QVector<QUndoCommand*> post_commands;
};

class MoveClipAction : public QUndoCommand {
public:
    MoveClipAction(ClipPtr c, const long iin, const long iout, const long iclip_in, const int itrack, const bool irelative);
    virtual ~MoveClipAction();
    virtual void undo();
    virtual void redo();
private:
    ClipPtr clip;

    long old_in;
    long old_out;
    long old_clip_in;
    int old_track;

    long new_in;
    long new_out;
    long new_clip_in;
    int new_track;

    bool relative;

    bool old_project_changed;
};

class RippleAction : public QUndoCommand {
public:
    RippleAction(SequencePtr is, const long ipoint, const long ilength, const QVector<int>& iignore);
    virtual void undo();
    virtual void redo();
private:
    SequencePtr s;
    long point;
    long length;
    QVector<int> ignore;
    ComboAction* ca;
};

class DeleteClipAction : public QUndoCommand {
public:
    DeleteClipAction(SequencePtr s, const int clip);
    virtual ~DeleteClipAction();
    virtual void undo();
    virtual void redo();
private:
    SequencePtr seq;
    ClipPtr ref;
    int index;

    int opening_transition;
    int closing_transition;

    QVector<int> linkClipIndex;
    QVector<int> linkLinkIndex;

    bool old_project_changed;
};

class ChangeSequenceAction : public QUndoCommand {
public:
    ChangeSequenceAction(SequencePtr s);
    virtual void undo();
    virtual void redo();
private:
    SequencePtr old_sequence;
    SequencePtr new_sequence;
};

class AddEffectCommand : public QUndoCommand {
public:
    AddEffectCommand(ClipPtr c, EffectPtr e, const EffectMeta* m, const int insert_pos = -1);
    virtual ~AddEffectCommand();
    virtual void undo();
    virtual void redo();
private:
    ClipPtr clip;
    const EffectMeta* meta;
    EffectPtr ref;
    int pos;
    bool done;
    bool old_project_changed;
};

class AddTransitionCommand : public QUndoCommand {
public:
    AddTransitionCommand(ClipPtr c, ClipPtr s, Transition *copy, const EffectMeta* itransition, const int itype, const int ilength);
    virtual void undo();
    virtual void redo();
private:
    ClipPtr clip;
    ClipPtr secondary;
    Transition* transition_to_copy;
    const EffectMeta* transition;
    int type;
    int length;
    bool old_project_changed;
    int old_ptransition;
    int old_stransition;
};

class ModifyTransitionCommand : public QUndoCommand {
public:
    ModifyTransitionCommand(ClipPtr c, const int itype, const long ilength);
    virtual void undo();
    virtual void redo();
private:
    ClipPtr clip;
    int type;
    long new_length;
    long old_length;
    bool old_project_changed;
};

class DeleteTransitionCommand : public QUndoCommand {
public:
    DeleteTransitionCommand(SequencePtr s, const int transition_index);
    virtual ~DeleteTransitionCommand();
    virtual void undo();
    virtual void redo();
private:
    SequencePtr seq;
    int index;
    Transition* transition;
    ClipPtr otc;
    ClipPtr ctc;
    bool old_project_changed;
};

class SetTimelineInOutCommand : public QUndoCommand {
public:
    SetTimelineInOutCommand(SequencePtr s, const bool enabled, const long in, const long out);
    virtual void undo();
    virtual void redo();
private:
    SequencePtr seq;

    bool old_workarea_enabled;

    bool old_enabled;
    long old_in;
    long old_out;

    bool new_enabled;
    long new_in;
    long new_out;

    bool old_project_changed;
};

class NewSequenceCommand : public QUndoCommand {
public:
    NewSequenceCommand(MediaPtr s, MediaPtr iparent);
    virtual ~NewSequenceCommand();
    virtual void undo();
    virtual void redo();
private:
    MediaPtr seq;
    MediaPtr parent;
    bool done;
    bool old_project_changed;
};

class AddMediaCommand : public QUndoCommand {
public:
    AddMediaCommand(MediaPtr iitem, MediaPtr iparent);
    virtual ~AddMediaCommand();
    virtual void undo();
    virtual void redo();
private:
    MediaPtr item;
    MediaPtr parent;
    bool done;
    bool old_project_changed;
};

class DeleteMediaCommand : public QUndoCommand {
public:
    DeleteMediaCommand(MediaPtr i);
    virtual ~DeleteMediaCommand();
    virtual void undo();
    virtual void redo();
private:
    MediaPtr item;
    MediaPtr parent;
    bool old_project_changed;
    bool done;
};

class AddClipCommand : public QUndoCommand {
public:
    AddClipCommand(SequencePtr s, QVector<ClipPtr>& add);
    virtual ~AddClipCommand();
    virtual void undo();
    virtual void redo();
private:
    SequencePtr seq;
    QVector<ClipPtr> clips;
    QVector<ClipPtr> undone_clips;
    bool old_project_changed;
};

class LinkCommand : public QUndoCommand {
public:
    LinkCommand();
    virtual void undo();
    virtual void redo();
    SequencePtr s;
    QVector<int> clips;
    bool link;
private:
    QVector< QVector<int> > old_links;
    bool old_project_changed;
};

class CheckboxCommand : public QUndoCommand {
public:
    CheckboxCommand(QCheckBox* b);
    virtual ~CheckboxCommand();
    virtual void undo();
    virtual void redo();
private:
    QCheckBox* box;
    bool checked;
    bool done;
    bool old_project_changed;
};

class ReplaceMediaCommand : public QUndoCommand {
public:
    ReplaceMediaCommand(MediaPtr, QString);
    virtual void undo();
    virtual void redo();
private:
    MediaPtr item;
    QString old_filename;
    QString new_filename;
    bool old_project_changed;
    void replace(QString& filename);
};

class ReplaceClipMediaCommand : public QUndoCommand {
public:
    ReplaceClipMediaCommand(MediaPtr , MediaPtr , const bool);
    virtual void undo();
    virtual void redo();
    QVector<ClipPtr> clips;
private:
    MediaPtr old_media;
    MediaPtr new_media;
    bool preserve_clip_ins;
    bool old_project_changed;
    QVector<int> old_clip_ins;
    void replace(bool undo);
};

class EffectDeleteCommand : public QUndoCommand {
public:
    EffectDeleteCommand();
    virtual ~EffectDeleteCommand();
    virtual void undo();
    virtual void redo();
    QVector<ClipPtr> clips;
    QVector<int> fx;
private:
    bool done;
    bool old_project_changed;
    QVector<EffectPtr> deleted_objects;
};

class MediaMove : public QUndoCommand {
public:
    MediaMove();
    QVector<MediaPtr> items;
    MediaPtr to;
    virtual void undo();
    virtual void redo();
private:
    QVector<MediaPtr> froms;
    bool old_project_changed;
};

class MediaRename : public QUndoCommand {
public:
    MediaRename(MediaPtr iitem, QString to);
    virtual void undo();
    virtual void redo();
private:
    bool old_project_changed;
    MediaPtr item;
    QString from;
    QString to;
};

class KeyframeDelete : public QUndoCommand {
public:
    KeyframeDelete(EffectFieldPtr ifield, const int iindex);
    virtual void undo();
    virtual void redo();
private:
    EffectFieldPtr field;
    int index;
    bool done;
    EffectKeyframe deleted_key;
    bool old_project_changed;
};

// a more modern version of the above, could probably replace it
// assumes the keyframe already exists
class KeyframeFieldSet : public QUndoCommand {
public:
    KeyframeFieldSet(EffectFieldPtr ifield, const int ii);
    virtual void undo();
    virtual void redo();
private:
    EffectFieldPtr field;
    int index;
    EffectKeyframe key;
    bool done;
    bool old_project_changed;
};

class EffectFieldUndo : public QUndoCommand {
public:
    EffectFieldUndo(EffectFieldPtr field);
    virtual void undo();
    virtual void redo();
private:
    EffectFieldPtr field;
    QVariant old_val;
    QVariant new_val;
    bool done;
    bool old_project_changed;
};

class SetAutoscaleAction : public QUndoCommand {
public:
    SetAutoscaleAction();
    virtual void undo();
    virtual void redo();
    QVector<ClipPtr> clips;
private:
    bool old_project_changed;
};

class AddMarkerAction : public QUndoCommand {
public:
    AddMarkerAction(SequencePtr s, const long t, QString n);
    virtual void undo();
    virtual void redo();
private:
    SequencePtr seq;
    long time;
    QString name;
    QString old_name;
    bool old_project_changed;
    int index;
};

class MoveMarkerAction : public QUndoCommand {
public:
    MoveMarkerAction(Marker* m, const long o, const long n);
    virtual void undo();
    virtual void redo();
private:
    Marker* marker;
    long old_time;
    long new_time;
    bool old_project_changed;
};

class DeleteMarkerAction : public QUndoCommand {
public:
    DeleteMarkerAction(SequencePtr s);
    virtual void undo();
    virtual void redo();
    QVector<int> markers;
private:
    SequencePtr seq;
    QVector<Marker> copies;
    bool sorted;
    bool old_project_changed;
};

class SetSpeedAction : public QUndoCommand {
public:
    SetSpeedAction(ClipPtr c, double speed);
    virtual void undo();
    virtual void redo();
private:
    ClipPtr clip;
    double old_speed;
    double new_speed;
    bool old_project_changed;
};

class SetBool : public QUndoCommand {
public:
    SetBool(bool* b, const bool setting);
    virtual void undo();
    virtual void redo();
private:
    bool* boolean;
    bool old_setting;
    bool new_setting;
    bool old_project_changed;
};

class SetSelectionsCommand : public QUndoCommand {
public:
    SetSelectionsCommand(SequencePtr s);
    virtual void undo();
    virtual void redo();
    QVector<Selection> old_data;
    QVector<Selection> new_data;
private:
    SequencePtr seq;
    bool done;
    bool old_project_changed;
};

class SetEnableCommand : public QUndoCommand {
public:
    SetEnableCommand(ClipPtr c, const bool enable);
    virtual void undo();
    virtual void redo();
private:
    ClipPtr clip;
    bool old_val;
    bool new_val;
    bool old_project_changed;
};

class EditSequenceCommand : public QUndoCommand {

public:
    EditSequenceCommand(MediaPtr i, SequencePtr s);
    virtual void undo();
    virtual void redo();
    void update();

    QString name;
    int width;
    int height;
    double frame_rate;
    int audio_frequency;
    int audio_layout;
private:
    MediaPtr item;
    SequencePtr seq;
    bool old_project_changed;

    QString old_name;
    int old_width;
    int old_height;
    double old_frame_rate;
    int old_audio_frequency;
    int old_audio_layout;
};

class SetInt : public QUndoCommand {
public:
    SetInt(int* pointer, const int new_value);
    virtual void undo();
    virtual void redo();
private:
    int* p;
    int oldval;
    int newval;
    bool old_project_changed;
};

class SetLong : public QUndoCommand {
public:
    SetLong(long* pointer, const long old_value, const long new_value);
    virtual void undo();
    virtual void redo();
private:
    long* p;
    long oldval;
    long newval;
    bool old_project_changed;
};

class SetDouble : public QUndoCommand {
public:
    SetDouble(double* pointer, double old_value, double new_value);
    virtual void undo();
    virtual void redo();
private:
    double* p;
    double oldval;
    double newval;
    bool old_project_changed;
};

class SetString : public QUndoCommand {
public:
    SetString(QString* pointer, QString new_value);
    virtual void undo();
    virtual void redo();
private:
    QString* p;
    QString oldval;
    QString newval;
    bool old_project_changed;
};

class CloseAllClipsCommand : public QUndoCommand {
public:
    virtual void undo();
    virtual void redo();
};

class UpdateFootageTooltip : public QUndoCommand {
public:
    UpdateFootageTooltip(MediaPtr i);
    virtual void undo();
    virtual void redo();
private:
    MediaPtr item;
};

class MoveEffectCommand : public QUndoCommand {
public:
    MoveEffectCommand();
    virtual void undo();
    virtual void redo();
    ClipPtr clip;
    int from;
    int to;
private:
    bool old_project_changed;
};

class RemoveClipsFromClipboard : public QUndoCommand {
public:
    RemoveClipsFromClipboard(int index);
    virtual ~RemoveClipsFromClipboard();
    virtual void undo();
    virtual void redo();
private:
    int pos;
    ClipPtr clip;
    bool old_project_changed;
    bool done;
};

class RenameClipCommand : public QUndoCommand {
public:
    RenameClipCommand();
    QVector<ClipPtr> clips;
    QString new_name;
    virtual void undo();
    virtual void redo();
private:
    QVector<QString> old_names;
    bool old_project_changed;
};

class SetPointer : public QUndoCommand {
public:
    SetPointer(void** pointer, void* data);
    virtual void undo();
    virtual void redo();
private:
    bool old_changed;
    void** p;
    void* new_data;
    void* old_data;
};

class ReloadEffectsCommand : public QUndoCommand {
public:
    virtual void undo();
    virtual void redo();
};

class SetQVariant : public QUndoCommand {
public:
    SetQVariant(QVariant* itarget, const QVariant& iold, const QVariant& inew);
    virtual void undo();
    virtual void redo();
private:
    QVariant* target;
    QVariant old_val;
    QVariant new_val;
};

class SetKeyframing : public QUndoCommand {
public:
    SetKeyframing(EffectRow* irow, const bool ib);
    virtual void undo();
    virtual void redo();
private:
    EffectRow* row;
    bool b;
};

class RefreshClips : public QUndoCommand {
public:
    RefreshClips(MediaPtr m);
    virtual void undo();
    virtual void redo();
private:
    MediaPtr media;
};

#endif // UNDO_H
