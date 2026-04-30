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

#ifndef EASYCHANGEDIALOG_H
#define EASYCHANGEDIALOG_H

#include "utilities.h"

#include <QDialog>
#include <QHBoxLayout>

class TrussElement;
class LineEdit;

class EasyChangeDialog : public QDialog // base class of all easychange-dialogs
{
    Q_OBJECT

public:
    EasyChangeDialog(QWidget *parent, TrussElement *easyChangeUnderlyingElement);
    // prevent copying/moving of the dialog since it is not needed so there is no reason to implement the necessary fcts
    ~EasyChangeDialog() override = default;
    EasyChangeDialog(const EasyChangeDialog &) = delete;
    EasyChangeDialog(EasyChangeDialog &&) = delete;
    EasyChangeDialog &operator =(const EasyChangeDialog &) = delete;
    EasyChangeDialog &operator =(EasyChangeDialog &&) = delete;

    void setupEasyChangeDialog(QHBoxLayout *hBoxLayout); // call this fct in every child's ctor somewhere after creating the layout (this fct takes care of setting the layout)

    virtual void okPressed(bool) = 0; // slot activated when ok is pressed, used for saving the values of the input-boxes
    void cancelPressed(bool); // slot activated when cancel is pressed
    void addNodePermanentlyToScene(); // called by the node-dialog to add the current node when the user presses the ok-button of the dialog
    void addLabelPermanentlyToScene(); // called by the label-dialog to ... (see above)

    virtual void updateDialog() = 0; // slot called when the values of the input-boxes changed

    TrussElement *getUnderlyingElement() const { return underlyingElement; }

protected:
    template<typename T>
    void connectLineEdit(LineEdit *lineEdit, T* receivingElement, void (T::*slot)()) // this fct is implemented to reduce writing in subclasses
    {
        Utilities::connectLineEditToDoneSignal(lineEdit, receivingElement, slot); // forward call to Utilities-fct
    }

protected:
    bool isMoving;
    QPoint offset;
    QPushButton *okButton; // parent is this->centralWidget()
    QPushButton *cancelButton; // parent is this->centralWidget()
    static QPoint position;
    TrussElement *underlyingElement; // weak ptr

    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#endif // EASYCHANGEDIALOG_H
