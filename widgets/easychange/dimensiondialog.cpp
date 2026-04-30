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

#include "dimensiondialog.h"

#include "elements/dimension.h"

#include <QComboBox>
#include <QFormLayout>

DimensionDialog::DimensionDialog(QWidget *parent, Dimension *underlyingElement) :
    EasyChangeDialog(parent, underlyingElement),
    dimension(underlyingElement),
    baselinePosInput(new LineEdit(QString::number(underlyingElement->getOrientation() == Orientation::Vertical ? underlyingElement->getBaselinePos() : - underlyingElement->getBaselinePos()), this)), // parent is this, flip y-axis if horizontal
    orientationInput(new QComboBox(this)) // parent is this
{
    auto layout = new QFormLayout(); // parent gets set later
    connectLineEdit(baselinePosInput, this, &DimensionDialog::setBaselinePos);
    layout->addRow("Position der Grundlinie [px]:", baselinePosInput);
    orientationInput->insertItem(0, "Horizontal"); // the indexes of the combobox-items have to be the same as in the Orientation-enum
    orientationInput->insertItem(1, "Vertikal");
    orientationInput->setCurrentIndex(static_cast<int>(dimension->getOrientation()));
    connect(orientationInput, qOverload<int>(&QComboBox::currentIndexChanged), this, &DimensionDialog::setDimensionOrientation);
    layout->addRow("Ausrichtung:", orientationInput);

    auto l = new QHBoxLayout(); // parent gets set later
    l->addLayout(layout);
    setupEasyChangeDialog(l); // reparents layout and l to this
}

void DimensionDialog::setBaselinePos()
{

    dimension->setBaselinePos(dimension->getOrientation() == Orientation::Vertical ? baselinePosInput->text().toDouble() : - baselinePosInput->text().toDouble()); // flip y-axis if horizontal
}

void DimensionDialog::setDimensionOrientation()
{
    dimension->setOrientation(static_cast<Orientation>(orientationInput->currentIndex()));
}

void DimensionDialog::okPressed(bool)
{
    setBaselinePos();
    setDimensionOrientation();
    close();
}

void DimensionDialog::updateDialog()
{
    baselinePosInput->setText(QString::number(dimension->getOrientation() == Orientation::Vertical ? dimension->getBaselinePos() : - dimension->getBaselinePos())); // flip y-axis if horizontal
    orientationInput->setCurrentIndex(static_cast<int>(dimension->getOrientation()));
}
