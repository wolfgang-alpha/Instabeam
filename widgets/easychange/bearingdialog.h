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

#ifndef LOCATINGBEARINGDIALOG_H
#define LOCATINGBEARINGDIALOG_H

#include "easychangedialog.h"

#include <QComboBox>
#include <QLabel>

class LineEdit;
class Bearing;

class BearingDialog : public EasyChangeDialog
{
    Q_OBJECT

public:
    BearingDialog(QWidget *parent, Bearing *elementBearing);
    // see base class
    ~BearingDialog() = default;
    BearingDialog(const BearingDialog &) = delete;
    BearingDialog(BearingDialog &&) = delete;
    BearingDialog &operator =(const BearingDialog &) = delete;
    BearingDialog &operator =(BearingDialog &&) = delete;

    void setAngle();
    void setType(int index);

private:
    Bearing *bearing; // weak ptr
    LineEdit *angleInputLineEdit; // parent is this
    QComboBox *typeInput; // parent is this
    QLabel *xForceLabel; // parent is this
    QLabel *yForceLabel; // parent is this
    QLabel *zMomentLabel; // parent is this

    // EasyChangeDialog interface
public:
    void okPressed(bool) override;
    void updateDialog() override;
};

#endif // LOCATINGBEARINGDIALOG_H
