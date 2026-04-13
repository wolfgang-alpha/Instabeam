#include "curvedbeamdialog.h"

#include "elements/curvedbeam.h"
#include "elements/node.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

CurvedBeamDialog::CurvedBeamDialog(QWidget *parent, CurvedBeam *elementCurvedBeam) :
    EasyChangeDialog(parent, elementCurvedBeam),
    curvedBeam(elementCurvedBeam),
    rodForceLabel(new QLabel(QString::number(elementCurvedBeam->getInnerForce()), this)),
    arcLengthLabel(new QLabel(QString::number(elementCurvedBeam->getArcLength()), this)),
    openingAngleLabel(new QLabel(QString::number(elementCurvedBeam->getOpeningAngle() * 180.0 / M_PI), this)),
    EInput(new LineEdit(QString::number(elementCurvedBeam->getE()), this)),
    AInput(new LineEdit(QString::number(elementCurvedBeam->getA()), this)),
    IInput(new LineEdit(QString::number(elementCurvedBeam->getI()), this)),
    RInput(new LineEdit(QString::number(elementCurvedBeam->getR()), this))
{
    auto layout = new QGridLayout();
    layout->addWidget(new QLabel("Young's modulus [N/m²]:", this), 0, 0);
    connectLineEdit(EInput, this, &CurvedBeamDialog::setE);
    layout->addWidget(EInput, 0, 1);
    layout->addWidget(new QLabel("Cross-sectional area [m²]:", this), 1, 0);
    connectLineEdit(AInput, this, &CurvedBeamDialog::setA);
    layout->addWidget(AInput, 1, 1);
    layout->addWidget(new QLabel("Second moment of area y [m<sup>4</sup>]:", this), 2, 0);
    connectLineEdit(IInput, this, &CurvedBeamDialog::setI);
    layout->addWidget(IInput, 2, 1);
    layout->addWidget(new QLabel("Radius of curvature [m] (sign = direction):", this), 3, 0);
    connectLineEdit(RInput, this, &CurvedBeamDialog::setR);
    layout->addWidget(RInput, 3, 1);
    layout->addWidget(new QLabel("Arc length [m]:", this), 4, 0);
    layout->addWidget(arcLengthLabel, 4, 1);
    layout->addWidget(new QLabel("Opening angle [°]:", this), 5, 0);
    layout->addWidget(openingAngleLabel, 5, 1);
    layout->addWidget(new QLabel("Normal force magnitude [N]:", this), 6, 0);
    layout->addWidget(rodForceLabel, 6, 1);
    connect(okButton, &QPushButton::clicked, this, &EasyChangeDialog::okPressed);
    layout->addWidget(okButton, 0, 2);
    setLayout(layout);

    setFixedSize(sizeHint());
    move(position);

    cancelButton->hide();
}

void CurvedBeamDialog::setE()
{
    curvedBeam->setE(EInput->text().toDouble());
}

void CurvedBeamDialog::setA()
{
    curvedBeam->setA(AInput->text().toDouble());
}

void CurvedBeamDialog::setI()
{
    curvedBeam->setI(IInput->text().toDouble());
}

void CurvedBeamDialog::setR()
{
    curvedBeam->setR(RInput->text().toDouble());
}

void CurvedBeamDialog::okPressed(bool)
{
    setE();
    setA();
    setI();
    setR();
    curvedBeam->closeEasyChangeDialog();
}

void CurvedBeamDialog::updateDialog()
{
    rodForceLabel->setText(QString::number(curvedBeam->getInnerForce()));
    arcLengthLabel->setText(QString::number(curvedBeam->getArcLength()));
    openingAngleLabel->setText(QString::number(curvedBeam->getOpeningAngle() * 180.0 / M_PI));
    EInput->setText(QString::number(curvedBeam->getE()));
    AInput->setText(QString::number(curvedBeam->getA()));
    IInput->setText(QString::number(curvedBeam->getI()));
    RInput->setText(QString::number(curvedBeam->getR()));
}
