#include "rodadder.h"

#include "elements/rod.h"
#include "elements/curvedbeam.h"
#include "elements/node.h"
#include "graphicsscene.h"

#include <QDebug>

RodAdder::RodAdder(GraphicsScene *graphicsScene) :
    scene(graphicsScene),
    rod(nullptr),
    node1(nullptr),
    virtualNode(nullptr)
{
    scene->closeEasyChangeDialog();
}

RodAdder::~RodAdder()
{
    if (rod != nullptr) { // there is still a rod present
        scene->removeItem(virtualNode); // remove virtualNode from scene (do not call removeElement)
        rod->takeNode2(); // break parent-child-relationship between rod and virtualNode
        delete virtualNode;
        rod->takeNode1(); // break parent-child-relationship between rod and node1
        scene->removeItem(rod); // remove rod from scene (without checking if the first node is now invalid)
        delete rod;
        // let node1 hold the ptr to the node because node1 gets out of scope at the end of this fct and then only the scene has the ptr to it again
        // no setting to nullptr of the ptrs because they get not checked for nullptr again since they get out of scope after this fct
    }
}

void RodAdder::mouseClicked(QGraphicsSceneMouseEvent *event, Node *element)
{
    if (event->buttons() == Qt::LeftButton) {
        if (element == nullptr) { // user clicked on empty scene space
            return; // ignore click
        } else if (node1 == element) { // user clicked on the same node, don't create a rod
            return; // ignore click
        } else if (rodExistsAlready(element)) { // if there is already a rod between node1 and element don't add another one
            return; // ignore click
        } else if (node1 == nullptr) { // user clicked on first node: create rod which follows the mouse
            node1 = element;
            createRod();
        } else { // user clicked on second node: add rod permanently between first and second node
            scene->removeItem(virtualNode); // remove virtualNode from scene (do not call removeElement)
            rod->takeNode2(); // break parent-child-relationship between rod and virtualNode; the return-value of this fct has to be the virtualNode, therefore it is not stored
            delete virtualNode;
            virtualNode = nullptr;
            rod->setNode2(element); // set node2 of rod to the node the user clicked secondly
            // to remove artifacts which occured if the user clicked on the node at its edge (because then the rod "jumps" to the middle of the node) the scene-space in which the
            // jump could happen (including half the rod's thickness pixels extra) is updated (and the rod itself too)
            scene->update(rod->boundingRect().adjusted(- rod->getThickness(), - rod->getThickness(), rod->getThickness(), rod->getThickness()));
            rod = nullptr; // the scene has now the ownership of the rod
            node1 = element; // set second node as starting point for the next rod
            createRod();
        }
    } else if (event->buttons() == Qt::RightButton) { // clear reference point upon right-click
        scene->removeItem(virtualNode); // remove virtualNode from scene (do not call removeElement)
        rod->takeNode2(); // break parent-child-relationship between rod and virtualNode
        delete virtualNode;
        virtualNode = nullptr;
        scene->removeItem(rod); // remove rod from scene (do not call removeElement)
        node1->removeRod(rod); // remove rod from starting-node
        delete rod;
        rod = nullptr;
        node1 = nullptr; // let the scene handle node1 again
    }
}

void RodAdder::positionChanged(QPointF newPosition)
{
    if (virtualNode != nullptr) {
        virtualNode->setPosition(newPosition); // dialog updating not necessary because it is not shown anyways
    }
}

void RodAdder::createRod() {
    if (virtualNode != nullptr) {
        qDebug() << "Error1 in RodAdder occured";
    }
    virtualNode = new Node(node1->x(), node1->y()); // gets temporarily added to the scene and then deleted if the user clicks on another node
    virtualNode->hide();
    scene->addItem(virtualNode);
    if (rod != nullptr) {
        qDebug() << "Error2 in RodAdder occured";
    }
    rod = new Rod(node1, virtualNode); // gets temporarily (or permanently) added to the scene and is deleted if the user aborts the adding of rods
    scene->addItem(rod); // add rod to scene immediately after creation (for memory-management-reasons)
}

bool RodAdder::rodExistsAlready(Node *node) const // returns true if there is already a straight rod between the starting node of this rod and the given node
{
    auto rods = node->getRods();
    for (auto r : rods) {
        if (dynamic_cast<CurvedBeam *>(r) != nullptr) {
            continue; // skip curved beams — only block duplicate straight rods
        }
        if ((r->getNode1() == node1 && r->getNode2() == node) || (r->getNode1() == node && r->getNode2() == node1)) {
            return true;
        }
    }
    return false;
}
