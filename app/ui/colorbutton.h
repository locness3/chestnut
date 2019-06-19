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
#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>
#include <QColor>
#include <QUndoCommand>

class ColorButton : public QPushButton {
    Q_OBJECT
  public:
    explicit ColorButton(QWidget* parent = nullptr);
    QColor get_color() const;
    void set_color(const QColor& c);
    const QColor& getPreviousValue();
  protected:
    virtual void showEvent(QShowEvent *event) override;
  private:
    QColor color;
    QColor previousColor;
    void set_button_color();
  signals:
    void color_changed();
  private slots:
    void open_dialog();
};

class ColorCommand : public QUndoCommand {
  public:
    ColorCommand(ColorButton* s, const QColor o, const QColor n);

    ColorCommand(const ColorCommand& ) = delete;
    ColorCommand& operator=(const ColorCommand&) = delete;

    void undo() override;
    void redo() override;
  private:
    ColorButton* sender;
    QColor old_color;
    QColor new_color;
};

#endif // COLORBUTTON_H
