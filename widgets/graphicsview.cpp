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

#include "graphicsview.h"

#include "elements/trusselement.h"
#include "graphicsscene.h"
#include "utilities.h"
#include "widgets/elementselectiondialog.h"
#include "jsonkeys.h"
#include "widgets/mainwindow.h"
#include "elements/label.h"

#include <QMouseEvent>
#include <QJsonObject>

GraphicsView::GraphicsView(QWidget *parent) :
    QGraphicsView(parent),
    ctrlPressed(false),
    elementSelectionDialog(nullptr),
    minSceneWidth(1440),
    minSceneHeight(900),
    mouseClickedInBackground(false)
{
    setMouseTracking(true); // to be able to deliver hover-events
    setRenderHint(QPainter::Antialiasing);
}

GraphicsView::~GraphicsView()
{
    if (elementSelectionDialog != nullptr) { // if a elementSelectionDialog is present upon destruction of the view delete the dialog
        delete elementSelectionDialog;
    }
    closeEasyChangeDialog(); // close (and delete) the easychange-dialog (do this because otherwise it is deleted in ~QGraphicsView() and the underlying element doesn't get notified)
}

GraphicsScene *GraphicsView::getGraphicsScene() const
{
    return static_cast<GraphicsScene *>(scene()); // static ptr-downcast is safe because the scene of this view has to be a GraphicsScene, not a QGraphicsScene
}

void GraphicsView::resizeSceneRect()
{
    auto rect = Utilities::max(getGraphicsScene()->itemsBoundingRect(), QRectF(- minSceneWidth / 2, - minSceneHeight / 2, minSceneWidth, minSceneHeight));
    getGraphicsScene()->setSceneRect(rect); // todo: find more suitable way to control the scene size
}

void GraphicsView::toggleNodeAdder(bool checked)
{
    getGraphicsScene()->addNode(checked);
}

void GraphicsView::toggleRodAdder(bool checked)
{
    getGraphicsScene()->addRod(checked);
}

void GraphicsView::toggleCurvedBeamAdder(bool checked)
{
    getGraphicsScene()->addCurvedBeam(checked);
}

void GraphicsView::toggleBearingAdder(bool checked)
{
    getGraphicsScene()->addBearing(checked);
}

void GraphicsView::toggleForceAdder(bool checked)
{
    getGraphicsScene()->addForce(checked);
}

void GraphicsView::toggleDimensionAdder(bool checked)
{
    getGraphicsScene()->addDimension(checked);
}

void GraphicsView::toggleLabelAdder(bool checked)
{
    getGraphicsScene()->addLabel(checked);
}

void GraphicsView::closeEasyChangeDialog()
{
    getGraphicsScene()->closeEasyChangeDialog();
}

void GraphicsView::setMinSceneWidth(double newMinSceneWidth)
{
    minSceneWidth = newMinSceneWidth;
    resizeSceneRect();
}

void GraphicsView::setMinSceneHeight(double newMinSceneHeight)
{
    minSceneHeight = newMinSceneHeight;
    resizeSceneRect();
}

QJsonObject GraphicsView::saveAsJson() const
{
    return {QPair<QString, QJsonValue>(JsonKeys::minSceneHeight, minSceneHeight),
            QPair<QString, QJsonValue>(JsonKeys::minSceneWidth, minSceneWidth),
            QPair<QString, QJsonValue>(JsonKeys::graphicsScene, getGraphicsScene()->saveAsJson())};
}

void GraphicsView::loadFromJson(const QJsonObject &object, MainWindow *parentForScene)
{
    minSceneHeight = object.value(JsonKeys::minSceneHeight).toDouble();
    minSceneWidth = object.value(JsonKeys::minSceneWidth).toDouble();
    delete scene();
    setScene(new GraphicsScene(object.value(JsonKeys::graphicsScene).toObject(), parentForScene)); // parent of the graphicsScene is parentForScene
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    mouseClickedInBackground = true;
    QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    mouseClickedInBackground = false;
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseClickedInBackground) {
        // todo: implement translation of the scene via mouse-drag
    }
    QGraphicsView::mouseMoveEvent(event);
    resizeSceneRect();
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (ctrlPressed) {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // zoom around mouse
        double factor = event->angleDelta().y(); // get scroll angle
        factor = factor < 0 ? 0.9 : 1.1; // wheel forward = zoom in
        scale(factor, factor);
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Control:
        ctrlPressed = true;
        break;
    case Qt::Key_Delete: {
        auto element = getGraphicsScene()->getEasyChangeDialogElement();
        if (dynamic_cast<Label *>(element) == nullptr || dynamic_cast<Label *>(element)->parentItem() == nullptr) {
            getGraphicsScene()->removeElement(element); // delete the currently selected element if it is not a label or if it is a label without a parent, otherwise do nothing
        }
        break;
    }
    case Qt::Key_Shift: {
        QList<TrussElement *> list = getGraphicsScene()->getElements(mapToScene(mapFromGlobal(QCursor::pos()))); // pos is in global coords, map it to the scene
        if (list.size() > 1) {
            if (elementSelectionDialog != nullptr) { // this can happen if the key-release-event is delivered to another widget
                elementSelectionDialog->close(); // close and delete the dialog before creating a new one
                delete elementSelectionDialog;
            }
            elementSelectionDialog = new ElementSelectionDialog(list, this); // is save because to press shift again the user has to release it first and thereby it gets deleted
            elementSelectionDialog->show();
        }
        break;
    }
    default:
        QGraphicsView::keyPressEvent(event);
        return; // to not set the event->accept()-flag when the event was passed to the parent
    }
    event->accept();
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Control:
        ctrlPressed = false;
        break;
    case Qt::Key_Shift:
        if (elementSelectionDialog != nullptr) {
            elementSelectionDialog->close(); // close and delete the dialog when the user releases the shift-key
            delete elementSelectionDialog;
            elementSelectionDialog = nullptr; // to use it as a flag
        }
        break;
    default:
        QGraphicsView::keyReleaseEvent(event);
        return; // to not set the event->accept()-flag when the event was passed to the parent
    }
    event->accept();
}
