#include "nodedialog.h"

#include "widgets/lineedit.h"
#include "elements/node.h"
#include "elements/bearing.h"
#include "graphicsscene.h"

#include <QGridLayout>
#include <QLabel>

NodeDialog::NodeDialog(QWidget *parent, Node *elementNode) :
    EasyChangeDialog(parent, elementNode),
    node(elementNode),
    idLabel(new QLabel(node->getId(), this)),
    xPosInput(new LineEdit(QString::number(node->x() / static_cast<GraphicsScene *>(node->scene())->getScaleValue()), this)),
    yPosInput(new LineEdit(QString::number(- node->y() / static_cast<GraphicsScene *>(node->scene())->getScaleValue()), this)), // flip y-axis
    uxLabel(new QLabel(QString::number(node->getUx()), this)),
    uyLabel(new QLabel(QString::number(node->getUy()), this))
{
    typeBox = new QComboBox(this);
    auto texts = QStringList({"Gerber joint", "Weld"}); // has to be the same order as in the NodeType-enum
    typeBox->addItems(texts);
    typeBox->setCurrentIndex(static_cast<int>(node->getNodeType()));

    auto gridLayout = new QGridLayout(); // parent gets set later
    gridLayout->addWidget(new QLabel("Node no.:"), 0, 0);
    gridLayout->addWidget(idLabel, 0, 1);
    gridLayout->addWidget(new QLabel("x-position [m]:"), 1, 0);
    connectLineEdit(xPosInput, this, &NodeDialog::setXPos);
    gridLayout->addWidget(xPosInput, 1, 1);
    gridLayout->addWidget(new QLabel("y-position [m]:"), 2, 0);
    connectLineEdit(yPosInput, this, &NodeDialog::setYPos);
    gridLayout->addWidget(yPosInput, 2, 1);
    gridLayout->addWidget(new QLabel("Node type:"), 3, 0);
    connect(typeBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &NodeDialog::setType);
    if (node->getBearing() != nullptr) { // disable type selection in case of fixed clamping, because the node then has to be a weld
        if (node->getBearing()->getBearingType() == BearingType::FixedClamping) {
            typeBox->setDisabled(true);
        }
    }
    gridLayout->addWidget(typeBox, 3, 1);
    gridLayout->addWidget(new QLabel("x-displacement [m]:"), 4, 0);
    gridLayout->addWidget(uxLabel, 4, 1);
    gridLayout->addWidget(new QLabel("y-displacement [m]:"), 5, 0);
    gridLayout->addWidget(uyLabel, 5, 1);
    auto finalLayout = new QHBoxLayout(); // parent gets set later
    finalLayout->addLayout(gridLayout);
    setupEasyChangeDialog(finalLayout); // add ok- and cancel-buttons to the above created layout, give the created widgets a parent and add the combined layout to the scene
}

void NodeDialog::setXPos()
{
    node->setXPos(xPosInput->text().toDouble() * static_cast<GraphicsScene *>(node->scene())->getScaleValue());
}

void NodeDialog::setYPos()
{
    node->setYPos(- yPosInput->text().toDouble() * static_cast<GraphicsScene *>(node->scene())->getScaleValue()); // flip y-axis
}

void NodeDialog::setType(int newType)
{
    node->setNodeType(static_cast<NodeType>(newType));
}

void NodeDialog::okPressed(bool)
{
    setXPos();
    setYPos();
    addNodePermanentlyToScene(); // checks if the node-adder is active and if so adds the node permanently to the scene
    node->closeEasyChangeDialog(); // close dialog via element member-fct to unselect the element when the dialog is closed
}

void NodeDialog::updateDialog()
{
    xPosInput->setText(QString::number(node->x() / static_cast<GraphicsScene *>(node->scene())->getScaleValue()));
    yPosInput->setText(QString::number(- node->y() / static_cast<GraphicsScene *>(node->scene())->getScaleValue())); // flip y-axis
    uxLabel->setText(QString::number(node->getUx()));
    uyLabel->setText(QString::number(node->getUy()));
}
