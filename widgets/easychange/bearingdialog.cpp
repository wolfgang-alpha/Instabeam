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

#include "bearingdialog.h"

#include "widgets/lineedit.h"
#include "elements/bearing.h"

#include <QFormLayout>

BearingDialog::BearingDialog(QWidget *parent, Bearing *elementBearing) :
    EasyChangeDialog(parent, elementBearing),
    bearing(elementBearing),
    angleInputLineEdit(new LineEdit(this)),
    typeInput(new QComboBox(this)),
    xForceLabel(new QLabel(QString::number(elementBearing->getReactionForceX()), this)),
    yForceLabel(new QLabel(QString::number(elementBearing->getReactionForceY()), this)),
    zMomentLabel(new QLabel(QString::number(elementBearing->getReactionMomentZ()), this))
{
    auto formLayout = new QFormLayout(); // gets reparented later
    connectLineEdit(angleInputLineEdit, this, &BearingDialog::setAngle);
    angleInputLineEdit->setText(QString::number(bearing->getAngle()));
    if (bearing->getBearingType() != BearingType::FloatingBearing) {
        formLayout->addRow("Angle [°]:", angleInputLineEdit);
    } else {
        angleInputLineEdit->hide();
    }
    typeInput->insertItem(0, "Locating bearing"); // the indexes of the combobox-items have to be the same as in the BearingType-enum
    typeInput->insertItem(1, "Floating bearing");
    typeInput->insertItem(2, "Fixed clamping");
    typeInput->setCurrentIndex(static_cast<int>(bearing->getBearingType()));
    connect(typeInput, qOverload<int>(&QComboBox::currentIndexChanged), this, &BearingDialog::setType);
    formLayout->addRow("Bearing type:", typeInput);
    formLayout->addRow("Reaction force x [N]:", xForceLabel);
    formLayout->addRow("Reaction force y [N]:", yForceLabel);
    formLayout->addRow("Reaction moment z [Nm]:", zMomentLabel);

    auto hBoxLayout = new QHBoxLayout(); // gets reparented later
    hBoxLayout->addLayout(formLayout);
    setupEasyChangeDialog(hBoxLayout); // reparents layouts
}

void BearingDialog::setAngle()
{
    bearing->setAngle(angleInputLineEdit->text().toDouble());
}

void BearingDialog::setType(int index)
{
    bearing->setBearingType(static_cast<BearingType>(index));
}

void BearingDialog::okPressed(bool)
{
    setAngle();
    setType(typeInput->currentIndex());
    bearing->closeEasyChangeDialog();
}

void BearingDialog::updateDialog()
{
    angleInputLineEdit->setText(QString::number(bearing->getAngle()));
    typeInput->setCurrentIndex(static_cast<int>(bearing->getBearingType()));
    xForceLabel->setText(QString::number(bearing->getReactionForceX()));
    yForceLabel->setText(QString::number(bearing->getReactionForceY()));
    zMomentLabel->setText(QString::number(bearing->getReactionMomentZ()));
}
