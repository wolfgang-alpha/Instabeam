#include "systemdefinitiondialog.h"
#include "ui_systemdefinitiondialog.h"

#include "graphicsscene.h"
#include "utilities.h"
#include "elements/node.h"
#include "elements/rod.h"
#include "elements/bearing.h"
#include "elements/singleforce.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>

SystemDefinitionDialog::SystemDefinitionDialog(GraphicsScene *graphicsScene, QWidget *parent) :
    QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::SystemDefinitionDialog)
{
    ui->setupUi(this);

    setWindowTitle("System definition");

    ui->tabWidget->setTabText(0, "Nodes");
    setupTable<Node>(ui->nodeView, graphicsScene->items(), QStringList{"ID", "Node type", "x-position [m]", "y-position [m]", "x-displacement [m]", "y-displacement [m]"});
    ui->tabWidget->setTabText(1, "Rods");
    setupTable<Rod>(ui->rodView, graphicsScene->items(), QStringList{"ID", "Node 1 ID", "Node 2 ID", "E [N/m²]", "A [m²]", "Iy [m^4]", "Normal force [N]", "x-displacement 1 [m]", "y-displacement 1 [m]", "z-rotation 1 [rad]", "x-displacement 2 [m]", "y-displacement 2 [m]", "z-rotation 2 [rad]"});
    ui->tabWidget->setTabText(2, "Bearings");
    setupTable<Bearing>(ui->bearingView, graphicsScene->items(), QStringList{"ID", "Node ID", "Bearing type", "Angle [°]", "Reaction force x [N]", "Reaction force y [N]", "Reaction moment z [Nm]"});
    ui->tabWidget->setTabText(3, "Forces");
    setupTable<SingleForce>(ui->forceView, graphicsScene->items(), QStringList{"ID", "Node ID", "Value [N]", "Angle [°]"});

    on_tabWidget_currentChanged(0); // resize the dialog
    move(100, 100); // position dialog
}

SystemDefinitionDialog::~SystemDefinitionDialog()
{
    delete ui;
}

void SystemDefinitionDialog::populateModel(QStandardItemModel *model, QList<Node *> list)
{
    for (int i = 0; i < list.length(); i++) {
        model->setItem(i, 0, new QStandardItem(list.at(i)->getId())); // model takes ownership of item
        model->setItem(i, 1, new QStandardItem(list.at(i)->getNodeType() == NodeType::GerberJoint ? "Gerber joint" : "Weld"));
        model->setItem(i, 2, new QStandardItem(QString::number(list.at(i)->x() / static_cast<GraphicsScene *>(list.at(i)->scene())->getScaleValue())));
        model->setItem(i, 3, new QStandardItem(QString::number(- list.at(i)->y() / static_cast<GraphicsScene *>(list.at(i)->scene())->getScaleValue()))); // flip y-axis
        model->setItem(i, 4, new QStandardItem(QString::number(list.at(i)->getUx())));
        model->setItem(i, 5, new QStandardItem(QString::number(list.at(i)->getUy())));
    }
}

void SystemDefinitionDialog::populateModel(QStandardItemModel *model, QList<Rod *> list)
{
    for (int i = 0; i < list.length(); i++) {
        model->setItem(i, 0, new QStandardItem(list.at(i)->getId()));
        model->setItem(i, 1, new QStandardItem(list.at(i)->getNode1()->getId()));
        model->setItem(i, 2, new QStandardItem(list.at(i)->getNode2()->getId()));
        model->setItem(i, 3, new QStandardItem(QString::number(list.at(i)->getE())));
        model->setItem(i, 4, new QStandardItem(QString::number(list.at(i)->getA())));
        model->setItem(i, 5, new QStandardItem(QString::number(list.at(i)->getI())));
        model->setItem(i, 6, new QStandardItem(QString::number(list.at(i)->getInnerForce())));
        model->setItem(i, 7, new QStandardItem(QString::number(list.at(i)->getU(4))));
        model->setItem(i, 8, new QStandardItem(QString::number(list.at(i)->getU(0))));
        model->setItem(i, 9, new QStandardItem(QString::number(list.at(i)->getU(1))));
        model->setItem(i, 10, new QStandardItem(QString::number(list.at(i)->getU(5))));
        model->setItem(i, 11, new QStandardItem(QString::number(list.at(i)->getU(2))));
        model->setItem(i, 12, new QStandardItem(QString::number(list.at(i)->getU(3))));
    }
}

void SystemDefinitionDialog::populateModel(QStandardItemModel *model, QList<Bearing *> list)
{
    for (int i = 0; i < list.length(); i++) {
        model->setItem(i, 0, new QStandardItem(list.at(i)->getId()));
        model->setItem(i, 1, new QStandardItem(static_cast<Node *>(list.at(i)->parentItem())->getId()));
        QString s;
        if (list.at(i)->getBearingType() == BearingType::LocatingBearing) {
            s = "Locating bearing";
        } else if (list.at(i)->getBearingType() == BearingType::FloatingBearing) {
            s = "Floating bearing";
        } else {
            s = "Fixed clamping";
        }
        model->setItem(i, 2, new QStandardItem(s));
        model->setItem(i, 3, new QStandardItem(QString::number(list.at(i)->getAngle())));
        model->setItem(i, 4, new QStandardItem(QString::number(list.at(i)->getReactionForceX())));
        model->setItem(i, 5, new QStandardItem(QString::number(list.at(i)->getReactionForceY())));
        model->setItem(i, 6, new QStandardItem(QString::number(list.at(i)->getReactionMomentZ())));
    }
}

void SystemDefinitionDialog::populateModel(QStandardItemModel *model, QList<SingleForce *> list)
{
    for (int i = 0; i < list.length(); i++) {
        model->setItem(i, 0, new QStandardItem(list.at(i)->getId()));
        model->setItem(i, 1, new QStandardItem(static_cast<Node *>(list.at(i)->parentItem())->getId()));
        model->setItem(i, 2, new QStandardItem(QString::number(list.at(i)->getValue())));
        model->setItem(i, 3, new QStandardItem(QString::number(list.at(i)->getAngle() * 180 / M_PI - 180)));
    }
}

template<typename T>
void SystemDefinitionDialog::setupTable(QTableView *view, const QList<QGraphicsItem *> &items, const QStringList &horizontalHeaderLabels)
{
    view->horizontalHeader()->setStretchLastSection(false);
    view->verticalHeader()->hide();
    QList<T *> elementList = Utilities::sortListById(Utilities::getAllElementsOfType<T *>(items));
    auto model = new QStandardItemModel(elementList.length(), horizontalHeaderLabels.length(), this); // parent is this
    model->setHorizontalHeaderLabels(horizontalHeaderLabels);
    populateModel(model, elementList);
    view->setModel(model);
    view->setEditTriggers(QTableView::NoEditTriggers);
    view->resizeColumnsToContents();
    int width = 2 * view->frameWidth(); // calculate the with the view has to be to exactly fit the model
    for (int col = 0; col < view->model()->columnCount(); col++) {
        width += view->columnWidth(col);
    }
    view->setMinimumWidth(width); // set size of view
    view->setMaximumWidth(width);
}

void SystemDefinitionDialog::on_tabWidget_currentChanged(int index)
{
    ui->tabWidget->widget(index)->adjustSize(); // resize tab-page to fit the content
    int width = ui->tabWidget->widget(index)->width(); // get new width of page
    setMinimumWidth(width + 30); // set the width of the dialog to fit the tab-widget and its borders
    setMaximumWidth(width + 30);
}

void SystemDefinitionDialog::on_okButton_clicked()
{
    close(); // close dialog on ok-press
}
