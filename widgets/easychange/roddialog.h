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

#ifndef RODDIALOG_H
#define RODDIALOG_H

#include "easychangedialog.h"

#include <QLabel>
#include <QComboBox>

class Rod;

class RodDialog : public EasyChangeDialog
{
    Q_OBJECT

public:
    RodDialog(QWidget *parent, Rod *elementRod);
    // see base class
    ~RodDialog() = default;
    RodDialog(const RodDialog &) = delete;
    RodDialog(RodDialog &&) = delete;
    RodDialog &operator =(const RodDialog &) = delete;
    RodDialog &operator =(RodDialog &&) = delete;

    void setE();
    void setA();
    void setI();

private:
    Rod *rod; // weak ptr
    QLabel *rodForceLabel; // parent is this
    QLabel *u1xLabel; // parent is this
    QLabel *u1yLabel; // parent is this
    QLabel *u1zLabel; // parent is this
    QLabel *u2xLabel; // parent is this
    QLabel *u2yLabel; // parent is this
    QLabel *u2zLabel; // parent is this
    LineEdit *EInput; // parent is this
    LineEdit *AInput; // parent is this
    LineEdit *IInput; // parent is this

    // EasyChangeDialog interface
public:
    void okPressed(bool) override;
    void updateDialog() override;
};

#endif // RODDIALOG_H
