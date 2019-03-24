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
#ifndef EFFECTCONTROLS_H
#define EFFECTCONTROLS_H

#include <QDockWidget>
#include <QUndoCommand>
#include <QMutex>
#include <QSplitter>

#include "project/effect.h"

class Clip;
class QMenu;
class TimelineHeader;
class QScrollArea;
class KeyframeView;
class QVBoxLayout;
class ResizableScrollBar;
class QLabel;
class KeyframeView;
class QScrollBar;
class QHBoxLayout;

class EffectsArea : public QWidget {
  public:
    explicit EffectsArea(QWidget* parent = nullptr);
    ~EffectsArea() override;
    EffectsArea(const EffectsArea& ) = delete;
    EffectsArea(const EffectsArea&& ) = delete;
    EffectsArea& operator=(const EffectsArea&) = delete;
    EffectsArea& operator=(const EffectsArea&&) = delete;

    QScrollArea* parent_widget = nullptr;
    KeyframeView* keyframe_area = nullptr;
    TimelineHeader* header = nullptr;
  protected:
    void resizeEvent(QResizeEvent *event) override;
};

class EffectControls : public QDockWidget
{
    Q_OBJECT

  public:
    explicit EffectControls(QWidget *parent = nullptr);
    virtual ~EffectControls();

    EffectControls(const EffectControls& ) = delete;
    EffectControls(const EffectControls&& ) = delete;
    EffectControls& operator=(const EffectControls&) = delete;
    EffectControls& operator=(const EffectControls&&) = delete;

    void set_clips(QVector<int>& clips, int mode);
    void clear_effects(bool clear_cache);
    void delete_effects();
    bool is_focused();
    void reload_clips();
    void set_zoom(bool in);
    bool keyframe_focus();
    void delete_selected_keyframes();
    void copy(bool del);
    bool multiple;
    void scroll_to_frame(long frame);

    QVector<int> selected_clips;

    double zoom;

    ResizableScrollBar* horizontalScrollBar;
    QScrollBar* verticalScrollBar;

    QMutex effects_loaded;


  public slots:
    void update_keyframes();
  private slots:
    void menu_select(QAction* q);

    void video_effect_click();
    void audio_effect_click();
    void video_transition_click();
    void audio_transition_click();

    void deselect_all_effects(QWidget*);
  protected:
    void resizeEvent(QResizeEvent *event);
  private:
    void show_effect_menu(int type, int subtype);
    void load_effects();
    void load_keyframes();
    void open_effect(QVBoxLayout* const hlayout, const EffectPtr& e);

    void setup_ui();


    int effect_menu_type;
    int effect_menu_subtype;
    QString panel_name;
    int mode;

    TimelineHeader* headers;
    EffectsArea* effects_area;
    QScrollArea* scrollArea;
    QLabel* lblMultipleClipsSelected;
    KeyframeView* keyframeView;
    QWidget* video_effect_area;
    QWidget* audio_effect_area;
    QWidget* vcontainer;
    QWidget* acontainer;
    QSplitter* splitter_ = nullptr;
};

#endif // EFFECTCONTROLS_H
