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
#ifndef COLLAPSIBLEWIDGET_H
#define COLLAPSIBLEWIDGET_H

#include <QWidget>
class QLabel;
class QCheckBox;
class QHBoxLayout;
class QVBoxLayout;
class QPushButton;
class QFrame;
class CheckboxEx;

class CollapsibleWidgetHeader : public QWidget {
    Q_OBJECT
  public:
    explicit CollapsibleWidgetHeader(QWidget* parent = nullptr);
    bool selected;
  protected:
    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);
  signals:
    void select(bool, bool);
};

class CollapsibleWidget : public QWidget
{
    Q_OBJECT
  public:
    CheckboxEx* enabled_check {nullptr};
    QPushButton* preset_button_ {nullptr};
    QPushButton* reset_button_ {nullptr};
    bool selected;
    QWidget* contents {nullptr};
    CollapsibleWidgetHeader* title_bar {nullptr};

    explicit CollapsibleWidget(QWidget* parent = nullptr);
    virtual ~CollapsibleWidget() override;
    CollapsibleWidget(const CollapsibleWidget& ) = delete;
    CollapsibleWidget(const CollapsibleWidget&& ) = delete;
    CollapsibleWidget& operator=(const CollapsibleWidget&) = delete;
    CollapsibleWidget& operator=(const CollapsibleWidget&&) = delete;

    void setContents(QWidget* c);
    void setText(const QString &);
    bool is_focused();
    bool is_expanded();

  private:
    QLabel* header {nullptr};
    QVBoxLayout* layout {nullptr};
    QPushButton* collapse_button {nullptr};
    QFrame* line {nullptr};
    QHBoxLayout* title_bar_layout {nullptr};
    void set_button_icon(bool open);

  signals:
    void deselect_others(QWidget*);
    void visibleChanged();

  private slots:
    void on_enabled_change(bool b);
    void on_visible_change();

  public slots:
    void header_click(bool s, bool deselect);
};

#endif // COLLAPSIBLEWIDGET_H
