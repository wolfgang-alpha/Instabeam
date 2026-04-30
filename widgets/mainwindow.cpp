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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "graphicsscene.h"
#include "widgets/settings.h"
#include "widgets/systemdefinitiondialog.h"
#include "utilities.h"
#include "elements/node.h"
#include "elements/label.h"
#include "elements/rod.h"
#include "jsonkeys.h"
#include "widgets/seteaiglobaldialog.h"

#include <QMouseEvent>
#include <QDebug>
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    toolBar(nullptr),
    actionToggleNodeAdder(nullptr),
    actionToggleRodAdder(nullptr),
    actionToggleCurvedBeamAdder(nullptr),
    actionToggleBearingAdder(nullptr),
    actionToggleForceAdder(nullptr),
    actionToggleDimensionAdder(nullptr),
    actionToggleLabelAdder(nullptr),
    colorRods(false),
    markZeroLoadingRods(false),
    showRodNumbers(false),
    drawDeformedSystem(false),
    openFilePath(""),
    statusBarMinForceLabel(new QLabel("0 N")), // gets reparented later
    statusBarMaxForceLabel(new QLabel("0 N")) // gets reparented later
{
    ui->setupUi(this);
    auto graphicsScene = new GraphicsScene(this); // gets deleted when this is dtored

    setWindowTitle("Instabeam v2.0");
    setWindowIcon(QIcon(":/windowtitleicon.png"));
    showMaximized();

    // setup status-bar
    setStatusBar(new QStatusBar(this)); // gets deleted when this is destroyed
    statusBar()->addPermanentWidget(statusBarMinForceLabel); // the label gets reparented to the status-bar
    QLabel *colorIcon = new QLabel();
    colorIcon->setPixmap(QPixmap(":/rodcolorscale.png"));
    statusBar()->addPermanentWidget(colorIcon); // colorIcon gets reparented to the status-bar
    statusBar()->addPermanentWidget(statusBarMaxForceLabel); // the label gets reparented to the status-bar

    // setup tool-bar
    toolBar = new QToolBar(this); // gets deleted when this is dtor-ed
    toolBar->setContextMenuPolicy(Qt::PreventContextMenu); // suppress the creation of context-menus
    toolBar->setIconSize(QSize(64, 64));

    actionToggleNodeAdder = new QAction(this); // parent is this
    actionToggleNodeAdder->setIcon(QIcon(":/nodeicon.png"));
    actionToggleNodeAdder->setToolTip("Add nodes");
    actionToggleNodeAdder->setCheckable(true);
    connect(actionToggleNodeAdder, &QAction::toggled, this, &MainWindow::toggleNodeAdder);
    toolBar->addAction(actionToggleNodeAdder);

    actionToggleRodAdder = new QAction(this); // parent is this
    actionToggleRodAdder->setIcon(QIcon(":/rodicon.png"));
    actionToggleRodAdder->setToolTip("Add rods");
    actionToggleRodAdder->setCheckable(true);
    connect(actionToggleRodAdder, &QAction::toggled, this, &MainWindow::toggleRodAdder);
    toolBar->addAction(actionToggleRodAdder);

    actionToggleCurvedBeamAdder = new QAction(this); // parent is this
    actionToggleCurvedBeamAdder->setIcon(QIcon(":/curvedbeamicon.png"));
    actionToggleCurvedBeamAdder->setToolTip("Add curved beams");
    actionToggleCurvedBeamAdder->setCheckable(true);
    connect(actionToggleCurvedBeamAdder, &QAction::toggled, this, &MainWindow::toggleCurvedBeamAdder);
    toolBar->addAction(actionToggleCurvedBeamAdder);

    actionToggleBearingAdder = new QAction(this); // parent is this
    actionToggleBearingAdder->setIcon(QIcon(":/bearingicon.png"));
    actionToggleBearingAdder->setToolTip("Add bearings");
    actionToggleBearingAdder->setCheckable(true);
    connect(actionToggleBearingAdder, &QAction::toggled, this, &MainWindow::toggleBearingAdder);
    toolBar->addAction(actionToggleBearingAdder);

    actionToggleForceAdder = new QAction(this); // parent is this
    actionToggleForceAdder->setIcon(QIcon(":/singleforceicon.png"));
    actionToggleForceAdder->setToolTip("Add forces");
    actionToggleForceAdder->setCheckable(true);
    connect(actionToggleForceAdder, &QAction::toggled, this, &MainWindow::toggleForceAdder);
    toolBar->addAction(actionToggleForceAdder);

    actionToggleDimensionAdder = new QAction(this); // parent is this
    actionToggleDimensionAdder->setIcon(QIcon(":/dimensionicon.png"));
    actionToggleDimensionAdder->setToolTip("Add dimensions");
    actionToggleDimensionAdder->setCheckable(true);
    connect(actionToggleDimensionAdder, &QAction::toggled, this, &MainWindow::toggleDimensionAdder);
    toolBar->addAction(actionToggleDimensionAdder);

    actionToggleLabelAdder = new QAction(this); // parent is this
    actionToggleLabelAdder->setIcon(QIcon(":/labelicon.png"));
    actionToggleLabelAdder->setToolTip("Add labels");
    actionToggleLabelAdder->setCheckable(true);
    connect(actionToggleLabelAdder, &QAction::toggled, this, &MainWindow::toggleLabelAdder);
    toolBar->addAction(actionToggleLabelAdder);

    addToolBar(Qt::LeftToolBarArea, toolBar);

    ui->graphicsView->setScene(graphicsScene); // view does not take ownership of scene
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::toggleNodeAdder(bool checked)
{
    if (checked) {
        if (actionToggleRodAdder->isChecked()) {
            actionToggleRodAdder->setChecked(false);
        } else if (actionToggleCurvedBeamAdder->isChecked()) {
            actionToggleCurvedBeamAdder->setChecked(false);
        } else if (actionToggleBearingAdder->isChecked()) {
            actionToggleBearingAdder->setChecked(false);
        } else if (actionToggleForceAdder->isChecked()) {
            actionToggleForceAdder->setChecked(false);
        } else if (actionToggleDimensionAdder->isChecked()) {
            actionToggleDimensionAdder->setChecked(false);
        } else if (actionToggleLabelAdder->isChecked()) {
            actionToggleLabelAdder->setChecked(false);
        }
    }
    ui->graphicsView->toggleNodeAdder(checked);
}

void MainWindow::toggleRodAdder(bool checked)
{
    if (checked) {
        if (actionToggleNodeAdder->isChecked()) {
            actionToggleNodeAdder->setChecked(false);
        } else if (actionToggleCurvedBeamAdder->isChecked()) {
            actionToggleCurvedBeamAdder->setChecked(false);
        } else if (actionToggleBearingAdder->isChecked()) {
            actionToggleBearingAdder->setChecked(false);
        } else if (actionToggleForceAdder->isChecked()) {
            actionToggleForceAdder->setChecked(false);
        } else if (actionToggleDimensionAdder->isChecked()) {
            actionToggleDimensionAdder->setChecked(false);
        } else if (actionToggleLabelAdder->isChecked()) {
            actionToggleLabelAdder->setChecked(false);
        }
    }
    ui->graphicsView->toggleRodAdder(checked);
}

void MainWindow::toggleCurvedBeamAdder(bool checked)
{
    if (checked) {
        if (actionToggleNodeAdder->isChecked()) {
            actionToggleNodeAdder->setChecked(false);
        } else if (actionToggleRodAdder->isChecked()) {
            actionToggleRodAdder->setChecked(false);
        } else if (actionToggleBearingAdder->isChecked()) {
            actionToggleBearingAdder->setChecked(false);
        } else if (actionToggleForceAdder->isChecked()) {
            actionToggleForceAdder->setChecked(false);
        } else if (actionToggleDimensionAdder->isChecked()) {
            actionToggleDimensionAdder->setChecked(false);
        } else if (actionToggleLabelAdder->isChecked()) {
            actionToggleLabelAdder->setChecked(false);
        }
    }
    ui->graphicsView->toggleCurvedBeamAdder(checked);
}

void MainWindow::toggleBearingAdder(bool checked)
{
    if (checked) {
        if (actionToggleNodeAdder->isChecked()) {
            actionToggleNodeAdder->setChecked(false);
        } else if (actionToggleRodAdder->isChecked()) {
            actionToggleRodAdder->setChecked(false);
        } else if (actionToggleCurvedBeamAdder->isChecked()) {
            actionToggleCurvedBeamAdder->setChecked(false);
        } else if (actionToggleForceAdder->isChecked()) {
            actionToggleForceAdder->setChecked(false);
        } else if (actionToggleDimensionAdder->isChecked()) {
            actionToggleDimensionAdder->setChecked(false);
        } else if (actionToggleLabelAdder->isChecked()) {
            actionToggleLabelAdder->setChecked(false);
        }
    }
    ui->graphicsView->toggleBearingAdder(checked);
}

void MainWindow::toggleForceAdder(bool checked)
{
    if (checked) {
        if (actionToggleNodeAdder->isChecked()) {
            actionToggleNodeAdder->setChecked(false);
        } else if (actionToggleRodAdder->isChecked()) {
            actionToggleRodAdder->setChecked(false);
        } else if (actionToggleCurvedBeamAdder->isChecked()) {
            actionToggleCurvedBeamAdder->setChecked(false);
        } else if (actionToggleBearingAdder->isChecked()) {
            actionToggleBearingAdder->setChecked(false);
        } else if (actionToggleDimensionAdder->isChecked()) {
            actionToggleDimensionAdder->setChecked(false);
        } else if (actionToggleLabelAdder->isChecked()) {
            actionToggleLabelAdder->setChecked(false);
        }
    }
    ui->graphicsView->toggleForceAdder(checked);
}

void MainWindow::toggleDimensionAdder(bool checked)
{
    if (checked) {
        if (actionToggleNodeAdder->isChecked()) {
            actionToggleNodeAdder->setChecked(false);
        } else if (actionToggleRodAdder->isChecked()) {
            actionToggleRodAdder->setChecked(false);
        } else if (actionToggleCurvedBeamAdder->isChecked()) {
            actionToggleCurvedBeamAdder->setChecked(false);
        } else if (actionToggleBearingAdder->isChecked()) {
            actionToggleBearingAdder->setChecked(false);
        } else if (actionToggleForceAdder->isChecked()) {
            actionToggleForceAdder->setChecked(false);
        } else if (actionToggleLabelAdder->isChecked()) {
            actionToggleLabelAdder->setChecked(false);
        }
    }
    ui->graphicsView->toggleDimensionAdder(checked);
}

void MainWindow::toggleLabelAdder(bool checked)
{
    if (checked) {
        if (actionToggleNodeAdder->isChecked()) {
            actionToggleNodeAdder->setChecked(false);
        } else if (actionToggleRodAdder->isChecked()) {
            actionToggleRodAdder->setChecked(false);
        } else if (actionToggleCurvedBeamAdder->isChecked()) {
            actionToggleCurvedBeamAdder->setChecked(false);
        } else if (actionToggleBearingAdder->isChecked()) {
            actionToggleBearingAdder->setChecked(false);
        } else if (actionToggleForceAdder->isChecked()) {
            actionToggleForceAdder->setChecked(false);
        } else if (actionToggleDimensionAdder->isChecked()) {
            actionToggleDimensionAdder->setChecked(false);
        }
    }
    ui->graphicsView->toggleLabelAdder(checked);
}

GraphicsView *MainWindow::getGraphicsView() const
{
    return ui->graphicsView;
}

void MainWindow::setStatusBarMessage(const QString &message)
{
    statusBar()->showMessage(message);
}

void MainWindow::updateRodColorMinMaxValue(double minValue, double maxValue)
{
    statusBarMinForceLabel->setText(QString::number(minValue) + " N");
    statusBarMaxForceLabel->setText(QString::number(maxValue) + " N");
}

void MainWindow::quitAddingElements() const
{
    if (actionToggleNodeAdder->isChecked()) {
        actionToggleNodeAdder->setChecked(false);
    } else if (actionToggleRodAdder->isChecked()) {
        actionToggleRodAdder->setChecked(false);
    } else if (actionToggleCurvedBeamAdder->isChecked()) {
        actionToggleCurvedBeamAdder->setChecked(false);
    } else if (actionToggleBearingAdder->isChecked()) {
        actionToggleBearingAdder->setChecked(false);
    } else if (actionToggleForceAdder->isChecked()) {
        actionToggleForceAdder->setChecked(false);
    } else if (actionToggleDimensionAdder->isChecked()) {
        actionToggleDimensionAdder->setChecked(false);
    } else if (actionToggleLabelAdder->isChecked()) {
        actionToggleLabelAdder->setChecked(false);
    }
}

bool MainWindow::saveSystemBeforeContinuing() // saves the system if wanted and returns true if the user wants to continue
{
    if (ui->graphicsView->scene()->items().isEmpty()) { // if there are no items on the scene, do not offer saving
        return true;
    }
    // there are items, offer saving them
    QMessageBox box(this);
    box.setWindowTitle("Warning:");
    box.setText("If you continue, all unsaved changes will be lost!\nWould you like to save them now?");
    box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    box.setButtonText(QMessageBox::Save, "Save");
    box.setButtonText(QMessageBox::Discard, "Don't save");
    box.setButtonText(QMessageBox::Cancel, "Cancel");
    switch (box.exec()) {
    case QMessageBox::Save:
        on_action_Save_triggered(); // save content
        return true;
    case QMessageBox::Discard:
        return true;
    case QMessageBox::Cancel:
        return false;
    default: // default case should never be entered
        return false;
    }
}

QByteArray MainWindow::createSaveFileContent() const // create a json-representation of the system and return it
{
    ui->graphicsView->closeEasyChangeDialog(); // close the dialog such that every element has its default color
    quitAddingElements(); // to not save half-added elements
    QJsonDocument document; // create json-document
    QJsonObject root; // create content-wrapper
    root.insert(JsonKeys::version, "1.0"); // add version
    root.insert(JsonKeys::graphicsView, ui->graphicsView->saveAsJson()); // add graphicsView
    document.setObject(root); // add object to document
    return document.toJson(QJsonDocument::Indented); // return document
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QMainWindow::mousePressEvent(event);
}

void MainWindow::on_action_New_triggered() // create new file
{
    // ask the user if the file should be saved before creating a new one
    if (!saveSystemBeforeContinuing()) { // if the user does not want to continue, abort
        return;
    }
    // close eventually active adders
    quitAddingElements();
    // delete model
    delete ui->graphicsView->getGraphicsScene();
    // create new model
    auto graphicsScene = new GraphicsScene(this); // parent is this
    // display the scene
    ui->graphicsView->setScene(graphicsScene);
    // clear the openFilePath
    openFilePath = "";
}

void MainWindow::on_action_Open_triggered() // open file
{
    on_action_New_triggered(); // offer user to save the current file, delete the scene and create a new (empty) one
    // find the file to open
    QString filePath = QFileDialog::getOpenFileName(this, "Open:", QDir::rootPath(), "Instabeam file (*.json)");
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) { // try to open the file, if unsuccessfull abort
        QMessageBox::critical(this, "Error:", "The selected file could not be opened!");
        return;
    }
    // populate the scene with new elements
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) { // an error occured
        QMessageBox::critical(this, "Error:", "The file contains syntax errors and could not be read correctly!");
        return;
    }
    QJsonObject root = document.object(); // get root element
    QString version = root.value(JsonKeys::version).toString();
    if (version == "1.0") { // version is supported
        ui->graphicsView->loadFromJson(root.value(JsonKeys::graphicsView).toObject(), this); // let the graphicsView load the other values
    } else { // version is not supported, error
        QMessageBox::critical(this, "Error:", "The file format version is not supported by this version of Instabeam!");
        return;
    }

    // trigger updates to display the newly opened system with the current display-options
    ui->action_markZeroLoadingRods->triggered(ui->action_markZeroLoadingRods->isChecked());
    ui->action_colorRods->triggered(ui->action_colorRods->isChecked());
    ui->action_ShowNodeNumbers->triggered(ui->action_ShowNodeNumbers->isChecked());
    ui->action_ShowRodNumbers->triggered(ui->action_ShowRodNumbers->isChecked());

    // save file-path for later usage
    openFilePath = filePath;
    file.close();
}

bool MainWindow::on_action_Save_triggered() // save to current file
{
    if (openFilePath.isEmpty()) { // if no file is open, create a new one
        return on_action_SaveAs_triggered();
    }
    QSaveFile file(openFilePath);
    if (!file.open(QSaveFile::WriteOnly)) {
        QMessageBox::critical(this, "Error:", "Could not create a file to save the system!");
        return false;
    }
    file.write(createSaveFileContent()); // overwrite content of existing file (or fill empty one)
    if (QFileInfo::exists(openFilePath)) { // if the file existed previously, resize it to the new content
        file.resize(file.pos());
    }
    file.commit(); // close file, flush buffer and write content to disk
    return true;
}

bool MainWindow::on_action_SaveAs_triggered() // save as new file
{
    QString selectedFilter; // allocate memory for the selected filter which gets set in the next line
    QString filePath = QFileDialog::getSaveFileName(this, "Save as:", QDir::homePath(), "Instabeam file (*.json);;Image (*.png)", &selectedFilter);
    if (filePath.isEmpty()) { // user pressed cancel
        return false;
    }
    QSaveFile file(filePath);
    if (!file.open(QSaveFile::WriteOnly)) {
        QMessageBox::critical(this, "Error:", "Could not create a file to save the system!");
        return false;
    }
    if (selectedFilter == "Instabeam file (*.json)") { // save system as json-file
        file.write(createSaveFileContent()); // write content to file
        if (QFileInfo::exists(filePath)) { // resize the file if the user selected a file that already existed
            file.resize(file.pos());
        }
        openFilePath = filePath; // save filepath for later usage
    } else if (selectedFilter == "Image (*.png)") { // save system as png-file
        ui->graphicsView->closeEasyChangeDialog(); // otherwise an element could be selected and therefore have a different color
        quitAddingElements(); // to not add half-added elements to the image
        QImage image(ui->graphicsView->scene()->sceneRect().size().toSize(), QImage::Format_ARGB32); // create image
        image.fill(Qt::white); // fill background
        QPainter painter(&image); // setup painter to draw onto the image
        painter.setRenderHint(QPainter::Antialiasing); // set painter-properties
        ui->graphicsView->scene()->render(&painter, image.rect(), ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio); // draw the scene-rect onto the image
        image.save(&file, "PNG"); // save the image as png
        // do not save the file-path-variable because every time an image is saved open the save-as dialog
    } else { // this case can never happen normally
        qFatal("error 1 in MainWindow::on_action_SaveAs_triggered(): filter not recognized");
    }
    file.commit(); // close file, flush buffer and write content to disk
    return true;
}

void MainWindow::on_action_Print_triggered()
{
    QPrinter printer(QPrinter::ScreenResolution); // create printer with the specified resolution
    QPrintDialog dialog(&printer, this); // create print-dialog
    if (dialog.exec() == QPrintDialog::Accepted) { // start printing process if the user clicks "ok"
        ui->graphicsView->closeEasyChangeDialog(); // close dialog to print every element in its default state (no hovering, etc.)
        QPainter painter(&printer); // create painter to print on printer
        painter.setRenderHint(QPainter::Antialiasing); // setup painter
        ui->graphicsView->scene()->render(&painter, printer.pageRect(), ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio); // render scene fullscreen on painter
    }
}

void MainWindow::on_action_Close_triggered() // close app
{
    close();
}

void MainWindow::on_action_About_triggered() // show about-dialog
{
    QMessageBox::about(this, "About Instabeam", "Instabeam is an application for quickly and easily drawing structural systems and computing deformations and internal forces using the Finite Element Method.\n"
                       "\nWarning: Although Instabeam has been developed with the utmost care, no guarantee is given for the correctness of the results "
                       "and no liability is accepted for damages resulting from the use of Instabeam's results!\n\n"
                       "Developers:\n"
                       "  Bernhard Adami (original TrussCalculator)\n"
                       "  Wolfgang Flachberger (element stiffness matrices, curved beams)\n\n"
                       "Copyright (C) 2020: Institute of Mechanics, Montanuniversitaet Leoben\n\nVersion: 2.1");
}

void MainWindow::on_action_Settings_triggered() // show settings-dialog
{
    Settings settings(this); // the settings-dialog must be destroyed immediately after returning because it holds a weak ptr to this and does not check any objects for nullptr
    settings.exec();
} // settings dtor called upon exiting this code-block

void MainWindow::closeEvent(QCloseEvent *event)
{
//#ifdef QT_DEBUG // remove the annoying "Would you really like to quit?" message when closing the mainwindow while developing
    event->accept();
//#else
//    // ask if the content should be saved
//    auto msgBox = std::make_unique<QMessageBox>(this);
//    msgBox->setWindowTitle("Warning:");
//    msgBox->setText("Would you like to save the changes to the current system?");
//    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
//    msgBox->setDefaultButton(QMessageBox::Yes);
//    msgBox->setButtonText(QMessageBox::Yes, "Yes");
//    msgBox->setButtonText(QMessageBox::No, "No");
//    msgBox->setButtonText(QMessageBox::Cancel, "Cancel");
//    switch(msgBox->exec()){
//    case QMessageBox::Yes:
//        if (on_action_Save_triggered()) // if saving was not successful, keep the app running, otherwise close it
//            event->accept();
//        else
//            event->ignore();
//        break;
//    case QMessageBox::No:
//        event->accept();
//        break;
//    case QMessageBox::Cancel:
//        event->ignore();
//        break;
//    default:
//        event->ignore();
//        break;
//    }
//#endif
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape: // if Esc is pressed, close all adders and the easychange-dialog
        quitAddingElements();
        static_cast<GraphicsScene *>(ui->graphicsView->scene())->closeEasyChangeDialog();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        return;
    }
    event->accept();
}

void MainWindow::on_action_colorRods_triggered(bool checked)
{
    colorRods = checked;
    ui->graphicsView->scene()->update(ui->graphicsView->scene()->itemsBoundingRect()); // update the scene to properly color the rods
}

void MainWindow::on_action_markZeroLoadingRods_triggered(bool checked)
{
    markZeroLoadingRods = checked;
    ui->graphicsView->scene()->update(ui->graphicsView->scene()->itemsBoundingRect()); // update scene after changing color-property
}

void MainWindow::on_action_ShowSystemDefinition_triggered()
{
    quitAddingElements(); // close all adders before creating the dialog (otherwise temporary elements would be displayed)
    SystemDefinitionDialog d(static_cast<GraphicsScene *>(ui->graphicsView->scene()), this);
    d.exec();
}

void MainWindow::on_action_ShowNodeNumbers_triggered(bool checked)
{
    auto nodes = Utilities::getAllElementsOfType<Node *>(ui->graphicsView->scene()->items());
    for (auto node : nodes) { // hide/show all node-labels
        node->getLabel()->setVisible(checked);
        if (!checked) {
            node->closeEasyChangeDialog(); // close eventually open easychange-dialogs if the label is not visible
        }
    }
}

void MainWindow::on_action_ShowRodNumbers_triggered(bool checked)
{
    auto rods = Utilities::getAllElementsOfType<Rod *>(ui->graphicsView->scene()->items());
    for (auto rod : rods) {
        rod->getLabel()->setVisible(checked);
        if (!checked) {
            rod->closeEasyChangeDialog();
        }
    }
}

void MainWindow::on_action_drawDeformedSystem_triggered(bool checked)
{
    drawDeformedSystem = checked;
    ui->graphicsView->scene()->update(ui->graphicsView->scene()->itemsBoundingRect()); // update the scene to draw the deformations
}

void MainWindow::on_actionDefineEAIglobal_triggered()
{
    quitAddingElements();
    SetEAIGlobalDialog d(static_cast<GraphicsScene *>(ui->graphicsView->scene()), this); // open dialog to change E, A, I
    d.exec();
}
