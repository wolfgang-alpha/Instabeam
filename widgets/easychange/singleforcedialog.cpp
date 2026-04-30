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

#include "singleforcedialog.h"

#include "elements/singleforce.h"
#include "widgets/lineedit.h"

#include <QFormLayout>

SingleForceDialog::SingleForceDialog(QWidget *parent, SingleForce *singleForce) :
    EasyChangeDialog(parent, singleForce),
    force(singleForce),
    valueInput(new LineEdit(this)),
    angleInput(new LineEdit(this))
{
    auto formLayout = new QFormLayout(); // parent gets set later
    valueInput->setText(QString::number(force->getValue()));
    connectLineEdit(valueInput, this, &SingleForceDialog::setValue);
    formLayout->addRow("Wert [N]:", valueInput);
    angleInput->setText(QString::number(force->getAngle() * 180 / M_PI - 180));
    connectLineEdit(angleInput, this, &SingleForceDialog::setAngle);
    formLayout->addRow("Winkel [°]:", angleInput);

    auto hBoxLayout = new QHBoxLayout(); // gets reparented later
    hBoxLayout->addLayout(formLayout);
    setupEasyChangeDialog(hBoxLayout); // reparents the above created layouts
}

void SingleForceDialog::setValue()
{
    force->setValue(valueInput->text().toDouble());
}

void SingleForceDialog::setAngle()
{
    force->setAngle((angleInput->text().toDouble() + 180) * M_PI / 180);
}

void SingleForceDialog::okPressed(bool)
{
    setValue();
    setAngle();
    force->closeEasyChangeDialog();
}

void SingleForceDialog::updateDialog()
{
    valueInput->setText(QString::number(force->getValue()));
    angleInput->setText(QString::number(force->getAngle() * 180 / M_PI));
}
