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

#include "easychangedialog.h"

#include "elements/trusselement.h"
#include "graphicsscene.h"
#include "widgets/mainwindow.h"
#include "factories/nodeadder.h"
#include "factories/labeladder.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QAction>

QPoint EasyChangeDialog::position = QPoint(200, 100);

EasyChangeDialog::EasyChangeDialog(QWidget *parent, TrussElement *easyChangeUnderlyingElement) :
    QDialog(parent, Qt::CustomizeWindowHint), // hide title bar
    isMoving(false),
    offset(QPoint(0, 0)),
    okButton(new QPushButton("Ok", this)), // parent is this
    cancelButton(new QPushButton("Abbrechen", this)), // parent is this
    underlyingElement(easyChangeUnderlyingElement)
{
}

void EasyChangeDialog::setupEasyChangeDialog(QHBoxLayout *hBoxLayout) // reparents hBoxLayout
{
    auto buttons = new QVBoxLayout(); // parent gets set later
    connect(okButton, &QPushButton::clicked, this, &EasyChangeDialog::okPressed);
    buttons->addWidget(okButton);
    connect(cancelButton, &QPushButton::clicked, this, &EasyChangeDialog::cancelPressed);
    buttons->addWidget(cancelButton);
    hBoxLayout->addLayout(buttons);
    setLayout(hBoxLayout); // reparents all above created widgets and layouts to this->centralWidget()

    okButton->setAutoDefault(false); // do not activate the button when the user presses enter or return while some other widget in the dialog has the focus
    cancelButton->setAutoDefault(false);
    setFixedSize(sizeHint()); // make dialog not resizeable
    move(position); // move to the same position as the last dialog
}

void EasyChangeDialog::cancelPressed(bool) // close dialog without saving changes
{
    static_cast<MainWindow *>(static_cast<GraphicsScene *>(underlyingElement->scene())->parent())->quitAddingElements(); // stop adding elements
    underlyingElement->closeEasyChangeDialog(); // do it via this fct because the dialog-variable in the element has to be set to nullptr
}

void EasyChangeDialog::addNodePermanentlyToScene()
{
    if (underlyingElement != nullptr) {
        if (static_cast<GraphicsScene *>(underlyingElement->scene())->nodeAdderActive()) { // check if in node-adding-mode
            static_cast<GraphicsScene *>(underlyingElement->scene())->getNodeAdder()->addNodePermanently();
        }
    }
}

void EasyChangeDialog::addLabelPermanentlyToScene()
{
    if (underlyingElement != nullptr) {
        if (static_cast<GraphicsScene *>(underlyingElement->scene())->labelAdderActive()) {
            static_cast<GraphicsScene *>(underlyingElement->scene())->getLabelAdder()->addLabelPermanently();
        }
    }
}

void EasyChangeDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        isMoving = true; // indicate that the dialog-box should move upon mouse-movement
        offset = QCursor::pos() - pos(); // calculate the offset between the point where the mouse was pressed and the current position of the dialog
        event->accept();
    } else {
        QDialog::mousePressEvent(event);
    }
}

void EasyChangeDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        isMoving = false; // stop moving
        event->accept();
    } else {
        QDialog::mouseReleaseEvent(event);
    }
}

void EasyChangeDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (isMoving) {
        move(QCursor::pos() - offset); // set the new position of the dialog to the current position of the mouse, corrected by the offset-value
        event->accept();
    } else {
        QDialog::mouseMoveEvent(event);
    }
}

void EasyChangeDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) { // handle button-behaviour
        if (okButton->hasFocus()) { // if the focus is on the button, activate it
            okButton->click();
        } else if (cancelButton->hasFocus()) {
            cancelButton->click();
        }
        event->accept();
    } else if (event->key() == Qt::Key_Escape) { // do not close the dialog when Esc is pressed, pass the event instead to the mainwindow
        static_cast<MainWindow *>(static_cast<GraphicsScene *>(underlyingElement->scene())->parent())->forwardKeyPressEvent(event);
        // do not call accept because if the mainwindow handles the event it will make the call itself
    } else {
        QDialog::keyPressEvent(event);
    }
}

void EasyChangeDialog::closeEvent(QCloseEvent *event)
{
    position = pos(); // save position
    if (underlyingElement != nullptr) {
        underlyingElement->setIsInSelection(false); // when the dialog closes, unselect the element
    }
    // do not call okPressed() to save the values in the input-fields
    QDialog::closeEvent(event); // pass event to parent
}
