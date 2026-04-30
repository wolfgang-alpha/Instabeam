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

#ifndef CURVEDBEAMDIALOG_H
#define CURVEDBEAMDIALOG_H

#include "easychangedialog.h"

#include <QLabel>

class CurvedBeam;

class CurvedBeamDialog : public EasyChangeDialog
{
    Q_OBJECT

public:
    CurvedBeamDialog(QWidget *parent, CurvedBeam *elementCurvedBeam);
    ~CurvedBeamDialog() = default;
    CurvedBeamDialog(const CurvedBeamDialog &) = delete;
    CurvedBeamDialog(CurvedBeamDialog &&) = delete;
    CurvedBeamDialog &operator =(const CurvedBeamDialog &) = delete;
    CurvedBeamDialog &operator =(CurvedBeamDialog &&) = delete;

    void setE();
    void setA();
    void setI();
    void setR();

private:
    CurvedBeam *curvedBeam; // weak ptr
    QLabel *rodForceLabel;
    QLabel *arcLengthLabel;
    QLabel *openingAngleLabel;
    LineEdit *EInput;
    LineEdit *AInput;
    LineEdit *IInput;
    LineEdit *RInput;

    // EasyChangeDialog interface
public:
    void okPressed(bool) override;
    void updateDialog() override;
};

#endif // CURVEDBEAMDIALOG_H
