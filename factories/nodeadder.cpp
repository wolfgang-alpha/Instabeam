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

#include "nodeadder.h"

#include "elements/node.h"
#include "graphicsscene.h"

NodeAdder::NodeAdder(GraphicsScene *graphicsScene) :
    scene(graphicsScene),
    node(new Node(0, 0)) // node gets either passed to scene permanently or deleted in dtor
{
    scene->addItem(node);
    node->setIsMoving(true);
    node->showEasyChangeDialog(); // setIsInSelection(true) is called by this fct
    node->hide(); // hide node and show it when the mouse is in the graphics-view
}

NodeAdder::~NodeAdder()
{
    if (node != nullptr) {
        scene->removeItem(node); // do not call removeElement
        delete node; // delete node if not added to scene permanently
    }
}

void NodeAdder::positionChanged(const QPointF &newPosition) // newPosition is in scene-coords, therefore no conversion needed
{
    node->setPositionAndUpdateDialog(newPosition);
    if (!node->isVisible()) { // show node when the mouse moves for the first time on the scene (because then the node can be drawn under the mouse)
        node->show();
    }
}

void NodeAdder::mouseClicked(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        addNodePermanently();
    }
}

void NodeAdder::addNodePermanently()
{
    node->setIsMoving(false);
    node->closeEasyChangeDialog();
    node = new Node(node->x(), node->y()); // node is managed by the scene therefore it is safe to create a new node and add it to the scene (at the same position as the old one)
    scene->addItem(node); // add the node immediately after creation to the scene  (for memory management, if node gets overwritten the scene has still a ptr to it)
    node->setIsMoving(true);
    node->showEasyChangeDialog();
}
