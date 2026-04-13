#include "curvedbeamadder.h"

#include "elements/curvedbeam.h"
#include "elements/node.h"
#include "graphicsscene.h"

#include <QDebug>

CurvedBeamAdder::CurvedBeamAdder(GraphicsScene *graphicsScene) :
    scene(graphicsScene),
    curvedBeam(nullptr),
    node1(nullptr),
    virtualNode(nullptr)
{
    scene->closeEasyChangeDialog();
}

CurvedBeamAdder::~CurvedBeamAdder()
{
    if (curvedBeam != nullptr) {
        scene->removeItem(virtualNode);
        curvedBeam->takeNode2();
        delete virtualNode;
        curvedBeam->takeNode1();
        scene->removeItem(curvedBeam);
        delete curvedBeam;
    }
}

void CurvedBeamAdder::mouseClicked(QGraphicsSceneMouseEvent *event, Node *element)
{
    if (event->buttons() == Qt::LeftButton) {
        if (element == nullptr) {
            return;
        } else if (node1 == element) {
            return;
        } else if (curvedBeamExistsAlready(element)) {
            return;
        } else if (node1 == nullptr) {
            node1 = element;
            createCurvedBeam();
        } else {
            scene->removeItem(virtualNode);
            curvedBeam->takeNode2();
            delete virtualNode;
            virtualNode = nullptr;
            curvedBeam->setNode2(element);
            scene->update(curvedBeam->boundingRect().adjusted(-curvedBeam->getThickness(), -curvedBeam->getThickness(),
                                                               curvedBeam->getThickness(), curvedBeam->getThickness()));
            curvedBeam = nullptr;
            node1 = element;
            createCurvedBeam();
        }
    } else if (event->buttons() == Qt::RightButton) {
        scene->removeItem(virtualNode);
        curvedBeam->takeNode2();
        delete virtualNode;
        virtualNode = nullptr;
        scene->removeItem(curvedBeam);
        node1->removeRod(curvedBeam);
        delete curvedBeam;
        curvedBeam = nullptr;
        node1 = nullptr;
    }
}

void CurvedBeamAdder::positionChanged(QPointF newPosition)
{
    if (virtualNode != nullptr) {
        virtualNode->setPosition(newPosition);
    }
}

void CurvedBeamAdder::createCurvedBeam()
{
    if (virtualNode != nullptr) {
        qDebug() << "Error1 in CurvedBeamAdder occured";
    }
    virtualNode = new Node(node1->x(), node1->y());
    virtualNode->hide();
    scene->addItem(virtualNode);
    if (curvedBeam != nullptr) {
        qDebug() << "Error2 in CurvedBeamAdder occured";
    }
    curvedBeam = new CurvedBeam(node1, virtualNode);
    scene->addItem(curvedBeam);
}

bool CurvedBeamAdder::curvedBeamExistsAlready(Node *node) const
{
    auto rods = node->getRods();
    for (auto r : rods) {
        if (dynamic_cast<CurvedBeam *>(r) == nullptr) {
            continue; // skip straight rods — only block duplicate curved beams
        }
        if ((r->getNode1() == node1 && r->getNode2() == node) || (r->getNode1() == node && r->getNode2() == node1)) {
            return true;
        }
    }
    return false;
}
