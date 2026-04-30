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

#ifndef SETEAIGLOBALDIALOG_H
#define SETEAIGLOBALDIALOG_H

#include "graphicsscene.h"

#include <QDialog>
#include <QMainWindow>

namespace Ui {
class SetEAIGlobalDialog;
}

class SetEAIGlobalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetEAIGlobalDialog(GraphicsScene *graphicsScene, QMainWindow *parent); // *scene is a weak-ptr
    virtual ~SetEAIGlobalDialog();

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::SetEAIGlobalDialog *ui;
    GraphicsScene *scene; // weak ptr
};

#endif // SETEAIGLOBALDIALOG_H
