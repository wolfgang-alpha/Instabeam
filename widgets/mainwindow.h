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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

class GraphicsView;

namespace Ui {
class MainWindow;
} // end namespace Ui

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override; // not intended to be inherited
    // it is not necessary to copy/move the mainwindow, therefore the operators are not implemented
    MainWindow(const MainWindow &) = delete;
    MainWindow(MainWindow &&) = delete;
    MainWindow &operator =(const MainWindow &) = delete;
    MainWindow &operator =(MainWindow &&) = delete;

    void toggleNodeAdder(bool checked);
    void toggleRodAdder(bool checked);
    void toggleCurvedBeamAdder(bool checked);
    void toggleBearingAdder(bool checked);
    void toggleForceAdder(bool checked);
    void toggleDimensionAdder(bool checked);
    void toggleLabelAdder(bool checked);

    void forwardKeyPressEvent(QKeyEvent *event) { keyPressEvent(event); } // make it possible to pass key-events to the MainWindow

    GraphicsView *getGraphicsView() const; // returns weak-ptr

    void setStatusBarMessage(const QString &message); // call with empty string to clear the status bar message
    void updateRodColorMinMaxValue(double minValue, double maxValue);

    bool getColorRods() const { return colorRods; }
    bool getMarkZeroLoadingRods() const { return markZeroLoadingRods; }
    bool getShowNodeNumbers() const { return showNodeNumbers; }
    bool getShowRodNumbers() const { return showRodNumbers; }
    bool getDrawDeformedSystem() const { return drawDeformedSystem; }

    void quitAddingElements() const;

private:
    bool saveSystemBeforeContinuing(); // shows the "Would you like to save the content before continuing?"-dialog and returns true if the user wants to continue

    QByteArray createSaveFileContent() const;

    Ui::MainWindow *ui; // deleted in dtor
    QToolBar *toolBar; // has this as parent
    QAction *actionToggleNodeAdder; // has this as parent
    QAction *actionToggleRodAdder; // has this as parent
    QAction *actionToggleCurvedBeamAdder; // has this as parent
    QAction *actionToggleBearingAdder; // has this as parent
    QAction *actionToggleForceAdder; // has this as parent
    QAction *actionToggleDimensionAdder; // has this as parent
    QAction *actionToggleLabelAdder; // has this as parent
    bool colorRods; // indicates if the rods should be colored relative to the size of their rod-force
    bool markZeroLoadingRods; // indicates if zero-loading-rods should be marked
    bool showNodeNumbers;
    bool showRodNumbers;
    bool drawDeformedSystem;
    QString openFilePath;
    QLabel *statusBarMinForceLabel; // gets reparented to this->statusBar()
    QLabel *statusBarMaxForceLabel; // gets reparented to this->statusBar()

private slots:
    void on_action_New_triggered();
    void on_action_Open_triggered();
    bool on_action_Save_triggered();
    bool on_action_SaveAs_triggered();
    void on_action_Print_triggered();
    void on_action_Close_triggered();
    void on_action_About_triggered();
    void on_action_Settings_triggered();
    void on_action_colorRods_triggered(bool checked);
    void on_action_markZeroLoadingRods_triggered(bool checked);
    void on_action_ShowSystemDefinition_triggered();
    void on_action_ShowNodeNumbers_triggered(bool checked);
    void on_action_ShowRodNumbers_triggered(bool checked);
    void on_action_drawDeformedSystem_triggered(bool checked);

    void on_actionDefineEAIglobal_triggered();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // MAINWINDOW_H
