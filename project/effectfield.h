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
#ifndef EFFECTFIELD_H
#define EFFECTFIELD_H

//#define EffectFieldType::DOUBLE 0
//#define EffectFieldType::COLOR 1
//#define EffectFieldType::STRING 2
//#define EffectFieldType::BOOL 3
//#define EffectFieldType::COMBO 4
//#define EffectFieldType::FONT 5
//#define EffectFieldType::FILE 6

#include <QObject>
#include <QVariant>
#include <QVector>
#include <memory>

#include "keyframe.h"

class EffectRow;
class ComboAction;

enum class EffectFieldType {
    DOUBLE = 0,
    COLOR = 1,
    STRING = 2,
    BOOL = 3,
    COMBO = 4,
    FONT = 5,
    FILE_T = 6,
    UNKNOWN
};

class EffectField : public QObject {
    Q_OBJECT
public:
    EffectField(EffectRow* parent, const EffectFieldType t, const QString& i);

    EffectField() = delete;
    EffectField(const EffectField&) = delete;
    EffectField(const EffectField&&) = delete;
    EffectField& operator=(const EffectField&) = delete;
    EffectField& operator=(const EffectField&&) = delete;

    EffectRow* parent_row;
    EffectFieldType type;
    QString id;

    QVariant get_previous_data();
    QVariant get_current_data();
    double frameToTimecode(long frame);
    long timecodeToFrame(double timecode);
    void set_current_data(const QVariant&);
    void get_keyframe_data(double timecode, int& before, int& after, double& d);
    QVariant validate_keyframe_data(double timecode, bool async = false);

    double get_double_value(double timecode, bool async = false);
    void set_double_value(double v);
    void set_double_default_value(double v);
    void set_double_minimum_value(double v);
    void set_double_maximum_value(double v);

    QString get_string_value(double timecode, bool async = false);
    void set_string_value(const QString &s);

    void add_combo_item(const QString& name, const QVariant &data);
    int get_combo_index(double timecode, bool async = false);
    QVariant get_combo_data(double timecode);
    QString get_combo_string(double timecode);
    void set_combo_index(int index);
    void set_combo_string(const QString& s);

    bool get_bool_value(double timecode, bool async = false);
    void set_bool_value(bool b);

    QString get_font_name(double timecode, bool async = false);
    void set_font_name(const QString& s);

    QColor get_color_value(double timecode, bool async = false);
    void set_color_value(QColor color);

    QString get_filename(double timecode, bool async = false);
    void set_filename(const QString& s);

    QWidget* get_ui_element();
    void set_enabled(bool e);
    QVector<EffectKeyframe> keyframes;
    QWidget* ui_element = nullptr;

    void make_key_from_change(ComboAction* ca);
public slots:
    void ui_element_change();
private:
    bool hasKeyframes();
signals:
    void changed();
    void toggled(bool);
    void clicked();
};

#endif // EFFECTFIELD_H
