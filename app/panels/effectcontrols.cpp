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
#include "effectcontrols.h"

#include <QMenu>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QScrollBar>
#include <QSplitter>
#include <QScrollArea>
#include <QPushButton>
#include <QSpacerItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QIcon>

#include "panels/panelmanager.h"
#include "project/effect.h"
#include "project/clip.h"
#include "project/transition.h"
#include "ui/collapsiblewidget.h"
#include "project/sequence.h"
#include "project/undo.h"
#include "ui/viewerwidget.h"
#include "io/clipboard.h"
#include "ui/timelineheader.h"
#include "ui/keyframeview.h"
#include "ui/resizablescrollbar.h"
#include "debug.h"

constexpr const char* const ADD_TRANSITION_ICON = ":/icons/add-transition.png";
constexpr int TRANSITION_LENGTH = 30;

using panels::PanelManager;

EffectControls::EffectControls(QWidget *parent) :
  QDockWidget(parent),
  multiple(false),
  zoom(1),
  panel_name(tr("Effects: ")),
  mode(TA_NO_TRANSITION)
{
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  setup_ui();

  clear_effects(false);
  headers->viewer = &PanelManager::sequenceViewer();
  headers->snapping = false;

  effects_area->parent_widget = scrollArea;
  effects_area->keyframe_area = keyframeView;
  effects_area->header = headers;
  keyframeView->header = headers;

  lblMultipleClipsSelected->setVisible(false);

  connect(horizontalScrollBar, SIGNAL(valueChanged(int)), headers, SLOT(set_scroll(int)));
  connect(horizontalScrollBar, SIGNAL(resize_move(double)), keyframeView, SLOT(resize_move(double)));
  connect(horizontalScrollBar, SIGNAL(valueChanged(int)), keyframeView, SLOT(set_x_scroll(int)));
  connect(verticalScrollBar, SIGNAL(valueChanged(int)), keyframeView, SLOT(set_y_scroll(int)));
  connect(verticalScrollBar, SIGNAL(valueChanged(int)), scrollArea->verticalScrollBar(), SLOT(setValue(int)));
  connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), verticalScrollBar, SLOT(setValue(int)));
}

EffectControls::~EffectControls()
{
  //TODO: check these ptrs to see if they need dealloc
  horizontalScrollBar = nullptr;
  verticalScrollBar = nullptr;
  headers = nullptr;
  effects_area = nullptr;
  scrollArea = nullptr;
  lblMultipleClipsSelected = nullptr;
  keyframeView = nullptr;
  video_effect_area = nullptr;
  audio_effect_area = nullptr;
  vcontainer = nullptr;
  acontainer = nullptr;
}

bool EffectControls::keyframe_focus() {
  return headers->hasFocus() || keyframeView->hasFocus();
}

void EffectControls::set_zoom(bool in) {
  zoom *= (in) ? 2 : 0.5;
  update_keyframes();
}

void EffectControls::menu_select(QAction* q)
{
  auto ca = new ComboAction();
  for (auto clip_id : selected_clips) {
    const ClipPtr& c = global::sequence->clip(clip_id);
    if ( (c != nullptr) &&  (c->timeline_info.isVideo() == (effect_menu_subtype == EFFECT_TYPE_VIDEO)) ) {
      EffectMeta meta = Effect::getRegisteredMeta(q->data().toString());
      if (effect_menu_type == EFFECT_TYPE_TRANSITION) {
        if (c->openingTransition() == nullptr) {
          ca->append(new AddTransitionCommand(c, nullptr, nullptr, meta, TA_OPENING_TRANSITION, TRANSITION_LENGTH));
        }
        if (c->closingTransition() == nullptr) {
          ca->append(new AddTransitionCommand(c, nullptr, nullptr, meta, TA_CLOSING_TRANSITION, TRANSITION_LENGTH));
        }
      } else {
        ca->append(new AddEffectCommand(c, nullptr, meta));
      }
    }
  }
  e_undo_stack.push(ca);
  if (effect_menu_type == EFFECT_TYPE_TRANSITION) {
    PanelManager::refreshPanels(true);
  } else {
    reload_clips();
    PanelManager::sequenceViewer().viewer_widget->frame_update();
  }
}

void EffectControls::update_keyframes() {
  headers->update_zoom(zoom);
  keyframeView->update();
}

void EffectControls::delete_selected_keyframes() {
  keyframeView->delete_selected_keyframes();
}

void EffectControls::copy(bool del)
{
  if (mode == TA_NO_TRANSITION) {
    bool cleared = false;

    auto ca = new ComboAction();
    auto del_com = del ? new EffectDeleteCommand() : nullptr;
    for (auto clip_id : selected_clips) {
      ClipPtr c = global::sequence->clip(clip_id);
      if (c == nullptr) {
        qWarning() << "Null Clip instance";
        continue;
      }
      for (int j=0; j<c->effects.size(); j++) {
        EffectPtr effect = c->effects.at(j);
        if (effect->container->selected) {
          if (!cleared) {
            clear_clipboard();
            cleared = true;
            e_clipboard_type = CLIPBOARD_TYPE_EFFECT;
          }

          e_clipboard.append(std::dynamic_pointer_cast<project::SequenceItem>(effect->copy(nullptr)));

          if (del_com != nullptr) {
            del_com->clips.append(c);
            del_com->fx.append(j);
          }
        }
      }//for
    }

    if (del_com != nullptr) {
      if (!del_com->clips.empty()) {
        ca->append(del_com);
      } else {
        delete del_com;
      }
    }
    e_undo_stack.push(ca);
  }
}

void EffectControls::scroll_to_frame(long frame) {
  scroll_to_frame_internal(horizontalScrollBar, frame, zoom, keyframeView->width());
}

void EffectControls::show_effect_menu(int type, int subtype) {
  effect_menu_type = type;
  effect_menu_subtype = subtype;

  effects_loaded.lock();

  QMenu effects_menu(this);
  for (const auto& em : Effect::getRegisteredMetas()) {
    if (em.type == type && em.subtype == subtype) {
      QAction* action = new QAction(&effects_menu);
      action->setText(em.name);
      action->setData(em.name.toLower());

      QMenu* parent = &effects_menu;
      if (!em.category.isEmpty()) {
        bool found = false;
        for (auto menuAction : effects_menu.actions()) {
          if (!menuAction) {
            continue;
          }
          if (menuAction->menu()->title() == em.category) {
            parent = menuAction->menu();
            found = true;
            break;
          }
        }
        if (!found) {
          parent = new QMenu(&effects_menu);
          parent->setTitle(em.category);

          bool actionFound = false;
          for (auto compAction : effects_menu.actions()) {
            if (!compAction) {
              continue;
            }
            if (compAction->text() > em.category) {
              effects_menu.insertMenu(compAction, parent);
              actionFound = true;
              break;
            }
          }
          if (!actionFound) {
            effects_menu.addMenu(parent);
          }
        }
      }

      bool found = false;
      for (auto compAction : parent->actions()) {
        if (!compAction) {
          continue;
        }
        if (compAction->text() > action->text()) {
          parent->insertAction(compAction, action);
          found = true;
          break;
        }
      }
      if (!found) {
        parent->addAction(action);
      }
    }
  }

  effects_loaded.unlock();

  connect(&effects_menu, SIGNAL(triggered(QAction*)), this, SLOT(menu_select(QAction*)));
  effects_menu.exec(QCursor::pos());
}

void EffectControls::clear_effects(bool clear_cache) {
  // clear existing clips
  deselect_all_effects(nullptr);

  // clear graph editor
  PanelManager::graphEditor().set_row(nullptr);

  QVBoxLayout* video_layout = static_cast<QVBoxLayout*>(video_effect_area->layout());
  QVBoxLayout* audio_layout = static_cast<QVBoxLayout*>(audio_effect_area->layout());
  QLayoutItem* item;
  while ((item = video_layout->takeAt(0))) {
    item->widget()->setParent(nullptr);
    disconnect(static_cast<CollapsibleWidget*>(item->widget()), SIGNAL(deselect_others(QWidget*)), this, SLOT(deselect_all_effects(QWidget*)));
  }
  while ((item = audio_layout->takeAt(0))) {
    item->widget()->setParent(nullptr);
    disconnect(static_cast<CollapsibleWidget*>(item->widget()), SIGNAL(deselect_others(QWidget*)), this, SLOT(deselect_all_effects(QWidget*)));
  }
  lblMultipleClipsSelected->setVisible(false);
  vcontainer->setVisible(false);
  acontainer->setVisible(false);
  headers->setVisible(false);
  keyframeView->setEnabled(false);
  if (clear_cache) {
    selected_clips.clear();
  }
  setWindowTitle(panel_name + "(none)");
}

void EffectControls::deselect_all_effects(QWidget* sender)
{
  for (auto clip_id : selected_clips) {
    if (auto clp = global::sequence->clip(clip_id)) {
      for (auto efct : clp->effects) {
        if (efct && (efct->container != sender) ) {
          efct->container->header_click(false, false);
        }
      }
    } else {
      qWarning() << "Null clip for id" << clip_id;
    }
  }//for
  PanelManager::sequenceViewer().viewer_widget->update();
}

void EffectControls::open_effect(QVBoxLayout* const layout, const EffectPtr& e)
{
  if (e == nullptr) {
    qCritical() << "Null effect ptr";
    return;
  }
  e->setupUi();
  CollapsibleWidget* container = e->container;
  layout->addWidget(container); // FIXME: threading issue with QObject::setParent. container was created in LoadThread
  connect(container, SIGNAL(deselect_others(QWidget*)), this, SLOT(deselect_all_effects(QWidget*)));
}

void EffectControls::setup_ui()
{
  QWidget* contents = new QWidget();

  QHBoxLayout* hlayout = new QHBoxLayout(contents);
  hlayout->setSpacing(0);
  hlayout->setMargin(0);

  auto splitter = new QSplitter(contents);
  splitter->setOrientation(Qt::Horizontal);
  splitter->setChildrenCollapsible(false);

  scrollArea = new QScrollArea(splitter);
  scrollArea->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setFrameShadow(QFrame::Plain);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scrollArea->setWidgetResizable(true);

  QWidget* scrollAreaWidgetContents = new QWidget();

  QHBoxLayout* scrollAreaLayout = new QHBoxLayout(scrollAreaWidgetContents);
  scrollAreaLayout->setSpacing(0);
  scrollAreaLayout->setMargin(0);

  effects_area = new EffectsArea(scrollAreaWidgetContents);

  QVBoxLayout* effects_area_layout = new QVBoxLayout(effects_area);
  effects_area_layout->setSpacing(0);
  effects_area_layout->setMargin(0);

  vcontainer = new QWidget(effects_area);
  QVBoxLayout* vcontainerLayout = new QVBoxLayout(vcontainer);
  vcontainerLayout->setSpacing(0);
  vcontainerLayout->setMargin(0);

  QWidget* veHeader = new QWidget(vcontainer);
  veHeader->setObjectName(QStringLiteral("veHeader"));
  veHeader->setStyleSheet(QLatin1String("#veHeader { background: rgba(0, 0, 0, 0.25); }"));

  QHBoxLayout* veHeaderLayout = new QHBoxLayout(veHeader);
  veHeaderLayout->setSpacing(0);
  veHeaderLayout->setMargin(0);

  QPushButton* btnAddVideoEffect = new QPushButton(veHeader);
  btnAddVideoEffect->setIcon(QIcon(":/icons/add-effect.png"));
  btnAddVideoEffect->setToolTip(tr("Add Video Effect"));
  veHeaderLayout->addWidget(btnAddVideoEffect);
  connect(btnAddVideoEffect, SIGNAL(clicked(bool)), this, SLOT(video_effect_click()));

  veHeaderLayout->addStretch();

  QLabel* lblVideoEffects = new QLabel(veHeader);
  QFont font;
  font.setPointSize(9);
  lblVideoEffects->setFont(font);
  lblVideoEffects->setAlignment(Qt::AlignCenter);
  lblVideoEffects->setText(tr("VIDEO EFFECTS"));
  veHeaderLayout->addWidget(lblVideoEffects);

  veHeaderLayout->addStretch();

  QPushButton* btnAddVideoTransition = new QPushButton(veHeader);
  btnAddVideoTransition->setIcon(QIcon(":/icons/add-transition.png"));
  btnAddVideoTransition->setToolTip(tr("Add Video Transition"));
  connect(btnAddVideoTransition, SIGNAL(clicked(bool)), this, SLOT(video_transition_click()));

  veHeaderLayout->addWidget(btnAddVideoTransition);

  vcontainerLayout->addWidget(veHeader);

  video_effect_area = new QWidget(vcontainer);
  QVBoxLayout* veAreaLayout = new QVBoxLayout(video_effect_area);
  veAreaLayout->setSpacing(0);
  veAreaLayout->setMargin(0);

  vcontainerLayout->addWidget(video_effect_area);

  effects_area_layout->addWidget(vcontainer);

  acontainer = new QWidget(effects_area);
  QVBoxLayout* acontainerLayout = new QVBoxLayout(acontainer);
  acontainerLayout->setSpacing(0);
  acontainerLayout->setMargin(0);
  QWidget* aeHeader = new QWidget(acontainer);
  aeHeader->setObjectName(QStringLiteral("aeHeader"));
  aeHeader->setStyleSheet(QLatin1String("#aeHeader { background: rgba(0, 0, 0, 0.25); }"));

  QHBoxLayout* aeHeaderLayout = new QHBoxLayout(aeHeader);
  aeHeaderLayout->setSpacing(0);
  aeHeaderLayout->setMargin(0);

  QPushButton* btnAddAudioEffect = new QPushButton(aeHeader);
  btnAddAudioEffect->setIcon(QIcon(":/icons/add-effect.png"));
  btnAddAudioEffect->setToolTip(tr("Add Audio Effect"));
  connect(btnAddAudioEffect, SIGNAL(clicked(bool)), this, SLOT(audio_effect_click()));
  aeHeaderLayout->addWidget(btnAddAudioEffect);

  aeHeaderLayout->addStretch();

  QLabel* lblAudioEffects = new QLabel(aeHeader);
  lblAudioEffects->setFont(font);
  lblAudioEffects->setAlignment(Qt::AlignCenter);
  lblAudioEffects->setText(tr("AUDIO EFFECTS"));
  aeHeaderLayout->addWidget(lblAudioEffects);

  aeHeaderLayout->addStretch();

  QPushButton* btnAddAudioTransition = new QPushButton(aeHeader);
  btnAddAudioTransition->setIcon(QIcon(ADD_TRANSITION_ICON));
  btnAddAudioTransition->setToolTip(tr("Add Audio Transition"));
  connect(btnAddAudioTransition, SIGNAL(clicked(bool)), this, SLOT(audio_transition_click()));
  aeHeaderLayout->addWidget(btnAddAudioTransition);

  acontainerLayout->addWidget(aeHeader);

  audio_effect_area = new QWidget(acontainer);
  QVBoxLayout* aeAreaLayout = new QVBoxLayout(audio_effect_area);
  aeAreaLayout->setSpacing(0);
  aeAreaLayout->setMargin(0);

  acontainerLayout->addWidget(audio_effect_area);

  effects_area_layout->addWidget(acontainer);

  lblMultipleClipsSelected = new QLabel(effects_area);
  lblMultipleClipsSelected->setAlignment(Qt::AlignCenter);
  lblMultipleClipsSelected->setText(tr("(Multiple clips selected)"));
  effects_area_layout->addWidget(lblMultipleClipsSelected);

  effects_area_layout->addStretch();

  scrollAreaLayout->addWidget(effects_area);

  scrollArea->setWidget(scrollAreaWidgetContents);
  splitter->addWidget(scrollArea);
  QWidget* keyframeArea = new QWidget(splitter);
  QSizePolicy keyframe_sp;
  keyframe_sp.setHorizontalPolicy(QSizePolicy::Minimum);
  keyframe_sp.setVerticalPolicy(QSizePolicy::Preferred);
  keyframe_sp.setHorizontalStretch(1);
  keyframeArea->setSizePolicy(keyframe_sp);
  QVBoxLayout* keyframeAreaLayout = new QVBoxLayout(keyframeArea);
  keyframeAreaLayout->setSpacing(0);
  keyframeAreaLayout->setMargin(0);
  headers = new TimelineHeader(keyframeArea);

  keyframeAreaLayout->addWidget(headers);

  QWidget* keyframeCenterWidget = new QWidget(keyframeArea);

  QHBoxLayout* keyframeCenterLayout = new QHBoxLayout(keyframeCenterWidget);
  keyframeCenterLayout->setSpacing(0);
  keyframeCenterLayout->setMargin(0);

  keyframeView = new KeyframeView(keyframeCenterWidget);

  keyframeCenterLayout->addWidget(keyframeView);

  verticalScrollBar = new QScrollBar(keyframeCenterWidget);
  verticalScrollBar->setOrientation(Qt::Vertical);

  keyframeCenterLayout->addWidget(verticalScrollBar);


  keyframeAreaLayout->addWidget(keyframeCenterWidget);

  horizontalScrollBar = new ResizableScrollBar(keyframeArea);
  horizontalScrollBar->setOrientation(Qt::Horizontal);

  keyframeAreaLayout->addWidget(horizontalScrollBar);

  splitter->addWidget(keyframeArea);

  hlayout->addWidget(splitter);

  setWidget(contents);
}

void EffectControls::load_effects() {
  lblMultipleClipsSelected->setVisible(multiple);

  if (!multiple) {
    // load in new clips
    for (auto clip_id : selected_clips) {
      ClipPtr c = global::sequence->clip(clip_id);
      if (c == nullptr) {
        qWarning() << "Null Clip instance";
        continue;
      }
      QVBoxLayout* layout;
      if (c->timeline_info.isVideo()) {
        vcontainer->setVisible(true);
        layout = static_cast<QVBoxLayout*>(video_effect_area->layout());
      } else {
        acontainer->setVisible(true);
        layout = static_cast<QVBoxLayout*>(audio_effect_area->layout());
      }
      if (mode == TA_NO_TRANSITION) {
        for (int j=0;j<c->effects.size();j++) {
          open_effect(layout, c->effects.at(j));
        }
      } else if (mode == TA_OPENING_TRANSITION && c->openingTransition() != nullptr) {
        open_effect(layout, c->openingTransition());
      } else if (mode == TA_CLOSING_TRANSITION && c->closingTransition() != nullptr) {
        open_effect(layout, c->closingTransition());
      }
    }//for

    if (!selected_clips.empty()) {
      if (auto sel_clip = global::sequence->clip(selected_clips.front())) {
        setWindowTitle(panel_name + sel_clip->timeline_info.name_);
      } else {
        setWindowTitle(panel_name + "Error");
        qWarning() << "Null Clip instance";
      }
      verticalScrollBar->setMaximum(qMax(0,
                                         effects_area->sizeHint().height()
                                         - headers->height()
                                         + scrollArea->horizontalScrollBar()->height()
                                         /* - keyframeView->height() - headers->height()*/));
      keyframeView->setEnabled(true);
      headers->setVisible(true);
      keyframeView->update();
    }
  }
}

void EffectControls::delete_effects()
{
  // load in new clips
  if (mode == TA_NO_TRANSITION) {
    auto command = new EffectDeleteCommand();
    for (auto clip_id : selected_clips) {
      ClipPtr c = global::sequence->clip(clip_id);
      for (int j=0; j<c->effects.size(); ++j) {
        EffectPtr effect = c->effects.at(j);
        if (effect->container->selected) {
          command->clips.append(c);
          command->fx.append(j);
        }
      }
    }

    if (!command->clips.empty()) {
      e_undo_stack.push(command);
      PanelManager::sequenceViewer().viewer_widget->update();
    } else {
      delete command;
    }
  }
}

void EffectControls::reload_clips() {
  clear_effects(false);
  load_effects();
}

void EffectControls::set_clips(QVector<int>& clips, int m) {
  clear_effects(true);

  // replace clip vector
  selected_clips = clips;
  mode = m;

  load_effects();
}

void EffectControls::video_effect_click() {
  show_effect_menu(EFFECT_TYPE_EFFECT, EFFECT_TYPE_VIDEO);
}

void EffectControls::audio_effect_click() {
  show_effect_menu(EFFECT_TYPE_EFFECT, EFFECT_TYPE_AUDIO);
}

void EffectControls::video_transition_click() {
  show_effect_menu(EFFECT_TYPE_TRANSITION, EFFECT_TYPE_VIDEO);
}

void EffectControls::audio_transition_click() {
  show_effect_menu(EFFECT_TYPE_TRANSITION, EFFECT_TYPE_AUDIO);
}

void EffectControls::resizeEvent(QResizeEvent*) {
  verticalScrollBar->setMaximum(qMax(0, effects_area->height() - keyframeView->height() - headers->height()));
}

bool EffectControls::is_focused()
{
  if (this->hasFocus()) {
    return true;
  }

  for (auto clip_id : selected_clips) {
    ClipPtr c = global::sequence->clip(clip_id);
    if (c == nullptr) {
      qWarning() << "Tried to check focus of a nullptr clip";
      continue;
    }

    for (const auto& eff : c->effects) {
      if (eff != nullptr && eff->container != nullptr && eff->container->is_focused()) {
        return true;
      }
    }
  }
  return false;
}

EffectsArea::EffectsArea(QWidget* parent) :
  QWidget(parent)
{

}

EffectsArea::~EffectsArea()
{
  parent_widget = nullptr;
  keyframe_area = nullptr;
  header = nullptr;
}

void EffectsArea::resizeEvent(QResizeEvent*)
{
  parent_widget->setMinimumWidth(sizeHint().width());
}
