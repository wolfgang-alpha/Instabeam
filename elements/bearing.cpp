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

#include "bearing.h"

#include "node.h"
#include "widgets/easychange/bearingdialog.h"
#include "utilities.h"
#include "jsonkeys.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainter>
#include <QJsonObject>

Bearing::Bearing(Node *parentNode) :
    TrussElement(0, 0, parentNode), // check if coords are correct or if the child uses the parent's c-system
    bearingType(BearingType::LocatingBearing),
    pen(Pen(Color(Qt::black), 3)),
    bearingWidthTotal(30),
    bearingHeight(50),
    floatingBearingVerticalSpacing(7),
    angle(270),
    fx(0),
    fy(0),
    id(Id<Bearing>()),
    fixedClampingWidth(15),
    fixedClampingHeight(70)
{
    setFlag(QGraphicsItem::ItemStacksBehindParent); // otherwise the child item is drawn above the parent-item, completely ignoring the z-value
    setZValue(ElementZValue::Bearing_);
}

Bearing::~Bearing()
{
}

void Bearing::setAngle(double newAngle)
{
    prepareGeometryChange();
    angle = newAngle;
}

void Bearing::setBearingType(BearingType newBearingType)
{
    if (bearingType != newBearingType) {
        prepareGeometryChange();
        bearingType = newBearingType;
        if (bearingType == BearingType::FixedClamping) {
            static_cast<Node *>(parentItem())->setNodeType(NodeType::Weld);
        }
    }
}

QList<double> Bearing::getReactionForces() const
{
    if (bearingType == BearingType::LocatingBearing) {
        return {getReactionForceY(), getReactionForceX()}; // return weak ptrs in container
    } else {
        return {getReactionForceY()}; // return weak ptr in container
    }
}

double Bearing::getReactionForceX() const
{
    return static_cast<Node *>(parentItem())->getFx(); // the bearing has to hold the force pushing on a node
}

double Bearing::getReactionForceY() const
{
    return static_cast<Node *>(parentItem())->getFy();
}

double Bearing::getReactionMomentZ() const
{
    return static_cast<Node *>(parentItem())->getMz();
}

EasyChangeDialog *Bearing::createEasyChangeDialog()
{
    return new BearingDialog(scene()->views().first(), this); // gets handled by the TrussElement
}

QString Bearing::getElementSelectionDialogButtonText() const
{
    if (bearingType == BearingType::LocatingBearing) {
        return QString("Locating bearing (ID: ") + getId() + QString(")");
    } else if (bearingType == BearingType::FloatingBearing) {
        return QString("Floating bearing (ID: ") + getId() + QString(")");
    } else {
        return QString("Fixed clamping (ID: ") + getId() + QString(")");
    }
}

QString Bearing::getId() const
{
    return id.toString();
}

QJsonObject Bearing::saveAsJson() const
{
    QJsonObject o(TrussElement::saveAsJson());
    o.insert(JsonKeys::bearingType, static_cast<int>(bearingType));
    o.insert(JsonKeys::angle, angle);
    o.insert(JsonKeys::id, getId());
    o.insert(JsonKeys::elementType, static_cast<int>(ElementType::Bearing));
    return o;
}

void Bearing::loadFromJson(const QJsonObject &object)
{
    TrussElement::loadFromJson(object);
    bearingType = static_cast<BearingType>(object.value(JsonKeys::bearingType).toInt());
    angle = object.value(JsonKeys::angle).toDouble();
    fx = object.value(JsonKeys::reactionForceX).toDouble();
    fy = object.value(JsonKeys::reactionForceY).toDouble();
    id.resetId(object.value(JsonKeys::id).toString());
}

QRectF Bearing::boundingRect() const
{
    QTransform t;
    t.rotate(- angle - 90); // 0° should be on the right side
    if (bearingType == BearingType::FixedClamping) {
        return t.mapRect(QRectF(- fixedClampingWidth - pen.width() / 2, - fixedClampingHeight / 2 - pen.width() / 2, fixedClampingWidth + pen.width(),
                                fixedClampingHeight + pen.width()));
    }
    int height = 0;
    if (bearingType == BearingType::LocatingBearing) { // change height depending on type of bearing
        height = bearingHeight - floatingBearingVerticalSpacing;
    } else {
        height = bearingHeight;
    }
    return t.mapRect(QRectF(- bearingWidthTotal / 2 - pen.width() / 2, - pen.width() / 2, bearingWidthTotal + pen.width(), height + pen.width()));
}

void Bearing::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const int bearingWidthSocketSupernatant = 5;
    const int bearingSocketHeight = 10;
    int height = 0;
    int bearingHeightWithoutSocketAndVerticalSpacing = 0;
    if (bearingType == BearingType::LocatingBearing) { // change height depending on type of bearing
        height = bearingHeight - floatingBearingVerticalSpacing;
        bearingHeightWithoutSocketAndVerticalSpacing = height - bearingSocketHeight;
    } else if (bearingType == BearingType::FloatingBearing) {
        height = bearingHeight;
        bearingHeightWithoutSocketAndVerticalSpacing = height - bearingSocketHeight - floatingBearingVerticalSpacing;
    }

    painter->save();
    painter->setPen(pen);
    if (isUnderHoverAction) {
        painter->setPen(pen.differentColor(hoverPenColor));
    }
    if (isInSelection) {
        painter->setPen(pen.differentColor(selectionPenColor));
    }
    painter->rotate(- angle - 90); // 0° should be on the right side

    if (bearingType == BearingType::FixedClamping) {
        int fcwidth = fixedClampingWidth;
        int fcheight = fixedClampingHeight;
        const QRectF rect(- fcwidth, - fcheight / 2, fcwidth - pen.width() / 2, fcheight); // draw socket
        painter->fillRect(rect, isInSelection || isUnderHoverAction ? painter->pen().color() : Color(Qt::gray));
        painter->save();
        painter->setPen(isInSelection || isUnderHoverAction ? painter->pen() : Pen(Color(Qt::gray), 3, Qt::RoundCap));
        painter->drawRect(rect);
        painter->restore();
        painter->drawLine(0, - fcheight / 2, 0, fcheight / 2); // draw border-line
    } else {
        // draw filled triangle that the dimension-border-line is not visible in the inner space of the triangle
        QPainterPath triangle(QPointF(0, 0));
        triangle.lineTo(- bearingWidthTotal / 2.0 + bearingWidthSocketSupernatant, bearingHeightWithoutSocketAndVerticalSpacing);
        triangle.lineTo(bearingWidthTotal / 2.0 - bearingWidthSocketSupernatant, triangle.currentPosition().y());
        triangle.lineTo(0, 0);
        painter->save();
        painter->fillPath(triangle, QBrush(Qt::white));
        painter->restore();
        // draw filled rectangle that the dimension-border-line is not visible in the space between the triangle and the socket if this is a floating-bearing
        if (bearingType == BearingType::FloatingBearing) {
            QRectF space(- bearingWidthTotal / 2.0, bearingHeightWithoutSocketAndVerticalSpacing + pen.width() / 2, bearingWidthTotal, floatingBearingVerticalSpacing);
            painter->save();
            painter->fillRect(space, QBrush(Qt::white));
            painter->restore();
        }
        // draw triangle
        QPainterPath path(QPointF(0, 0));
        path.lineTo(- bearingWidthTotal / 2.0 + bearingWidthSocketSupernatant, bearingHeightWithoutSocketAndVerticalSpacing);
        path.moveTo(bearingWidthTotal / 2.0 - bearingWidthSocketSupernatant, path.currentPosition().y());
        path.lineTo(0, 0);
        QPen joinPen(painter->pen());
        joinPen.setCapStyle(Qt::FlatCap); // with the default RoundCap there are points that exceed the joint, therefore change it to FlatCap
        painter->save();
        painter->setPen(joinPen);
        painter->drawPath(path);
        painter->restore();
        if (bearingType == BearingType::FloatingBearing) {
            painter->drawLine(- bearingWidthTotal / 2.0, bearingHeightWithoutSocketAndVerticalSpacing + pen.width() / 2, bearingWidthTotal / 2.0,
                              bearingHeightWithoutSocketAndVerticalSpacing + pen.width() / 2);
        }
        // draw socket
        QRectF rect(- bearingWidthTotal / 2.0, height - bearingSocketHeight, bearingWidthTotal, bearingSocketHeight);
        painter->save();
        if (!isInSelection && !isUnderHoverAction) {
            painter->setPen(pen.differentColor(Color(Qt::gray)));
        }
        painter->drawRect(rect);
        painter->fillRect(rect, painter->pen().color());
        painter->restore();
        int lineHeight = bearingHeightWithoutSocketAndVerticalSpacing;
        if (bearingType == BearingType::FloatingBearing) { // change height of socket-line depending on type of bearing
            lineHeight += floatingBearingVerticalSpacing;
        }
        painter->drawLine(- bearingWidthTotal / 2.0, lineHeight, bearingWidthTotal / 2.0, lineHeight);
    }

    painter->restore();
}

QPainterPath Bearing::shape() const
{
    QTransform t;
    t.rotate(- angle - 90); // correction so that 0° is on the right side
    if (bearingType == BearingType::FixedClamping) {
        QPainterPath p(QPointF(0, 0));
        p.addRect(boundingRect());
        return p;
    }
    double height = 0;
    if (bearingType == BearingType::LocatingBearing) { // change height depending on type of bearing
        height = bearingHeight - floatingBearingVerticalSpacing;
    } else if (bearingType == BearingType::FloatingBearing) {
        height = bearingHeight;
    }
    QPainterPath p(QPointF(0, 0));
    p.addPolygon(t.mapToPolygon(QRect(- bearingWidthTotal / 2 - pen.width() / 2, static_cast<Node *>(parentItem())->getRadius(), bearingWidthTotal + pen.width(),
                                      height - static_cast<Node *>(parentItem())->getRadius()))); // the shape is without the circle, because the circle should be the node
    return p;
}
