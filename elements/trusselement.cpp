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

#include "trusselement.h"

#include "graphicsscene.h"
#include "widgets/easychange/easychangedialog.h"
#include "jsonkeys.h"

#include <QJsonObject>

EasyChangeDialog *TrussElement::easyChangeDialog = nullptr;

TrussElement::TrussElement(double xPosition, double yPosition, TrussElement *parentTrussElement) :
    QGraphicsObject(parentTrussElement),
    hoverPenColor(Color(Qt::blue)),
    selectionPenColor(Color(Qt::darkBlue)),
    isMoving(false),
    isInSelection(false),
    isUnderHoverAction(false)
{
    setAcceptHoverEvents(true);
    setX(xPosition);
    setY(yPosition);
}

TrussElement::~TrussElement()
{
    closeEasyChangeDialog(); // within this fct it is examined if the dialog belongs to this element, if not the dialog is not closed
    // the children get removed and deleted in ~QGraphicsItem()
}

void TrussElement::setXPos(double newXPos)
{
    prepareGeometryChange();
    setX(newXPos);
}

void TrussElement::setYPos(double newYPos)
{
    prepareGeometryChange();
    setY(newYPos);
}

void TrussElement::setPosition(QPointF newPosition)
{
    setXPos(newPosition.x());
    setYPos(newPosition.y());
}

void TrussElement::setPositionAndUpdateDialog(QPointF newPosition)
{
    setPosition(newPosition);
    updateEasyChangeDialog();
}

void TrussElement::setIsMoving(bool on)
{
    isMoving = on;
}

void TrussElement::setIsInSelection(bool on)
{
    if (isInSelection != on) {
        prepareGeometryChange(); // do not use update(boundingRect()) because this crashes the app when it is called as a result of the user pressing DEL (the br-fct behaves strangely)
        isInSelection = on; // prepareGeometryChange() will also shedule a redraw of this item even when the br got not changed but only the color
        if (isInSelection) {
            setZValue(zValue() + ElementZValue::Selected); // draw on top of everything
        } else {
            setZValue(zValue() - ElementZValue::Selected); // draw on normal level again
        }
    }
}

void TrussElement::setIsUnderHoverAction(bool on)
{
    if (isUnderHoverAction != on) {
        prepareGeometryChange();
        isUnderHoverAction = on;
        if (isUnderHoverAction) {
            setZValue(zValue() + ElementZValue::Hovered);
        } else {
            setZValue(zValue() - ElementZValue::Hovered);
        }
    }
}

void TrussElement::showEasyChangeDialog()
{
    if (!easyChangeDialogIsShown()) {
        setIsInSelection(true); // this gets set to false in the close-event-fct of the dialog
        if (easyChangeDialog != nullptr) { // close displayed dialog if present
            easyChangeDialog->close(); // close dialog no matter what element it belongs
            delete easyChangeDialog; // delete old dialog created in createEasyChangeDialog()
        }
        easyChangeDialog = createEasyChangeDialog(); // make new dialog
        easyChangeDialog->show();
    }
}

void TrussElement::closeEasyChangeDialog()
{
    if (easyChangeDialogIsShown()) { // close dialog if it belongs to this element
        easyChangeDialog->close(); // this fct makes the underlying element not selected
        delete easyChangeDialog; // delete ptr created in createEasyChangeDialog()
        easyChangeDialog = nullptr; // set to nullptr for flag-checks
    }
}

void TrussElement::updateEasyChangeDialog()
{
    if (easyChangeDialogIsShown()) {
        easyChangeDialog->updateDialog();
    }
}

bool TrussElement::easyChangeDialogIsShown() const // returns true if an easychangedialog is shown and belongs to this element
{
    if (easyChangeDialog == nullptr) {
        return false;
    } else if (easyChangeDialog->getUnderlyingElement() == this) {
        return true;
    } else {
        return false;
    }
}

QJsonObject TrussElement::saveAsJson() const
{
    return {QPair<QString, QJsonValue>(JsonKeys::x, x()),
            QPair<QString, QJsonValue>(JsonKeys::y, y()),
            QPair<QString, QJsonValue>(JsonKeys::zValue, zValue()),
            QPair<QString, QJsonValue>(JsonKeys::parentItem, Utilities::convertAddressToString(static_cast<TrussElement *>(parentItem()))),
            QPair<QString, QJsonValue>(JsonKeys::thisItem, Utilities::convertAddressToString(this))};
}

void TrussElement::loadFromJson(const QJsonObject &object)
{
    setXPos(object.value(JsonKeys::x).toDouble());
    setYPos(object.value(JsonKeys::y).toDouble());
    setZValue(object.value(JsonKeys::zValue).toDouble());
}

void TrussElement::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        isMoving = true;
        if (!easyChangeDialogIsShown()) {
            showEasyChangeDialog();
        }
        event->accept();
    } else {
        QGraphicsObject::mousePressEvent(event);
    }
}

void TrussElement::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isMoving) {
        setPositionAndUpdateDialog(mapToScene(event->pos()));
        event->accept();
    } else {
        QGraphicsObject::mouseMoveEvent(event);
    }
}

void TrussElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        isMoving = false;
        event->accept();
    } else {
        QGraphicsObject::mouseReleaseEvent(event);
    }
}

void TrussElement::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (!static_cast<GraphicsScene *>(scene())->rodAdderActive()) { // if the rod-adder is active, only nodes should be hoverable, therefore this fct is reimplemented in Node
        isUnderHoverAction = true;
        update(boundingRect()); // repaint the element because the color changes
        event->accept();
    }
}

void TrussElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isUnderHoverAction = false;
    update(boundingRect());
    event->accept();
}
