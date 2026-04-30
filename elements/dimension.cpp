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

#include "dimension.h"

#include "elements/node.h"
#include "elements/label.h"
#include "widgets/easychange/dimensiondialog.h"
#include "jsonkeys.h"
#include "graphicsscene.h"

#include <QGraphicsView>
#include <QPainter>
#include <QJsonObject>

Dimension::Dimension(Node *elementNode1, Node *elementNode2) :
    TrussElement(0, 0, nullptr),
    id(Id<Dimension>()),
    node1(elementNode1),
    node2(elementNode2),
    arrowWidth(7),
    arrowHeight(20),
    minimumArrowToArrowDistance(10),
    borderLineSupernatant(10),
    baseLineSupernatant(15),
    orientation(Orientation::Horizontal), // default is horizontal
    pen(Pen(Color(255, 215, 0), 2)),
    label(nullptr),
    initialDimensionLabelDistance(10)
{
    setZValue(ElementZValue::Dimension_);
    if (node1 != nullptr) {
        node1->addDimension(this);
    }
    if (node2 != nullptr) {
        node2->addDimension(this);
    }
    if (node1 != nullptr && node2 != nullptr) {
        baselinePos = (node1->y() + node2->y()) / 2 - 50; // draw the dimension in the middle of node1 and node2 with a given offset
        label = new Label(QString("a") + id.toString(), (node1->x() + node2->x()) / 2, baselinePos - initialDimensionLabelDistance, this); // create label, this is parent
        label->setXPos(label->x() - label->getTextWidth() / 2); // center label around its x-pos
        label->setColor(pen.getColor()); // color label
    }
}

Dimension::~Dimension()
{
    // remove this from the dimension-lists of the nodes
    if (node1 != nullptr) { // if a node got deleted previously, do not try to access a member fct of it
        node1->removeDimension(this); // remove this from the node's dimension-list
    }
    if (node2 != nullptr) {
        node2->removeDimension(this);
    }
}

bool Dimension::isForLongMeasurement() const
{
    int minDist = 2 * arrowHeight + minimumArrowToArrowDistance; // minimal required distance between the border-lines
    if (node1 == nullptr || node2 == nullptr) {
        return false;
    }
    double dist = orientation == Orientation::Horizontal ? fabs(node2->x() - node1->x()) : fabs(node2->y() - node1->y()); // actual (and absolute) distance between the border-lines
    return std::isgreaterequal(dist, minDist); // returns true if the actual distance is greater than the minimum distance
}

void Dimension::setBaselinePos(double newBaselinePos)
{
    if (orientation == Orientation::Horizontal) {
        label->setYPos(label->y() + (newBaselinePos - baselinePos));
    } else {
        label->setXPos(label->x() + (newBaselinePos - baselinePos));
    }
    prepareGeometryChange();
    baselinePos = newBaselinePos;
    updateEasyChangeDialog();
}

void Dimension::setOrientation(Orientation newOrientation)
{
    if (orientation != newOrientation) {
        // reposition label
        if (orientation == Orientation::Horizontal) { // orientation change from horizontal to vertical
            label->setXPos(baselinePos - initialDimensionLabelDistance - label->getTextWidth());
            label->setYPos((node1->y() + node2->y()) / 2 + label->getTextHeight() / 2);
        } else { // orientation changed from vertical to horizontal
            label->setXPos((node1->x() + node2->x()) / 2 - label->getTextWidth() / 2);
            label->setYPos(baselinePos - initialDimensionLabelDistance);
        }
        prepareGeometryChange();
        orientation = newOrientation;
    }
}

Node *Dimension::takeNode1()
{
    node1->removeDimension(this);
    Node *tmp = node1;
    node1 = nullptr;
    return tmp;
}

Node *Dimension::takeNode2()
{
    node2->removeDimension(this);
    Node *tmp = node2;
    node2 = nullptr;
    return tmp;
}

void Dimension::setNode2(Node *newNode2)
{
    if (node2 != nullptr) { // if node2 is not present don't remove this from it's dimension-list
        node2->removeDimension(this); // remove this from the dimension-list of the old node2
    }
    node2 = newNode2; // since node2 is just a weak ptr this line is allowed
    node2->addDimension(this);
}

void Dimension::linkNode1(Node *newNode1)
{
    if (node1 != nullptr) {
        qFatal("error 1 in Dimension::linkNode1(): node1 != nullptr");
    }
    node1 = newNode1;
}

void Dimension::linkNode2(Node *newNode2)
{
    if (node2 != nullptr) {
        qFatal("error 1 in Dimension::linkNode2(): node2 != nullptr");
    }
    node2 = newNode2;
}

void Dimension::forwardPrepareGeometryChange()
{
    prepareGeometryChange();
}

void Dimension::moveLabelX(double distance)
{
    label->setXPos(label->x() + distance);
}

void Dimension::moveLabelY(double distance)
{
    label->setYPos(label->y() + distance);
}

void Dimension::setLabel(Label *newLabel)
{
    if (label != nullptr) {
        delete label;
    }
    label = newLabel;
    label->setColor(pen.getColor()); // change color of label to the proper color
}

EasyChangeDialog *Dimension::createEasyChangeDialog()
{
    return new DimensionDialog(scene()->views().first(), this); // the TrussElement has the ownership of the pointer
}

QString Dimension::getElementSelectionDialogButtonText() const
{
    return QString("Bemaßung (ID: ") + getId() + QString(")");
}

QString Dimension::getId() const
{
    return id.toString();
}

QJsonObject Dimension::saveAsJson() const
{
    QJsonObject o = TrussElement::saveAsJson();
    o.insert(JsonKeys::node1, Utilities::convertAddressToString(node1));
    o.insert(JsonKeys::node2, Utilities::convertAddressToString(node2));
    o.insert(JsonKeys::orientation, static_cast<int>(orientation));
    o.insert(JsonKeys::baselinePos, baselinePos);
    o.insert(JsonKeys::id, id.toString());
    o.insert(JsonKeys::elementType, static_cast<int>(ElementType::Dimension));
    o.insert(JsonKeys::label, Utilities::convertAddressToString(label));
    return o;
}

void Dimension::loadFromJson(const QJsonObject &object)
{
    TrussElement::loadFromJson(object);
    orientation = static_cast<Orientation>(object.value(JsonKeys::orientation).toInt());
    baselinePos = object.value(JsonKeys::baselinePos).toDouble();
    id.resetId(object.value(JsonKeys::id).toString());
}

QRectF Dimension::boundingRect() const
{
    if (node1 == nullptr || node2 == nullptr) {
        return {0, 0, 0, 0};
    }
    if (isForLongMeasurement()) {
        if (orientation == Orientation::Horizontal) {
            double left = fmin(node1->x(), node2->x());
            double right = fmax(node1->x(), node2->x());
            double top = fmin(fmin(node1->y(), node2->y()), baselinePos);
            double bottom = fmax(fmax(node1->y(), node2->y()), baselinePos);
            return {left - pen.width() / 2, top - borderLineSupernatant - pen.width() / 2, right - left + pen.width(), bottom - top + 2 * borderLineSupernatant + pen.width()};
        } else {
            double left = fmin(fmin(node1->x(), node2->x()), baselinePos);
            double right = fmax(fmax(node1->x(), node2->x()), baselinePos);
            double top = fmin(node1->y(), node2->y());
            double bottom = fmax(node1->y(), node2->y());
            return {left - borderLineSupernatant - pen.width() / 2, top - pen.width() / 2, right - left + 2 * borderLineSupernatant + pen.width(), bottom - top + pen.width()};
        }
    } else {
        if (orientation == Orientation::Horizontal) {
            double left = fmin(node1->x(), node2->x());
            double right = fmax(node1->x(), node2->x());
            double top = fmin(fmin(node1->y(), node2->y()), baselinePos);
            double bottom = fmax(fmax(node1->y(), node2->y()), baselinePos);
            return {left - pen.width() / 2 - arrowHeight - baseLineSupernatant, top - borderLineSupernatant - pen.width() / 2, right - left + 2 * baseLineSupernatant +
                    2 * arrowHeight + pen.width(), bottom - top + 2 * borderLineSupernatant + pen.width()};
        } else {
            double left = fmin(fmin(node1->x(), node2->x()), baselinePos);
            double right = fmax(fmax(node1->x(), node2->x()), baselinePos);
            double top = fmin(node1->y(), node2->y());
            double bottom = fmax(node1->y(), node2->y());
            return {left - borderLineSupernatant - pen.width() / 2, top - arrowHeight - baseLineSupernatant - pen.width() / 2, right - left + 2 * borderLineSupernatant + pen.width(),
                    bottom - top + 2 * arrowHeight + 2 * baseLineSupernatant + pen.width()};
        }
    }
}

QPainterPath Dimension::shape() const
{
    QPainterPath p(QPointF(0, 0));
    QRectF rect;
    if (isForLongMeasurement()) {
        if (orientation == Orientation::Horizontal) {
            double left = fmin(node1->x(), node2->x());
            double right = fmax(node1->x(), node2->x());
            rect.setRect(left, baselinePos - arrowWidth / 2, right - left, arrowWidth);
        } else {
            double top = fmin(node1->y(), node2->y());
            double bottom = fmax(node1->y(), node2->y());
            rect.setRect(baselinePos - arrowWidth / 2, top, arrowWidth, bottom - top);
        }
    } else {
        if (orientation == Orientation::Horizontal) {
            double left = fmin(node1->x(), node2->x());
            double right = fmax(node1->x(), node2->x());
            rect.setRect(left - arrowHeight - baseLineSupernatant, baselinePos - arrowWidth / 2, right - left + 2 * arrowHeight + 2 * baseLineSupernatant, arrowWidth);
        } else {
            double top = fmin(node1->y(), node2->y());
            double bottom = fmax(node1->y(), node2->y());
            rect.setRect(baselinePos - arrowWidth / 2, top - arrowHeight - baseLineSupernatant, arrowWidth, bottom - top + 2 * arrowHeight + 2 * baseLineSupernatant);
        }
    }
    p.addRect(rect);
    return p;
}

void Dimension::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (node1 == nullptr || node2 == nullptr) {
        return;
    }
    painter->save();
    painter->setPen(pen);
    if (isUnderHoverAction) {
        painter->setPen(pen.differentColor(hoverPenColor));
    }
    if (isInSelection) {
        painter->setPen(pen.differentColor(selectionPenColor));
    }
    QPainterPath p(QPointF(0, 0));
    if (isForLongMeasurement()) {
        if (orientation == Orientation::Horizontal) {
            // make n1 the left and n2 the right node
            Node *n1 = node1->x() < node2->x() ? node1 : node2; // weak ptrs
            Node *n2 = std::isgreaterequal(node1->x(), node2->x()) ? node1 : node2; // weak ptrs
            p.moveTo(n1->x(), n1->y());
            double delta = baselinePos < n1->y() ? - borderLineSupernatant : borderLineSupernatant;
            p.lineTo(p.currentPosition().x(), baselinePos + delta);
            p.moveTo(p.currentPosition().x(), p.currentPosition().y() - delta);
            p.moveTo(p.currentPosition().x() + arrowHeight, p.currentPosition().y());
            p.lineTo(n2->x() - arrowHeight, p.currentPosition().y());
            p.moveTo(n2->x(), n2->y());
            delta = baselinePos < n2->y() ? - borderLineSupernatant : borderLineSupernatant; // recompute delta
            p.lineTo(p.currentPosition().x(), baselinePos + delta);
            // draw arrows
            painter->save();
            painter->translate(n1->x(), baselinePos);
            Utilities::drawArrow(painter, arrowWidth, arrowHeight);
            painter->restore();
            painter->save();
            painter->translate(n2->x(), baselinePos);
            painter->rotate(180);
            Utilities::drawArrow(painter, arrowWidth, arrowHeight);
            painter->restore();
        } else {
            Node *n1 = std::isgreaterequal(node1->y(), node2->y()) ? node1 : node2;
            Node *n2 = node1->y() < node2->y() ? node1 : node2;
            p.moveTo(n1->x(), n1->y());
            double delta = baselinePos < n1->x() ? - borderLineSupernatant : borderLineSupernatant;
            p.lineTo(baselinePos + delta, p.currentPosition().y());
            p.moveTo(p.currentPosition().x() - delta, p.currentPosition().y());
            p.moveTo(p.currentPosition().x(), p.currentPosition().y() - arrowHeight);
            p.lineTo(p.currentPosition().x(), n2->y() + arrowHeight);
            p.moveTo(n2->x(), n2->y());
            delta = baselinePos < n2->x() ? - borderLineSupernatant : borderLineSupernatant;
            p.lineTo(baselinePos + delta, p.currentPosition().y());
            // arrows
            painter->save();
            painter->translate(baselinePos, n1->y());
            painter->rotate(270);
            Utilities::drawArrow(painter, arrowWidth, arrowHeight);
            painter->restore();
            painter->save();
            painter->translate(baselinePos, n2->y());
            painter->rotate(90);
            Utilities::drawArrow(painter, arrowWidth, arrowHeight);
            painter->restore();
        }
    } else {
        if (orientation == Orientation::Horizontal) {
            // make n1 the left and n2 the right node
            Node *n1 = node1->x() < node2->x() ? node1 : node2; // weak ptrs
            Node *n2 = std::isgreaterequal(node1->x(), node2->x()) ? node1 : node2; // weak ptrs
            p.moveTo(n1->x(), n1->y());
            double delta = baselinePos < n1->y() ? - borderLineSupernatant : borderLineSupernatant;
            p.lineTo(p.currentPosition().x(), baselinePos + delta);
            p.moveTo(p.currentPosition().x(), p.currentPosition().y() - delta);
            p.moveTo(p.currentPosition().x() - arrowHeight, p.currentPosition().y());
            p.lineTo(p.currentPosition().x() - baseLineSupernatant, p.currentPosition().y());
            p.moveTo(n1->x(), p.currentPosition().y());
            p.lineTo(n2->x(), p.currentPosition().y());
            p.moveTo(p.currentPosition().x() + arrowHeight, p.currentPosition().y());
            p.lineTo(p.currentPosition().x() + baseLineSupernatant, p.currentPosition().y());
            p.moveTo(n2->x(), n2->y());
            delta = baselinePos < n2->y() ? - borderLineSupernatant : borderLineSupernatant; // recompute delta
            p.lineTo(p.currentPosition().x(), baselinePos + delta);
            // draw arrows
            painter->save();
            painter->translate(n1->x(), baselinePos);
            painter->rotate(180);
            Utilities::drawArrow(painter, arrowWidth, arrowHeight);
            painter->restore();
            painter->save();
            painter->translate(n2->x(), baselinePos);
            Utilities::drawArrow(painter, arrowWidth, arrowHeight);
            painter->restore();
        } else {
            Node *n1 = std::isgreaterequal(node1->y(), node2->y()) ? node1 : node2;
            Node *n2 = node1->y() < node2->y() ? node1 : node2;
            p.moveTo(n1->x(), n1->y());
            double delta = baselinePos < n1->x() ? - borderLineSupernatant : borderLineSupernatant;
            p.lineTo(baselinePos + delta, p.currentPosition().y());
            p.moveTo(p.currentPosition().x() - delta, p.currentPosition().y());
            p.moveTo(p.currentPosition().x(), p.currentPosition().y() + arrowHeight);
            p.lineTo(p.currentPosition().x(), p.currentPosition().y() + baseLineSupernatant);
            p.moveTo(p.currentPosition().x(), n1->y());
            p.lineTo(p.currentPosition().x(), n2->y());
            p.moveTo(p.currentPosition().x(), p.currentPosition().y() - arrowHeight);
            p.lineTo(p.currentPosition().x(), p.currentPosition().y() - baseLineSupernatant);
            p.moveTo(n2->x(), n2->y());
            delta = baselinePos < n2->x() ? - borderLineSupernatant : borderLineSupernatant;
            p.lineTo(baselinePos + delta, p.currentPosition().y());
            // arrows
            painter->save();
            painter->translate(baselinePos, n1->y());
            painter->rotate(90);
            Utilities::drawArrow(painter, arrowWidth, arrowHeight);
            painter->restore();
            painter->save();
            painter->translate(baselinePos, n2->y());
            painter->rotate(270);
            Utilities::drawArrow(painter, arrowWidth, arrowHeight);
            painter->restore();
        }
    }
    painter->drawPath(p);
    painter->restore();
}

void Dimension::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isMoving) { // move the baseline of the dimension
        if (orientation == Orientation::Horizontal) {
            setBaselinePos(event->pos().y());
        } else {
            setBaselinePos(event->pos().x());
        }
    }
    event->accept();
}
