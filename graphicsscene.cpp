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

#include "graphicsscene.h"

#include "elements/trusselement.h"
#include "elements/node.h"
#include "factories/nodeadder.h"
#include "factories/rodadder.h"
#include "factories/curvedbeamadder.h"
#include "factories/bearingadder.h"
#include "factories/forceadder.h"
#include "factories/dimensionadder.h"
#include "elements/rod.h"
#include "elements/curvedbeam.h"
#include "utilities.h"
#include "calculator.h"
#include "widgets/mainwindow.h"
#include "jsonkeys.h"
#include "elements/bearing.h"
#include "elements/singleforce.h"
#include "elements/label.h"
#include "elements/dimension.h"
#include "factories/labeladder.h"

#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QJsonObject>
#include <QJsonArray>
#include <QPair>

GraphicsScene::GraphicsScene(MainWindow *parent) :
    QGraphicsScene(parent),
    nodeAdder(nullptr),
    rodAdder(nullptr),
    curvedBeamAdder(nullptr),
    bearingAdder(nullptr),
    forceAdder(nullptr),
    dimensionAdder(nullptr),
    labelAdder(nullptr),
    scaleValue(100),
    clickInEmptySceneSpace(false),
    maxDisplacementDistance(20.0),
    displacementCalculationStep(10.0)
{
    // with the default index-method a SIGSEGV-error occurs when an item gets removed via removeItem and the deleted, because event if the item is removed from the scene,
    // the BSP-tree keeps a ptr to it and on the next redraw of the scene it dereferences the ptr which causes a crash, therefore use no item-indexing
    setItemIndexMethod(QGraphicsScene::NoIndex);
}

GraphicsScene::GraphicsScene(const QJsonObject &object, MainWindow *parent) :
    QGraphicsScene(parent),
    nodeAdder(nullptr),
    rodAdder(nullptr),
    curvedBeamAdder(nullptr),
    bearingAdder(nullptr),
    forceAdder(nullptr),
    dimensionAdder(nullptr),
    labelAdder(nullptr),
    scaleValue(100),
    maxDisplacementDistance(20.0),
    displacementCalculationStep(10.0)
{
    setItemIndexMethod(QGraphicsScene::NoIndex);
    // firstly create the elements (with nullptrs as member-ptrs), then link them together (set the member-ptrs to the correct address)
    auto elements = object.value(JsonKeys::items).toArray();
    QList<QPair<QString, TrussElement *>> memoryMap; // this map holds pairs of the old-address (= loaded from file) and the new-address (= new constructed) of an element
    // construct every element
    for (auto element : elements) {
        switch (static_cast<ElementType>(element.toObject().value(JsonKeys::elementType).toInt())) {
        case ElementType::Node:
            setupElementFromJson<Node>(element, memoryMap);
            break;
        case ElementType::Rod:
            setupElementFromJson<Rod>(element, memoryMap);
            break;
        case ElementType::CurvedBeam:
            setupElementFromJson<CurvedBeam>(element, memoryMap);
            break;
        case ElementType::Bearing:
            setupElementFromJson<Bearing>(element, memoryMap);
            break;
        case ElementType::SingleForce:
            setupElementFromJson<SingleForce>(element, memoryMap);
            break;
        case ElementType::Label:
            setupElementFromJson<Label>(element, memoryMap);
            break;
        case ElementType::Dimension:
            setupElementFromJson<Dimension>(element, memoryMap);
            break;
        default:
            qFatal("error 1 in GraphicsScene::GraphicsScene(QJsonObject, MainWindow) occured: invalid element-type");
        }
    }
    // link elements together
    for (auto pair : memoryMap) { // loop through every pair (oldAddress, newAddress (= ptr-to-object))
        for (int i = 0; i < elements.size(); i++) { // check every element if it has the current oldAddress as a member-ptr
            auto e = elements.at(i).toObject();
            if (pair.first == e.value(JsonKeys::parentItem).toString()) { // if the oldAddress equals a parent's oldAddress, then set the parent to the newAddress
                memoryMap.at(i).second->setParentItem(pair.second);
            } // no else-case because a element can be a parent and a member of an element
            switch (static_cast<ElementType>(e.value(JsonKeys::elementType).toInt())) { // switch the execution-path depending on the type of the element (to call different fcts)
            case ElementType::Rod:
            case ElementType::CurvedBeam:
                if (pair.first == e.value(JsonKeys::label).toString()) { // if the oldAddress is the rod's label, set the rod's member-ptr to the newAddress of the same label
                    static_cast<Rod *>(memoryMap.at(i).second)->setLabel(static_cast<Label *>(pair.second)); // memoryMap and elements have the same order
                } else if (pair.first == e.value(JsonKeys::node1).toString()) { // check if the element at the oldAddress is the node1 of the rod
                    static_cast<Rod *>(memoryMap.at(i).second)->linkNode1(static_cast<Node *>(pair.second)); // if so set the member-ptr to the newAddress of the same element
                } else if (pair.first == e.value(JsonKeys::node2).toString()) {
                    static_cast<Rod *>(memoryMap.at(i).second)->linkNode2(static_cast<Node *>(pair.second));
                }
                break;
            case ElementType::Node:
                if (pair.first == e.value(JsonKeys::label).toString()) { // check for label
                    static_cast<Node *>(memoryMap.at(i).second)->setLabel(static_cast<Label *>(pair.second));
                } else {
                    for (auto rod : e.value(JsonKeys::rods).toArray()) { // loop through the array of rod-address-strings and check if one matches the oldAddress of the current pair
                        if (pair.first == rod.toString()) {
                            static_cast<Node *>(memoryMap.at(i).second)->addRod(static_cast<Rod *>(pair.second));
                        }
                    }
                }
                break;
            case ElementType::Bearing:
                // nothing to link
                break;
            case ElementType::SingleForce:
                // nothing to link
                break;
            case ElementType::Label:
                // nothing to link
                break;
            case ElementType::Dimension:
                if (pair.first == e.value(JsonKeys::label).toString()) { // see rod for comments
                    static_cast<Dimension *>(memoryMap.at(i).second)->setLabel(static_cast<Label *>(pair.second));
                } else if (pair.first == e.value(JsonKeys::node1).toString()) {
                    static_cast<Dimension *>(memoryMap.at(i).second)->linkNode1(static_cast<Node *>(pair.second));
                } else if (pair.first == e.value(JsonKeys::node2).toString()) {
                    static_cast<Dimension *>(memoryMap.at(i).second)->linkNode2(static_cast<Node *>(pair.second));
                }
                break;
            default:
                qFatal("error 2 in GraphicsScene::GraphicsScene(QJsonObject, MainWindow) occured: invalid element-type");
            }
        }
    }
}

GraphicsScene::~GraphicsScene() // the items that the scene contains get deleted within ~QGraphicsScene()
{
    // close any open adder
    if (nodeAdder != nullptr) {
        nodeAdder.reset();
    } else if (rodAdder != nullptr) {
        rodAdder.reset();
    } else if (curvedBeamAdder != nullptr) {
        curvedBeamAdder.reset();
    } else if (bearingAdder != nullptr) {
        bearingAdder.reset();
    } else if (forceAdder != nullptr) {
        forceAdder.reset();
    } else if (dimensionAdder != nullptr) {
        dimensionAdder.reset();
    } else if (labelAdder != nullptr) {
        labelAdder.reset();
    }
}

QList<TrussElement *> GraphicsScene::getElements(const QPointF &point) const
{
    QList<TrussElement *> list;
    for (TrussElement *element : Utilities::convertTo<TrussElement *>(items())) {
        if (element->contains(element->mapFromScene(point))) {
            list.append(element);
        }
    }
    return list;
}

TrussElement *GraphicsScene::getElementAt(const QPointF &point) const
{
    for (TrussElement *element : Utilities::convertTo<TrussElement *>(items())) {
        if (element->contains(element->mapFromScene(point))) {
            return element;
        }
    }
    return nullptr;
}

Node *GraphicsScene::getNodeAt(const QPointF &point) const
{
    auto nodes = Utilities::getAllElementsOfType<Node *>(getElements(point));
    if (nodes.isEmpty()) {
        return nullptr; // if there is no node at the given position, return a nullptr
    } else {
        for (auto node : nodes) {
            if (node->isVisible()) { // only return visible nodes (this prevents the returning of the virtualNode of the rodAdder)
                return node;
            }
        }
        return nullptr; // if no node is visible, return a nullptr
    }
}

void GraphicsScene::removeElement(TrussElement *element)
{
    delete element; // deletes element and thus removes itself from the scene in ~QGraphisItem()
    // the dtor of the QGraphicsItem also removes the element from the parent's (if any) children-list
    // the dtor of the TrussElement removes all its children from the scene
}

void GraphicsScene::removeElement(const QList<TrussElement *> &elementList)
{
    while (!elementList.isEmpty()) { // while !empty-loop because if elementList contains tha parent and its child the length of the list can change if the parent gets deleted first
        removeElement(elementList.first());
    }
}

void GraphicsScene::removeElementAt(const QPointF &point)
{
    TrussElement *element = getElementAt(point);
    if (element != nullptr) {
        removeElement(element);
    }
}

void GraphicsScene::clearScene()
{
    removeElement(Utilities::convertTo<TrussElement *>(items()));
}

void GraphicsScene::validateNode(Node *node) // if a node has no rods connected, remove the node
{
    if (node->getNumberOfRods() < 1) {
        removeElement(node);
    }
}

void GraphicsScene::addNode(bool checked)
{
    if (checked) {
        nodeAdder = std::make_unique<NodeAdder>(this);
    } else {
        nodeAdder.reset(nullptr); // calls dtor of NodeAdder which cleans up
    }
}

void GraphicsScene::addRod(bool checked)
{
    if (checked) {
        rodAdder = std::make_unique<RodAdder>(this);
    } else {
        rodAdder.reset(nullptr); // calls dtor of RodAdder which cleans up
    }
}

void GraphicsScene::addCurvedBeam(bool checked)
{
    if (checked) {
        curvedBeamAdder = std::make_unique<CurvedBeamAdder>(this);
    } else {
        curvedBeamAdder.reset(nullptr);
    }
}

void GraphicsScene::addBearing(bool checked)
{
    if (checked) {
        closeEasyChangeDialog();
        bearingAdder = std::make_unique<BearingAdder>();
    } else {
        bearingAdder.reset(nullptr);
    }
}

void GraphicsScene::addForce(bool checked)
{
    if (checked) {
        closeEasyChangeDialog();
        forceAdder = std::make_unique<ForceAdder>();
    } else {
        forceAdder.reset(nullptr);
    }
}

void GraphicsScene::addDimension(bool checked)
{
    if (checked) {
        closeEasyChangeDialog();
        dimensionAdder = std::make_unique<DimensionAdder>(this);
    } else {
        dimensionAdder.reset(nullptr); // calls the dtor of the DimensionAdder which cleans up
    }
}

void GraphicsScene::addLabel(bool checked)
{
    if (checked) {
        closeEasyChangeDialog();
        labelAdder = std::make_unique<LabelAdder>(this);
    } else {
        labelAdder.reset(nullptr); // calls dtor of LabelAdder which cleans up
    }
}

TrussElement *GraphicsScene::getEasyChangeDialogElement() const
{
    for (TrussElement *element : Utilities::convertTo<TrussElement *>(items())) {
        if (element->easyChangeDialogIsShown()) {
            return element;
        }
    }
    return nullptr;
}

void GraphicsScene::closeEasyChangeDialog()
{
    getEasyChangeDialogElement()->closeEasyChangeDialog();
}

QJsonObject GraphicsScene::saveAsJson() const
{
    QJsonArray a;
    for (auto element : Utilities::convertTo<TrussElement *>(items())) {
        a.append(element->saveAsJson());
    }
    return {QPair<QString, QJsonValue>(JsonKeys::items, a)};
}

void GraphicsScene::setScaleValue(double newScaleValue)
{
    if (std::islessgreater(newScaleValue, scaleValue)) { // if (newScaleValue != scaleValue)
        for (auto item : items()) {
            if (auto node = dynamic_cast<Node *>(item)) {
                node->setXPos(node->x() / scaleValue * newScaleValue);
                node->setYPos(node->y() / scaleValue * newScaleValue);
            }
        }
        scaleValue = newScaleValue;
    }
}

void GraphicsScene::setMaxDisplacementDistance(double d)
{
    for (auto rod : Utilities::getAllElementsOfType<Rod *>(items())) {
        rod->forwardPrepareGeometryChange();
    }
    maxDisplacementDistance = d;
}

template<typename T>
void GraphicsScene::setupElementFromJson(const QJsonValue &jsonElement, QList<QPair<QString, TrussElement *>> &memoryMap)
{
    auto trussElement = new T(); // default construct the element (the scene takes ownership of the trussElement later)
    trussElement->loadFromJson(jsonElement.toObject()); // alter values of element
    memoryMap.append(QPair<QString, TrussElement *>(jsonElement.toObject().value(JsonKeys::thisItem).toString(), trussElement)); // append element to map (oldAddress, newAddress)
    addItem(trussElement); // pass the ownership of the trussElement to the scene
}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (nodeAdder != nullptr) {
        nodeAdder->mouseClicked(event);
        event->accept();
    } else if (rodAdder != nullptr) {
        Node *node = getNodeAt(event->scenePos());
        rodAdder->mouseClicked(event, node);
        event->accept();
    } else if (curvedBeamAdder != nullptr) {
        Node *node = getNodeAt(event->scenePos());
        curvedBeamAdder->mouseClicked(event, node);
        event->accept();
    } else if (bearingAdder) {
        Node *node = getNodeAt(event->scenePos());
        bearingAdder->mouseClicked(event, node);
        event->accept();
    } else if (forceAdder) {
        Node *node = getNodeAt(event->scenePos());
        forceAdder->mouseClicked(event, node);
        event->accept();
    } else if (dimensionAdder) {
        Node *node = getNodeAt(event->scenePos());
        dimensionAdder->mouseClicked(event, node);
        event->accept();
    } else if (labelAdder) {
        labelAdder->mouseClicked(event);
        event->accept();
    } else {
        if (event->buttons() == Qt::LeftButton) {
            if (getElementAt(event->scenePos()) == nullptr) { // click in empty scene space occured
                closeEasyChangeDialog(); // if an easychangedialog is open close it
                clickInEmptySceneSpace = true; // remember click
            }
        }
        QGraphicsScene::mousePressEvent(event);
    }
}

void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (nodeAdder != nullptr) {
        nodeAdder->positionChanged(event->scenePos());
        event->accept();
    } else if (rodAdder != nullptr) {
        rodAdder->positionChanged(event->scenePos());
        event->accept();
    } else if (curvedBeamAdder != nullptr) {
        curvedBeamAdder->positionChanged(event->scenePos());
        event->accept();
    } else if (dimensionAdder != nullptr) {
        dimensionAdder->positionChanged(event->scenePos());
        event->accept();
    } else if (labelAdder != nullptr) {
        labelAdder->positionChanged(event->scenePos());
        event->accept();
    }
    QGraphicsScene::mouseMoveEvent(event); // pass event to parent to handle hovering of the nodes
    // recalculate the system every time the mouse moves when not in adding-mode
    if (nodeAdder == nullptr && rodAdder == nullptr && curvedBeamAdder == nullptr && forceAdder == nullptr && bearingAdder == nullptr && dimensionAdder == nullptr) {
        static_cast<MainWindow *>(parent())->setStatusBarMessage(Calculator::calculate(this));
        if (static_cast<MainWindow *>(parent())->getColorRods() || static_cast<MainWindow *>(parent())->getMarkZeroLoadingRods()) {
            update(itemsBoundingRect()); // redraw whole scene after recalculating the model to properly color the rods
        }
    }
    if (clickInEmptySceneSpace == true) {
        // todo: implement mouse-moving the scene
    }
}

void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    clickInEmptySceneSpace = false;
    QGraphicsScene::mouseReleaseEvent(event);
}
