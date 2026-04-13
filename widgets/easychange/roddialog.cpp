#include "roddialog.h"

#include "elements/rod.h"
#include "elements/node.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

RodDialog::RodDialog(QWidget *parent, Rod *elementRod) :
    EasyChangeDialog(parent, elementRod),
    rod(elementRod),
    rodForceLabel(new QLabel(QString::number(elementRod->getInnerForce()), this)), // parent is this
    u1xLabel(new QLabel(QString::number(elementRod->getU(4)), this)), // parent is this
    u1yLabel(new QLabel(QString::number(elementRod->getU(0)), this)), // parent is this
    u1zLabel(new QLabel(QString::number(elementRod->getU(1)), this)), // parent is this
    u2xLabel(new QLabel(QString::number(elementRod->getU(5)), this)), // parent is this
    u2yLabel(new QLabel(QString::number(elementRod->getU(2)), this)), // parent is this
    u2zLabel(new QLabel(QString::number(elementRod->getU(3)), this)), // parent is this
    EInput(new LineEdit(QString::number(elementRod->getE()), this)), // parent is this
    AInput(new LineEdit(QString::number(elementRod->getA()), this)), // parent is this
    IInput(new LineEdit(QString::number(elementRod->getI()), this)) // parent is this
{
    auto layout = new QGridLayout(); // parent gets set later
    layout->addWidget(new QLabel("Young's modulus [N/m²]:", this), 0, 0); // parent of the label is this
    connectLineEdit(EInput, this, &RodDialog::setE);
    layout->addWidget(EInput, 0, 1);
    layout->addWidget(new QLabel("Cross-sectional area [m²]:", this), 1, 0);
    connectLineEdit(AInput, this, &RodDialog::setA);
    layout->addWidget(AInput, 1, 1);
    layout->addWidget(new QLabel("Second moment of area y [m<sup>4</sup>]:", this), 2, 0);
    connectLineEdit(IInput, this, &RodDialog::setI);
    layout->addWidget(IInput, 2, 1);
    layout->addWidget(new QLabel("Normal force magnitude [N]:", this), 3, 0);
    layout->addWidget(rodForceLabel, 3, 1);
    layout->addWidget(new QLabel(QString("x-displacement node ") + rod->getNode1()->getId() + QString(" [m]:"), this), 4, 0);
    layout->addWidget(u1xLabel, 4, 1);
    layout->addWidget(new QLabel(QString("y-displacement node ") + rod->getNode1()->getId() + QString(" [m]:"), this), 5, 0);
    layout->addWidget(u1yLabel, 5, 1);
    layout->addWidget(new QLabel(QString("z-rotation node ") + rod->getNode1()->getId() + QString(" [rad]:"), this), 6, 0);
    layout->addWidget(u1zLabel, 6, 1);
    layout->addWidget(new QLabel(QString("x-displacement node ") + rod->getNode2()->getId() + QString(" [m]:"), this), 7, 0);
    layout->addWidget(u2xLabel, 7, 1);
    layout->addWidget(new QLabel(QString("y-displacement node ") + rod->getNode2()->getId() + QString(" [m]:"), this), 8, 0);
    layout->addWidget(u2yLabel, 8, 1);
    layout->addWidget(new QLabel(QString("z-rotation node ") + rod->getNode2()->getId() + QString(" [rad]:"), this), 9, 0);
    layout->addWidget(u2zLabel, 9, 1);
    connect(okButton, &QPushButton::clicked, this, &EasyChangeDialog::okPressed);
    layout->addWidget(okButton, 0, 2);
    setLayout(layout); // reparents all above created widgets and layouts to this->centralWidget()

    setFixedSize(sizeHint()); // make dialog not resizeable
    move(position); // move to the same position as the last dialog

    // do not call setupEasyChangeDialog() since this dialog should not have a cancel-button
    cancelButton->hide(); // because the cancel-button is created with superclass as parent it is drawn on the dialog; to prevent this, simply hide the button
}

void RodDialog::setE()
{
    rod->setE(EInput->text().toDouble());
}

void RodDialog::setA()
{
    rod->setA(AInput->text().toDouble());
}

void RodDialog::setI()
{
    rod->setI(IInput->text().toDouble());
}

void RodDialog::okPressed(bool)
{
    setE();
    setA();
    setI();
    rod->closeEasyChangeDialog(); // close dialog via element
}

void RodDialog::updateDialog()
{
    rodForceLabel->setText(QString::number(rod->getInnerForce())); // which node does not matter since only the value of the force is used here
    u1xLabel->setText(QString::number(rod->getU(4)));
    u1yLabel->setText(QString::number(rod->getU(0)));
    u1zLabel->setText(QString::number(rod->getU(1)));
    u2xLabel->setText(QString::number(rod->getU(5)));
    u2yLabel->setText(QString::number(rod->getU(2)));
    u2zLabel->setText(QString::number(rod->getU(3)));
    EInput->setText(QString::number(rod->getE()));
    AInput->setText(QString::number(rod->getA()));
    IInput->setText(QString::number(rod->getI()));
}
