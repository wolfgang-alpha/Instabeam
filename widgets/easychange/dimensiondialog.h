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

#ifndef DIMENSIONDIALOG_H
#define DIMENSIONDIALOG_H

#include "easychangedialog.h"

class Dimension;
class LineEdit;
class QComboBox;

class DimensionDialog : public EasyChangeDialog
{
    Q_OBJECT

public:
    DimensionDialog(QWidget *parent, Dimension *underlyingElement);
    // see base class
    ~DimensionDialog() = default;
    DimensionDialog(const DimensionDialog &) = delete;
    DimensionDialog(DimensionDialog &&) = delete;
    DimensionDialog &operator =(const DimensionDialog &) = delete;
    DimensionDialog &operator =(DimensionDialog &&) = delete;


    void setBaselinePos();
    void setDimensionOrientation();

private:
    Dimension *dimension;
    LineEdit *baselinePosInput;
    QComboBox *orientationInput;

    // EasyChangeDialog interface
public:
    void okPressed(bool) override;
    void updateDialog() override;
};

#endif // DIMENSIONDIALOG_H
