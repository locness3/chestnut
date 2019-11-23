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
#ifndef EFFECTROW_H
#define EFFECTROW_H

#include <QObject>
#include <QVector>
#include <QGridLayout>
#include <memory>

#include "project/effectfield.h"
#include "project/ixmlstreamer.h"
#include "database.h"

class Effect;
class ComboAction;
class KeyframeNavigator;
class ClickableLabel;

class EffectRow : public QObject, public project::IXMLStreamer {
  Q_OBJECT
public:
  EffectRow(Effect* parent_eff,
            const bool save,
            QGridLayout& uilayout,
            const QString& n,
            const int row,
            const bool keyframable = true);

  virtual ~EffectRow() override;

  EffectRow(const EffectRow& ) = delete;
  EffectRow& operator=(const EffectRow&) = delete;

  EffectField* add_field(const EffectFieldType type, const QString &id, int colspan = 1);
  void add_widget(QWidget *w);
  EffectField* field(const int index);
  EffectField* field(const QString& name);
  QVector<EffectField*> getFields() const;
  int fieldCount();
  void set_keyframe_now(ComboAction *ca);
  void delete_keyframe_at_time(ComboAction *ca, long time);
  const QString& get_name();

  bool isKeyframing();
  void setKeyframing(bool);

  /**
   * @brief         Set up the row based on stored values
   * @param params  The stored values
   */
  void setValues(const chestnut::ParamsType& params);

  virtual bool load(QXmlStreamReader& stream) override;
  virtual bool save(QXmlStreamWriter& stream) const override;

  Effect* parent_effect;
  ClickableLabel* label = nullptr;
  bool saveable;
  QString name;

public slots:
  void goto_previous_key();
  void toggle_key();
  void goto_next_key();
  void focus_row();
private slots:
  void set_keyframe_enabled(bool);
private:
  bool keyframing{false};
  QGridLayout& ui;
  int ui_row{-1};
  QVector<EffectField*> fields;

  KeyframeNavigator* keyframe_nav{nullptr};

  bool just_made_unsafe_keyframe{false};
  QVector<int> unsafe_keys;
  QVector<QVariant> unsafe_old_data;
  QVector<bool> key_is_new;

  int column_count{1};
  bool keyframable_{false};


  void setupUi();

};

typedef std::shared_ptr<EffectRow> EffectRowPtr;

#endif // EFFECTROW_H
