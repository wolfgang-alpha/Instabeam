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

#include "forceadder.h"

#include "elements/singleforce.h"
#include "elements/node.h"
#include "utilities.h"

ForceAdder::ForceAdder()
{
}

void ForceAdder::mouseClicked(QGraphicsSceneMouseEvent *event, Node *node)
{
    if (event->buttons() == Qt::LeftButton) {
        if (node == nullptr) { // user clicked on empty scene-space, do nothing
            return;
        } else { // create new bearing and add it to the scene
            auto force = new SingleForce(node); // the ownership is passed to the node (=parent) (the item is automatically added to the scene by the node)
            force->showEasyChangeDialog(); // show its dialog
        }
    }
}
