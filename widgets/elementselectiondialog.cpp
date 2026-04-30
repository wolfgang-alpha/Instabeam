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

#include "elementselectiondialog.h"

#include "elements/trusselement.h"
#include "widgets/graphicsview.h"

#include <QVBoxLayout>
#include <QPushButton>

ElementSelectionDialog::ElementSelectionDialog(QList<TrussElement *> elementList, GraphicsView *parent) :
    QDialog(parent, Qt::CustomizeWindowHint)
{
    QVBoxLayout *layout = new QVBoxLayout(); // gets reparented later
    for (TrussElement *element : elementList) { // add for each element a button with a descriptive text
        QPushButton *button = new QPushButton(element->getElementSelectionDialogButtonText(), this); // parent is this
        connect(button, &QPushButton::clicked, element, &TrussElement::showEasyChangeDialog); // if the user clicks on a button, select the corresponding element and show its dialog
        connect(button, &QPushButton::clicked, this, &QDialog::close);
        layout->addWidget(button);
    }
    setLayout(layout); // dialog takes ownership of layout
    setFixedSize(sizeHint()); // make dialog not resizeable
}

void ElementSelectionDialog::keyPressEvent(QKeyEvent *event) // forward key-events to graphicsview to close the dialog when the user releases shift
{
    static_cast<GraphicsView *>(parent())->forwardKeyPressEvent(event);
}

void ElementSelectionDialog::keyReleaseEvent(QKeyEvent *event)
{
    static_cast<GraphicsView *>(parent())->forwardKeyReleaseEvent(event);
}
