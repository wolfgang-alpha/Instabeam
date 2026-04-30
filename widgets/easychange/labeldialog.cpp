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

#include "labeldialog.h"

#include "widgets/lineedit.h"
#include "elements/dimension.h"

#include <QFormLayout>

LabelDialog::LabelDialog(QWidget *parent, Label *elementLabel) :
    EasyChangeDialog(parent, elementLabel),
    label(elementLabel),
    textInput(new LineEdit(elementLabel->getText(), this)), // parent is this
    xPosInput(new LineEdit(QString::number(elementLabel->x()), this)), // parent is this
    yPosInput(new LineEdit(QString::number(- elementLabel->y()), this)) // parent is this, flip y-axis
{
    auto formLayout = new QFormLayout(); // parent gets set later
    if (label->parentItem() == nullptr || dynamic_cast<Dimension *>(label->parentItem()) != nullptr) { // if there is no parent or if it is a dimension, make the text editable
        connectLineEdit(textInput, this, &LabelDialog::setText);
        formLayout->addRow("Text:", textInput);
    } else {
        textInput->hide();
    }
    connectLineEdit(xPosInput, this, &LabelDialog::setXPos);
    formLayout->addRow("x-Position:", xPosInput);
    connectLineEdit(yPosInput, this, &LabelDialog::setYPos);
    formLayout->addRow("y-Position:", yPosInput);

    auto layout = new QHBoxLayout(); // parent gets set later
    layout->addLayout(formLayout);
    setupEasyChangeDialog(layout); // reparents the layouts
}

void LabelDialog::setText()
{
    auto dim = label->parentItem() != nullptr ? dynamic_cast<Dimension *>(label->parentItem()) : nullptr;
    label->setText(textInput->text(), dim != nullptr ? dim->getOrientation() == Orientation::Horizontal : false); // center the text if the parent is a horizontal dimension
}

void LabelDialog::setXPos()
{
    label->setXPos(xPosInput->text().toDouble());
}

void LabelDialog::setYPos()
{
    label->setYPos(- yPosInput->text().toDouble()); // flip y-axis
}

void LabelDialog::okPressed(bool)
{
    setXPos();
    setYPos();
    setText(); // this fct updates the values in the x- and y-input-lineEdits, therefore save those values (via calling set_Pos) before saving the text (= calling this fct)
    addLabelPermanentlyToScene();
    label->closeEasyChangeDialog();
}

void LabelDialog::updateDialog()
{
    textInput->setText(label->getText());
    xPosInput->setText(QString::number(label->x()));
    yPosInput->setText(QString::number(- label->y())); // flip y-axis
}
