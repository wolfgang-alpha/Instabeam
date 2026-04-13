#include "node.h"

#include "rod.h"
#include "utilities.h"
#include "widgets/easychange/nodedialog.h"
#include "graphicsscene.h"
#include "bearing.h"
#include "widgets/mainwindow.h"
#include "label.h"
#include "dimension.h"
#include "jsonkeys.h"
#include "singleforce.h"

#include <QGraphicsView>
#include <QPainter>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

Node::Node() :
    TrussElement(0, 0, nullptr),
    nodeType(NodeType::GerberJoint),
    pen(Pen(Color(Qt::black), 3)),
    radius(5),
    rods(QList<Rod *>()),
    id(Id<Node>()),
    label(nullptr),
    dimensions(QList<Dimension *>()),
    fx(0),
    fy(0),
    mz(0),
    calcId(-1) // has to be -1 if unset
{
    setZValue(ElementZValue::Node_);
}

Node::Node(double xPosition, double yPosition, NodeType type) :
    TrussElement(xPosition, yPosition, nullptr),
    nodeType(type),
    pen(Pen(Color(Qt::black), 3)),
    radius(5),
    rods(QList<Rod *>()),
    id(Id<Node>()),
    label(new Label(id.toString(), 10, - 10, this)), // deleted in ~QGraphicsItem()
    dimensions(QList<Dimension *>()),
    fx(0),
    fy(0),
    mz(0),
    calcId(-1) // has to be -1 if unset
{
    setZValue(ElementZValue::Node_);
    label->hide();
}

Node::~Node()
{
    while (!rods.isEmpty()) { // Rod::takeNode1() calls Node::removeRod() with itself as parameter, therefore rods is shrinking constantly
        auto rod = rods.first();
        if (rod->getNode1() == this) {
            rod->takeNode1(); // this sets rod->node1 to nullptr
        } else if (rod->getNode2() == this) { // this sets rod->node2 to nullptr
            rod->takeNode2(); // this sets rod->node2 to nullptr
        } else { // this case should not be entered because the node has to be either node1 or node2 of the rod
            qFatal("error 1 in dtor of node");
        }
        static_cast<GraphicsScene *>(scene())->removeElement(rod); // since the rod now has only one node attached remove it
    }
    while (!dimensions.isEmpty()) { // comments see above
        auto dim = dimensions.first();
        if (dim->getNode1() == this) {
            dim->takeNode1();
        } else if (dim->getNode2() == this) {
            dim->takeNode2();
        } else {
            qFatal("error 2 in dtor of node");
        }
        static_cast<GraphicsScene *>(scene())->removeElement(dim);
    }
}

void Node::addRod(Rod *rod)
{
    rods.append(rod);
}

void Node::removeRod(Rod *rod)
{
    if (!rods.isEmpty() && rods.contains(rod)) { // contains() is checked to prevent the removal of the same rod twice
        rods.removeOne(rod);
    }
}

int Node::getNumberOfRods() const
{
    return rods.size();
}

Bearing *Node::getBearing() const
{
    auto list = Utilities::getAllElementsOfType<Bearing *>(childItems());
    if (!list.isEmpty()) {
        return list.first();
    }
    return nullptr;
}

void Node::setLabel(Label *newLabel)
{
    if (label != nullptr) {
        delete label;
    }
    label = newLabel;
}

void Node::addDimension(Dimension *dimension)
{
    dimensions.append(dimension);
}

void Node::removeDimension(Dimension *dimension)
{
    if (!dimensions.isEmpty() && dimensions.contains(dimension)) {
        dimensions.removeOne(dimension);
    }
}

void Node::setNodeType(NodeType newType)
{
    if (nodeType != newType) {
        prepareGeometryChange(); // account for br-changes
        nodeType = newType;
    }
}

bool Node::hasAppliedForce() const
{
    return !Utilities::getAllElementsOfType<SingleForce *>(childItems()).isEmpty();
}

Eigen::Vector2d Node::getResultingAppliedForce() const
{
    Eigen::Vector2d f(0, 0); // resulting-applied-force-vector (has to be return [0, 0] if no force is applied because the calculator relies on this)
    for (auto child : childItems()) { // search the list of childrens and see if there are forces present, if so add them to the vector
        if (auto force = dynamic_cast<SingleForce *>(child)) {
            f(0) += force->getValue() * cos(force->getAngle()); // fx, positive: right
            f(1) += force->getValue() * sin(force->getAngle()); // fy, positive: up
        }
    }
    return f;
}

double Node::getResultingAppliedMoment() const // todo: implement!
{
    return 0;
}

double Node::getUx() const
{
    if (rods.isEmpty()) {
        return 0;
    }
    if (this == rods.first()->getNode1()) { // which rod doesn't matter because the displacement is the same for every rod connected to this node
        return rods.first()->getU(4);
    }
    return rods.first()->getU(5); // this == node2
}

double Node::getUy() const
{
    if (rods.isEmpty()) {
        return 0;
    }
    if (this == rods.first()->getNode1()) {
        return rods.first()->getU(0);
    }
    return rods.first()->getU(2); // this == node2
}

void Node::setXPos(double newXPos)
{
    for (Rod *rod : rods) {
        rod->forwardPrepareGeometryChange(); // notify the rods that one of its starting/ending points is about to change position
        rod->moveLabelX((newXPos - x()) / 2); // move the labels half the distance the node moves
    }
    for (Dimension *dim : dimensions) {
        dim->forwardPrepareGeometryChange();
        if (dim->getOrientation() == Orientation::Horizontal) {
            dim->moveLabelX((newXPos - x()) / 2); // move the label half the distance the node (and therefore the dimension) moves if the dimension is horizontal
        }
    }
    TrussElement::setXPos(newXPos);
}

void Node::setYPos(double newYPos)
{
    for (Rod *rod : rods) {
        rod->forwardPrepareGeometryChange();
        rod->moveLabelY((newYPos - y()) / 2);
    }
    for (Dimension *dim : dimensions) {
        dim->forwardPrepareGeometryChange();
        if (dim->getOrientation() == Orientation::Vertical) {
            dim->moveLabelY((newYPos - y()) / 2);
        }
    }
    TrussElement::setYPos(newYPos);
}

EasyChangeDialog *Node::createEasyChangeDialog()
{
    return new NodeDialog(scene()->views().first(), this); // ptr gets handled by TrussElement
}

QRectF Node::boundingRect() const
{
    if (nodeType == NodeType::GerberJoint) {
        return {- radius - pen.width() / 2, - radius - pen.width() / 2, 2 * radius + pen.width(), 2 * radius + pen.width()};
    } else {
        return {0.0 - pen.width() / 2, 0.0 - pen.width() / 2, 0.0 + pen.width(), 0.0 + pen.width()};
    }
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();
    painter->setPen(pen);
    if (isUnderHoverAction) {
        painter->setPen(pen.differentColor(hoverPenColor));
    }
    if (isInSelection) {
        painter->setPen(pen.differentColor(selectionPenColor));
    }
    if (nodeType == NodeType::GerberJoint) {
        QPainterPath p(QPointF(0, 0));
        p.addEllipse(QPointF(0, 0), radius, radius);
        QBrush brush;
        if (isInSelection || isUnderHoverAction) {
            brush = QBrush(painter->pen().color(), Qt::SolidPattern);
        } else {
            brush = QBrush(Qt::white, Qt::SolidPattern);
        }
        painter->fillPath(p, brush);
        painter->drawPath(p);
    } else { // todo: implement design for weld
        painter->drawPoint(0, 0);
    }
    painter->restore();
}

void Node::hoverEnterEvent(QGraphicsSceneHoverEvent *event) // reimplement this fct to make the node always hoverable
{
    // the event-handling is as follows: the graphicsView gets a mouse-move-event which it passes to the scene; the scene creates a graphics-scene-mouse-event which is passed to the
    // top-level item at the event's position, but if this item has a parent, it is passed to the parent instead and then propagates through every child item
    // to indicate the bearing is hovered over and not the node, check if the event happens on this item; if so, hover this item and the event is not propagated since there is no child
    // at the event's position, if not, do nothing with this item and let the event propagate to the correct child
    if (shape().contains(event->pos())) {
        isUnderHoverAction = true;
        update(boundingRect()); // repaint the element because the color changes
        event->accept();
    }
}

QString Node::getElementSelectionDialogButtonText() const
{
    if (nodeType == NodeType::GerberJoint) {
        return QString("Gerber joint (ID: ") + getId() + QString(")");
    } else {
        return QString("Weld (ID: ") + getId() + QString(")");
    }
}

QString Node::getId() const
{
    return id.toString();
}

QJsonObject Node::saveAsJson() const
{
    QJsonObject o(TrussElement::saveAsJson());
    o.insert(JsonKeys::nodeType, static_cast<int>(nodeType));
    QJsonArray a;
    for (auto rod : rods) {
        a.append(Utilities::convertAddressToString(rod));
    }
    o.insert(JsonKeys::rods, a);
    o.insert(JsonKeys::id, getId());
    o.insert(JsonKeys::label, Utilities::convertAddressToString(label));
    o.insert(JsonKeys::elementType, static_cast<int>(ElementType::Node));
    return o;
}

void Node::loadFromJson(const QJsonObject &object)
{
    TrussElement::loadFromJson(object);
    nodeType = static_cast<NodeType>(object.value(JsonKeys::nodeType).toInt());
    id.resetId(object.value(JsonKeys::id).toString());
}
