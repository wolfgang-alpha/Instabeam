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

#include "rod.h"

#include "node.h"
#include "widgets/easychange/roddialog.h"
#include "utilities.h"
#include "graphicsscene.h"
#include "widgets/mainwindow.h"
#include "elements/label.h"
#include "jsonkeys.h"

#include <QGraphicsView>
#include <QPainter>
#include <QDebug>
#include <QJsonObject>

Rod::ColorMap Rod::colorMap = Rod::ColorMap();
double Rod::maxDisplacement = 0.0;

Rod::Rod() :
    TrussElement(0, 0, nullptr),
    node1(nullptr),
    node2(nullptr),
    pen(Pen(Color(Qt::black), 5)),
    id(Id<Rod>()),
    label(nullptr), // parent is this, ~QGraphicsItem() deletes it
    calcId(0),
    innerForce(0),
    dof{-1, -1, -1, -1, -1, -1}, // init dofs with -1 because against this value gets checked if a dof has a valid id (-1 is an invalid id)
    u{0, 0, 0, 0, 0, 0},
    T(Eigen::Matrix6d::Identity(6, 6)),
    E(1),
    A(1),
    I(1)
{
    setZValue(ElementZValue::Rod_);
}

Rod::Rod(Node *elementNode1, Node *elementNode2) :
    TrussElement(0, 0, nullptr),
    node1(elementNode1),
    node2(elementNode2),
    pen(Pen(Color(Qt::black), 5)),
    id(Id<Rod>()),
    label(new Label(id.toString(), 0, 0, this)), // parent is this, ~QGraphicsItem() deletes it
    calcId(0),
    innerForce(0),
    dof{-1, -1, -1, -1, -1, -1}, // init dofs with -1 because against this value gets checked if a dof has a valid id (-1 is an invalid id)
    u{0, 0, 0, 0, 0, 0},
    T(Eigen::Matrix6d::Identity(6, 6)),
    E(1),
    A(1),
    I(1)
{
    setZValue(ElementZValue::Rod_);
    if (node1 != nullptr) { // check for null if a nullptr is passed as node1
        node1->addRod(this); // add this to the rod-lists of the nodes
    }
    if (node2 != nullptr) {
        node2->addRod(this);
    }
    label->hide(); // set position of label
    if (node1 != nullptr && node2 != nullptr) {
        double radius = sqrt(pow(node2->x() - node1->x(), 2) + pow(node2->y() - node1->y(), 2)) / 2; // half the distance between the two nodes, node1 is reference point
        double angle = - getAngle(); // the minus is because the y-axis in the angle-calculation is inverted (arctan(-a/b) = -arctan(a/b))
        label->setXPos(radius * cos(angle) + node1->x()); // angle is already in rad
        label->setYPos(radius * sin(angle) + node1->y());
    }
}

Rod::~Rod()
{
    // remove this from the rod-lists of the nodes
    if (node1 != nullptr) { // if a node got deleted previously, do not try to access a member fct of it
        node1->removeRod(this); // remove this from the node's rod-list
        static_cast<GraphicsScene *>(scene())->validateNode(node1); // check if there are still rods connected, otherwise delete the node
    }
    if (node2 != nullptr) {
        node2->removeRod(this);
        static_cast<GraphicsScene *>(scene())->validateNode(node2);
    }
}

Node *Rod::takeNode1() // see takeNode2()
{
    node1->removeRod(this);
    Node *tmp = node1;
    node1 = nullptr;
    return tmp;
}

Node *Rod::takeNode2()
{
    node2->removeRod(this); // remove this from the rod-list of the old node2
    Node *tmp = node2; // copy address of node2 into tmp (this is done to be able to set node2 to a nullptr for further flag-checks and still return the address node2 pointed to)
    node2 = nullptr; // make node2 point to nothing (null)
    return tmp; // return tmp, it points to the old node2
}

void Rod::setNode2(Node *newNode2)
{
    if (node2 != nullptr) { // if node2 is not present don't remove this from it's rod-list
        node2->removeRod(this); // remove this from the rod-list of the old node2
    }
    node2 = newNode2; // since node2 is just a weak ptr this line is allowed
    node2->addRod(this); // add this to the rod-list of the new node2
    // update label-position
    double radius = sqrt(pow(node2->x() - node1->x(), 2) + pow(node2->y() - node1->y(), 2)) / 2; // half the distance between the two nodes
    double angle = - getAngle(); // the radius-vector points away from node1
    label->setXPos(radius * cos(angle) + node1->x()); // angle is already in rad
    label->setYPos(radius * sin(angle) + node1->y());
}

void Rod::forwardPrepareGeometryChange()
{
    prepareGeometryChange();
}

void Rod::setColorMapValues(double minValue, double maxValue)
{
    colorMap.setMinValue(minValue);
    colorMap.setMaxValue(maxValue);
    static_cast<MainWindow *>(scene()->parent())->updateRodColorMinMaxValue(minValue, maxValue); // update the values shown in the status-bar
}

void Rod::setLabel(Label *newLabel)
{
    if (label != nullptr) {
        delete label;
    }
    label = newLabel;
}

void Rod::moveLabelX(double distance)
{
    label->setXPos(label->x() + distance);
}

void Rod::moveLabelY(double distance)
{
    label->setYPos(label->y() + distance);
}

void Rod::linkNode1(Node *newNode1)
{
    if (node1 != nullptr) {
        qFatal("error 1 in Rod::linkNode1(): node1 != nullptr");
    }
    node1 = newNode1;
}

void Rod::linkNode2(Node *newNode2)
{
    if (node2 != nullptr) {
        qFatal("error 1 in Rod::linkNode2(): node2 != nullptr");
    }
    node2 = newNode2;
}

double Rod::getLength() const // returns the length of the rod [m]
{
    if (node1 != nullptr && node2 != nullptr) {
        double dx = (node2->x() - node1->x()) / static_cast<GraphicsScene *>(scene())->getScaleValue();
        double dy = (node2->y() - node1->y()) / static_cast<GraphicsScene *>(scene())->getScaleValue();
        return sqrt(dx * dx + dy * dy); // sqrt(dx² + dy²)
    }
    return 0;
}

double Rod::getAngle() const // this fct returns an angle with a value between [-pi/2; +pi/2]
{
    if (node1 != nullptr && node2 != nullptr) {
        double dx = node2->x() - node1->x();
        double dy = node2->y() - node1->y();
        return atan2(- dy, dx); // because the scene's y-axis is downwards positive
    }
    return 0;
}

double Rod::wGraph(double x) const // returns w(x) in local coords
{
    double wx = 0;
    for (int i = 0; i < 4; i++) {
        double value = (T.inverse() * Eigen::Map<const Eigen::Vector6d>(u))(i) * getPhi(i, x); // transform global_u to local_u and multiply with interpolation-fct
        wx += value;
    }
    return wx;
}

double Rod::uGraph(double x) const // returns u(x) in local coords
{
    double ux = 0;
    for (int i = 4; i < 6; i++) {
        double value = (T.inverse() * Eigen::Map<const Eigen::Vector6d>(u))(i) * getPhi(i, x);
        ux += value;
    }
    return ux;
}

double Rod::getPhi(int index, double x) const // returns the value of the interpolation-fct for the given dof at the position x [m] in local coords
{
    double l = getLength();
    switch (index) {
    case 0:
        return 1 - 3 * pow(x / l, 2) + 2 * pow(x / l, 3);
    case 1:
        return x * pow(1 - x / l, 2);
    case 2:
        return 3 * pow(x / l, 2) - 2 * pow(x / l, 3);
    case 3:
        return x * (pow(x / l, 2) - x / l);
    case 4:
        return 1 - x / l;
    case 5:
        return x / l;
    default:
        qDebug() << "error in Rod::getPhi()";
        return 0;
    }
}

void Rod::setElementTransformationMatrix(const Eigen::Matrix6d &matrix)
{
    prepareGeometryChange();
    T = matrix;
}

QPainterPath Rod::getDeformedRod() const // returns the deformed shape of the rod with the coord-system origin in node1 and x (y) horizontal (vertical)
{
    QPainterPath p(QPointF(0, 0));
    double sv = maxDisplacement == 0 ? 1 : static_cast<GraphicsScene *>(scene())->getMaxDisplacementDistance() / maxDisplacement; // scale graphs to global max u or w
    double step = static_cast<GraphicsScene *>(scene())->getDisplacementCalculationStep();
    for (int t = 0; t <= 100; t += step) { // make t an int but use it as percentage from 0 to 100 %
        Eigen::Vector2d displacement(t / 100.0 * getLength() * static_cast<GraphicsScene *>(scene())->getScaleValue() + uGraph(t / 100.0 * getLength()) * sv,
                                     - wGraph(t / 100.0 * getLength()) * sv); // displacement = [a (in px) + u(a) * scale, - w(a) * scale]; - y because the y-axis is flipped
        Eigen::Matrix2d R;
        R << cos(-getAngle()), - sin(-getAngle()),
             sin(-getAngle()),   cos(-getAngle()); // rotation matrix for clockwise rotation because for the painter positive y is downwards
        Eigen::Vector2d r = R * displacement; // position vector r = (x, y)
        if (t == 0) { // move to the first point and connect all other points by a straight line
            p.moveTo(r(0), r(1));
        } else {
            p.lineTo(r(0), r(1));
        }

    }
    return p;
}

void Rod::setMaxDisplacement(double d)
{
    prepareGeometryChange();
    maxDisplacement = d;
    scene()->update(); // needed for updating the br of all rods (because maxDisplacement is static)
}

QRectF Rod::boundingRect() const
{
    if (node1 != nullptr && node2 != nullptr) {
        double left = fmin(node1->x(), node2->x());
        double right = fmax(node1->x(), node2->x());
        double top = fmin(node1->y(), node2->y());
        double bottom = fmax(node1->y(), node2->y());
        QRectF rect1(left - pen.width() / 2, top - pen.width() / 2, right - left + pen.width(), bottom - top + pen.width()); // br of undeformed rod
        QRectF rect2(getDeformedRod().boundingRect());
        return rect1.united(rect2);
    }
    return {0, 0, 0, 0};
}

void Rod::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();
    painter->setPen(pen);
    if (static_cast<MainWindow *>(scene()->parent())->getColorRods()) {
        painter->setPen(pen.differentColor(colorMap.getColor(getInnerForce())));
    }
    if (static_cast<MainWindow *>(scene()->parent())->getMarkZeroLoadingRods() && Utilities::isequal(getInnerForce(), 0)) {
        painter->setPen(pen.differentColor(Color(Qt::lightGray)));
    }
    if (isUnderHoverAction) {
        painter->setPen(pen.differentColor(hoverPenColor));
    }
    if (isInSelection) {
        painter->setPen(pen.differentColor(selectionPenColor));
    }
    if (node1 != nullptr && node2 != nullptr) {
        painter->drawLine(node1->pos(), node2->pos()); // draw rod
        if (static_cast<MainWindow *>(scene()->parent())->getDrawDeformedSystem()) { // draw displacement
            painter->save();
            painter->translate(node1->x(), node1->y());
            QPen dpen(Qt::gray);
            dpen.setWidth(3);
            painter->setPen(dpen);
            painter->drawPath(getDeformedRod());
            painter->restore();
        }
    }
    painter->restore();
}

QPainterPath Rod::shape() const
{
    QPainterPath p(QPointF(0, 0));
    p.moveTo(node1->pos());
    // calculate orthogonal vector to the rod-vector
    double a = node2->x() - node1->x(); // component x of known vector (rod-vector)
    double b = node2->y() - node1->y(); // component y of rod-vector
    double t = pen.width() / 2; // parameter to eliminate a or b
    QPointF vec; // orthogonal vector to rod-vector
    double angle = QLineF(QPointF(0, 0), node1->pos() - node2->pos()).angle();
    if (angle < 45) { // select vec-calculation depending on the angle of the rod to avoid inf/nan-values (if the divisor (a or b) becomes 0)
        vec = QPointF(- t * b / a, t);
    } else if (angle < 135) {
        vec = QPointF(t, - t * a / b);
    } else if (angle < 225) {
        vec = QPointF(- t * b / a, t);
    } else if (angle < 315) {
        vec = QPointF(t, - t * a / b);
    } else {
        vec = QPointF(- t * b / a, t);
    }
    p.lineTo(node1->pos() + vec);
    p.lineTo(node2->pos() + vec);
    p.lineTo(node2->pos() - vec);
    p.lineTo(node1->pos() - vec);
    p.lineTo(node1->pos());
    return p;
}

QJsonObject Rod::saveAsJson() const
{
    QJsonObject o(TrussElement::saveAsJson());
    o.insert(JsonKeys::node1, Utilities::convertAddressToString(node1));
    o.insert(JsonKeys::node2, Utilities::convertAddressToString(node2));
    o.insert(JsonKeys::id, getId());
    o.insert(JsonKeys::label, Utilities::convertAddressToString(label));
    o.insert(JsonKeys::elementType, static_cast<int>(ElementType::Rod));
    return o;
}

void Rod::loadFromJson(const QJsonObject &object)
{
    TrussElement::loadFromJson(object);
    id.resetId(object.value(JsonKeys::id).toString());
}

EasyChangeDialog *Rod::createEasyChangeDialog()
{
    return new RodDialog(scene()->views().first(), this); // gets handled by TrussElement
}

QString Rod::getElementSelectionDialogButtonText() const
{
    return QString("Rod (ID: ") + getId() + QString(")"); // use address of this as a unique identifier
}

QString Rod::getId() const
{
    return id.toString();
}
