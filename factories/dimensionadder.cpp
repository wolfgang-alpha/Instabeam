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

#include "dimensionadder.h"

#include "elements/dimension.h"
#include "elements/node.h"
#include "graphicsscene.h"

#include <QGraphicsSceneMouseEvent>

DimensionAdder::DimensionAdder(GraphicsScene *graphicsScene) :
    scene(graphicsScene),
    dimension(nullptr),
    node1(nullptr),
    virtualNode(nullptr)
{
    scene->closeEasyChangeDialog();
}

DimensionAdder::~DimensionAdder()
{
    if (dimension != nullptr) { // there is still a dimension present
        scene->removeItem(virtualNode); // remove virtualNode from scene (do not call removeElement)
        dimension->takeNode2(); // break parent-child-relationship between dimension and virtualNode
        delete virtualNode;
        dimension->takeNode1(); // break parent-child-relationship between dimension and node1
        scene->removeItem(dimension); // remove dimension from scene
        delete dimension;
        // let node1 hold the ptr to the node because node1 gets out of scope at the end of this fct and then only the scene has the ptr to it again
        // no setting to nullptr of the ptrs because they get not checked for nullptr again since they get out of scope after this fct
    }
}

void DimensionAdder::mouseClicked(QGraphicsSceneMouseEvent *event, Node *node)
{
    if (event->buttons() == Qt::LeftButton) {
        if (node == nullptr) { // user didn't click on a node, do nothing
            return;
        }
        if (node == node1) { // user clicked on the same node again, do nothing
            return;
        }
        if (node1 == nullptr) { // user clicked on first node
            node1 = node; // store weak ptr to node
            createDimension();
        } else {
            scene->removeItem(virtualNode); // remove virtualNode from scene (do not call removeElement)
            dimension->takeNode2(); // break parent-child-relationship between dimension and virtualNode; the return-value of this fct is the virtualNode, therefore it is not stored
            delete virtualNode;
            virtualNode = nullptr;
            dimension->setNode2(node); // set node2 of dimension to the node the user clicked secondly
            // to remove artifacts which occured if the user clicked on the node at its edge the scene-space in which the jump could happen is updated
            scene->update(dimension->boundingRect().adjusted(- node->getRadius(), - node->getRadius(), node->getRadius(), node->getRadius()));
            dimension = nullptr; // the scene has now the ownership of the dimension
            node1 = node; // set second node as starting point for the next dimension
            createDimension();
        }
    } else if (event->buttons() == Qt::RightButton) {
        scene->removeItem(virtualNode); // remove virtualNode from scene (do not call removeElement)
        dimension->takeNode2(); // break parent-child-relationship between dimension and virtualNode
        delete virtualNode;
        virtualNode = nullptr;
        scene->removeItem(dimension); // remove dimension from scene (do not call removeElement)
        node1->removeDimension(dimension); // remove dimension from starting-node
        delete dimension;
        dimension = nullptr;
        node1 = nullptr; // let the scene handle node1 again
    }
}

void DimensionAdder::positionChanged(const QPointF &newPosition)
{
    if (virtualNode != nullptr) {
        virtualNode->setPosition(newPosition); // dialog updating not necessary because it is not shown anyways
    }
}

void DimensionAdder::createDimension()
{
    if (virtualNode != nullptr) {
        qDebug() << "Error1 in DimensionAdder occured";
    }
    virtualNode = new Node(node1->x(), node1->y()); // gets temporarily added to the scene and then deleted if the user clicks on another node
    virtualNode->hide();
    scene->addItem(virtualNode);
    if (dimension != nullptr) {
        qDebug() << "Error2 in DimensionAdder occured";
    }
    dimension = new Dimension(node1, virtualNode); // gets temporarily (or permanently) added to the scene and is deleted if the user aborts the adding of dimensions
    scene->addItem(dimension); // add dimension to scene immediately after creation (for memory-management-reasons)
}
