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
#include "effectrow.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "project/undo.h"
#include "project/clip.h"
#include "project/sequence.h"
#include "panels/panelmanager.h"
#include "effect.h"
#include "ui/viewerwidget.h"
#include "ui/keyframenavigator.h"
#include "ui/clickablelabel.h"

using panels::PanelManager;



EffectRow::EffectRow(Effect* parent_eff, const bool save, QGridLayout& uilayout,
                     const QString& n, const int row, const bool keyframable)
  : QObject(parent_eff),
    parent_effect(parent_eff),
    label(new ClickableLabel(n + ":")),
    saveable(save),
    ui(uilayout),
    name(n),
    ui_row(row),
    keyframable_(keyframable)
{
  setupUi();
}

EffectRow::~EffectRow()
{
  label = nullptr;
}

bool EffectRow::isKeyframing() {
  return keyframing;
}

void EffectRow::setKeyframing(bool b)
{
  Q_ASSERT(parent_effect != nullptr);
  if (parent_effect->meta.type  != EFFECT_TYPE_TRANSITION) {
    keyframing = b;
    if (keyframe_nav != nullptr) {
      keyframe_nav->enable_keyframes(b);
    }
  }
}




bool EffectRow::load(QXmlStreamReader& stream)
{
  // setup row's fields
  while (stream.readNextStartElement()) {
    auto name = stream.name().toString().toLower();
    if (name == "field") {
      if (stream.readNextStartElement()) {
        name = stream.name().toString().toLower();
        if (name == "name") {
          const auto fld_name = stream.readElementText();
          if (auto fld = field(fld_name)) {
            if (!fld->load(stream)) {
              return false;
            }
          } else {
            qCritical() << "Unknown Field" << fld_name;
            return false;
          }
        } else {
          qCritical() << "Unexpected element" << name;
          return false;
        }
      }
    } else {
      qCritical() << "Unexpected element" << name;
      return false;
    }
  }
  return true;
}
bool EffectRow::save(QXmlStreamWriter& stream) const
{
  stream.writeStartElement("row");
  stream.writeTextElement("name", name);

  for (const auto& field : fields) {
    if (!field->save(stream)) {
      qCritical() << "Failed to save EffectField";
      return false;
    }
  }
  stream.writeEndElement();
  return true;
}

void EffectRow::set_keyframe_enabled(bool enabled) {
  if (enabled) {
    auto ca = new ComboAction();
    ca->append(new SetKeyframing(this, true));
    set_keyframe_now(ca);
    e_undo_stack.push(ca);
  } else {
    const auto msg = QMessageBox::question(&PanelManager::fxControls(),
                                           tr("Disable Keyframes"),
                                           tr("Disabling keyframes will delete all current keyframes. Are you sure you want to do this?"),
                                           QMessageBox::Yes, QMessageBox::No);
    if (msg == QMessageBox::Yes) {
      // clear
      auto ca = new ComboAction();
      for (int i=0;i<fieldCount();i++) {
        auto f = field(i);
        for (int j=0;j<f->keyframes.size();j++) {
          ca->append(new KeyframeDelete(f, 0));
        }
      }
      ca->append(new SetKeyframing(this, false));
      e_undo_stack.push(ca);
      PanelManager::fxControls().update_keyframes();
    } else {
      setKeyframing(true);
    }
  }
}

void EffectRow::goto_previous_key()
{
  Q_ASSERT(parent_effect != nullptr);
  long key = LONG_MIN;
  const ClipPtr& c = parent_effect->parent_clip;
  for (int i=0; i < fieldCount(); ++i) {
    EffectField* f = field(i);
    for (const auto& kf : f->keyframes) {
      long comp = kf.time - c->timeline_info.clip_in + c->timeline_info.in;
      if (comp < global::sequence->playhead_) {
        key = qMax(comp, key);
      }
    }
  }
  if (key != LONG_MIN) {
    PanelManager::sequenceViewer().seek(key);
  }
}

void EffectRow::toggle_key()
{
  Q_ASSERT(parent_effect != nullptr);
  QVector<EffectField*> key_fields;
  QVector<int> key_field_index;
  ClipPtr c = parent_effect->parent_clip;
  if (c == nullptr) {
    qCritical() << "Null Clip instance";
    return;
  }
  for (const auto f : fields) {
    if (f == nullptr) {
      qWarning() << "Null Effect field insance";
      continue;
    }
    for (int i=0; i < f->keyframes.size();i++) {
      long comp = c->timeline_info.in - c->timeline_info.clip_in + f->keyframes.at(i).time;
      if (comp == global::sequence->playhead_) {
        key_fields.append(f);
        key_field_index.append(i);
      }
    }
  }

  auto ca = new ComboAction();
  if (key_fields.empty()) {
    // keyframe doesn't exist, set one
    set_keyframe_now(ca);
  } else {
    for (int i=0;i<key_fields.size();i++) {
      ca->append(new KeyframeDelete(key_fields.at(i), key_field_index.at(i)));
    }
  }
  e_undo_stack.push(ca);
  PanelManager::refreshPanels(false);
}

void EffectRow::goto_next_key()
{
  Q_ASSERT(parent_effect != nullptr);
  long key = LONG_MAX;

  ClipPtr c = parent_effect->parent_clip;
  if (c == nullptr) {
    qCritical() << "Null Clip instance";
    return;
  }
  for (const auto f : fields) {
    if (f == nullptr) {
      qWarning() << "Null Effect field insance";
      continue;
    }
    for (const auto& kf : f->keyframes) {
      const long comp = kf.time - c->timeline_info.clip_in + c->timeline_info.in;
      if (comp > global::sequence->playhead_) {
        key = qMin(comp, key);
      }
    }
  }
  if (key != LONG_MAX) {
    PanelManager::sequenceViewer().seek(key);
  }
}

void EffectRow::focus_row()
{
  panels::PanelManager::graphEditor().set_row(this);
}

EffectField* EffectRow::add_field(const EffectFieldType type, const QString& id, int colspan)
{
  Q_ASSERT(parent_effect != nullptr);

  auto field = new EffectField(this, type, id);
  if (parent_effect->meta.type  != EFFECT_TYPE_TRANSITION) {
    connect(field, SIGNAL(clicked()), this, SLOT(focus_row()));
  }
  fields.append(field);
  QWidget* element = field->get_ui_element();
  ui.addWidget(element, ui_row, column_count, 1, colspan);
  column_count++;
  connect(field, SIGNAL(changed()), parent_effect, SLOT(field_changed()));
  return field;
}

void EffectRow::add_widget(QWidget* w)
{
  ui.addWidget(w, ui_row, column_count);
  column_count++;
}



void EffectRow::set_keyframe_now(ComboAction* ca)
{
  long time = global::sequence->playhead_-parent_effect->parent_clip->timeline_info.in
      + parent_effect->parent_clip->timeline_info.clip_in;

  if (!just_made_unsafe_keyframe) {

    unsafe_keys.resize(fieldCount());
    unsafe_old_data.resize(fieldCount());
    key_is_new.resize(fieldCount());

    for (int i=0;i<fieldCount();i++) {
      EffectField* f = field(i);
      EffectKeyframe key(f);
      key.time = time;

      int exist_key = -1;
      int closest_key = 0;
      for (int j=0;j<f->keyframes.size();j++) {
        if (f->keyframes.at(j).time == time) {
          exist_key = j;
        } else if (f->keyframes.at(j).time < time
                   && f->keyframes.at(closest_key).time < f->keyframes.at(j).time) {
          closest_key = j;
        }
      }
      if (exist_key == -1) {
        key.type = f->keyframes.empty() ? KeyframeType::LINEAR : f->keyframes.at(closest_key).type;
        key.data = f->get_current_data();//f->keyframes.at(closest_key).data;
        unsafe_keys[i] = f->keyframes.size();
        f->keyframes.append(key);
        key_is_new[i] = true;
      } else {
        unsafe_keys[i] = exist_key;
        key_is_new[i] = false;
      }
      unsafe_old_data[i] = f->get_current_data();
    }
    just_made_unsafe_keyframe = true;
  }

  for (int i=0;i<fieldCount();i++) {
    field(i)->keyframes[unsafe_keys.at(i)].data = field(i)->get_current_data();
  }

  if (ca != nullptr)	{
    for (int i=0;i<fieldCount();i++) {
      if (key_is_new.at(i)) ca->append(new KeyframeFieldSet(field(i), unsafe_keys.at(i)));
      ca->append(new SetQVariant(&field(i)->keyframes[unsafe_keys.at(i)].data, unsafe_old_data.at(i), field(i)->get_current_data()));
    }
    unsafe_keys.clear();
    unsafe_old_data.clear();
    just_made_unsafe_keyframe = false;
  }

  PanelManager::fxControls().update_keyframes();





  /*if (ca != nullptr) {
        just_made_unsafe_keyframe = false;
    } else {
        if (!just_made_unsafe_keyframe) {
            just_made_unsafe_keyframe = true;
        }
    }*/


  /*int index = -1;
    long time = sequence->playhead-parent_effect->parent_clip->timeline_in+parent_effect->parent_clip->timeline_info.clip_in;
    for (int j=0;j<fieldCount();j++) {
        EffectFieldUPtr f = field(j);
        for (int i=0;i<f->keyframes.size();i++) {
            if (f->keyframes.at(i).time == time) {
                index = i;
                break;
            }
        }
    }

    KeyframeSet* ks = new KeyframeSet(this, index, time, just_made_unsafe_keyframe);

    if (ca != nullptr) {
        just_made_unsafe_keyframe = false;
        ca->append(ks);
    } else {
        if (index == -1) just_made_unsafe_keyframe = true;
        ks->redo();
        delete ks;
    }

    panel_effect_controls->update_keyframes();*/
}

void EffectRow::delete_keyframe_at_time(ComboAction* ca, long time) {
  for (int j=0;j<fieldCount();j++) {
    EffectField* f = field(j);
    for (int i=0;i<f->keyframes.size();i++) {
      if (f->keyframes.at(i).time == time) {
        ca->append(new KeyframeDelete(f, i));
        break;
      }
    }
  }
}

const QString &EffectRow::get_name() {
  return name;
}

EffectField* EffectRow::field(const int index) {
  return fields.at(index);
}


EffectField* EffectRow::field(const QString& name)
{
  for (auto fld : fields) {
    if ( (fld != nullptr) && (fld->id == name) ) {
      return fld;
    }
  }
  return nullptr;
}


const QVector<EffectField*>& EffectRow::getFields() const
{
  return fields;
}

int EffectRow::fieldCount() {
  return fields.size();
}

void EffectRow::setupUi()
{
  ui.addWidget(label, ui_row, 0);

  if ( (parent_effect != nullptr) && (parent_effect->meta.type >= 0)
       && (parent_effect->meta.type != EFFECT_TYPE_TRANSITION)
       && keyframable_) {
    qDebug() << "Setup of valid, keyframable transition";
    connect(label, SIGNAL(clicked()), this, SLOT(focus_row()));

    keyframe_nav = new KeyframeNavigator();
    connect(keyframe_nav, SIGNAL(goto_previous_key()), this, SLOT(goto_previous_key()));
    connect(keyframe_nav, SIGNAL(toggle_key()), this, SLOT(toggle_key()));
    connect(keyframe_nav, SIGNAL(goto_next_key()), this, SLOT(goto_next_key()));
    connect(keyframe_nav, SIGNAL(keyframe_enabled_changed(bool)), this, SLOT(set_keyframe_enabled(bool)));
    connect(keyframe_nav, SIGNAL(clicked()), this, SLOT(focus_row()));
    ui.addWidget(keyframe_nav, ui_row, 6);
  }
}

