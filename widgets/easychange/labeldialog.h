/*
 * Instabeam - 2D structural analysis with FEM (curved beams, trusses, CST)
 * Copyright (C) 2018-2019 bernhard1536 (original TrussCalculator code)
 * Copyright (C) 2020-2026 Wolfgang Flachberger (Instabeam additions)
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LABELDIALOG_H
#define LABELDIALOG_H

#include "easychangedialog.h"
#include "elements/label.h"

class LabelDialog : public EasyChangeDialog
{
    Q_OBJECT

public:
    LabelDialog(QWidget *parent, Label *elementLabel);
    // see base class
    ~LabelDialog() = default;
    LabelDialog(const LabelDialog &) = delete;
    LabelDialog(LabelDialog &&) = delete;
    LabelDialog &operator =(const LabelDialog &) = delete;
    LabelDialog &operator =(LabelDialog &&) = delete;

    void setText();
    void setXPos();
    void setYPos();

private:
    Label *label; // weak ptr
    LineEdit *textInput; // parent is this
    LineEdit *xPosInput; // parent is this
    LineEdit *yPosInput; // parent is this

    // EasyChangeDialog interface
public:
    void okPressed(bool) override;
    void updateDialog() override;
};

#endif // LABELDIALOG_H
