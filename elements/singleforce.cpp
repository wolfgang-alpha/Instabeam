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

#include "singleforce.h"

#include "node.h"
#include "utilities.h"
#include "widgets/easychange/singleforcedialog.h"
#include "jsonkeys.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainter>
#include <QJsonObject>

SingleForce::SingleForce(Node *parentNode, double elementValue, double elementAngle) :
    TrussElement(0, 0, parentNode),
    pen(Pen(Color(Qt::red), 3)),
    arrowWidth(10),
    arrowHeight(20),
    length(100),
    value(elementValue),
    angle(elementAngle),
    id(Id<SingleForce>())
{
    setFlag(QGraphicsItem::ItemStacksBehindParent);
    setZValue(ElementZValue::SingleForce_);
}

void SingleForce::setValue(double newValue)
{
    if (value != newValue) {
        value = newValue;
    }
}

void SingleForce::setAngle(double newAngle)
{
    if (angle != newAngle) {
        prepareGeometryChange();
        angle = newAngle;
    }
}

EasyChangeDialog *SingleForce::createEasyChangeDialog()
{
    return new SingleForceDialog(scene()->views().first(), this); // the TrussElement is responsible for deletion
}

QString SingleForce::getElementSelectionDialogButtonText() const
{
    return QString("Force (ID: ") + getId() + QString(")");
}

QString SingleForce::getId() const
{
    return id.toString();
}

QJsonObject SingleForce::saveAsJson() const
{
    QJsonObject o(TrussElement::saveAsJson());
    o.insert(JsonKeys::value, value);
    o.insert(JsonKeys::angle, angle);
    o.insert(JsonKeys::id, getId());
    o.insert(JsonKeys::elementType, static_cast<int>(ElementType::SingleForce));
    return o;
}

void SingleForce::loadFromJson(const QJsonObject &object)
{
    TrussElement::loadFromJson(object);
    value = object.value(JsonKeys::value).toDouble();
    angle = object.value(JsonKeys::angle).toDouble();
    id.resetId(object.value(JsonKeys::id).toString());
}

QRectF SingleForce::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath SingleForce::shape() const
{
    QPainterPath p(QPointF(0, 0));
    double translationX = 0;
    if (parentItem() != nullptr) {
        if (static_cast<Node *>(parentItem())->getNodeType() == NodeType::GerberJoint) {
            translationX = static_cast<Node *>(parentItem())->getRadius();
        }
    }
    QRect rect(translationX - pen.width() / 2, - arrowWidth / 2 - pen.width() / 2, length + pen.width(), arrowWidth + pen.width());
    QTransform t;
    t.rotate(- (angle * 180 / M_PI - 180));
    p.addPolygon(t.mapToPolygon(rect));
    p.closeSubpath(); // draw a line back from the last polygon-point to the starting-point
    return p;
}

void SingleForce::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();
    painter->setPen(pen);
    if (isUnderHoverAction) {
        painter->setPen(pen.differentColor(hoverPenColor));
    }
    if (isInSelection) {
        painter->setPen(pen.differentColor(selectionPenColor));
    }
    painter->rotate(- (angle * 180 / M_PI - 180));
    if (parentItem() != nullptr) {
        if (static_cast<Node *>(parentItem())->getNodeType() == NodeType::GerberJoint) {
            painter->translate(static_cast<Node *>(parentItem())->getRadius(), 0); // start drawing outside of the node-circle
        }
    }

    painter->drawLine(arrowHeight, 0, length, 0);
    Utilities::drawArrow(painter, arrowWidth, arrowHeight);

    painter->restore();
}
