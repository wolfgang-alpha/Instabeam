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

#include "seteaiglobaldialog.h"
#include "ui_seteaiglobaldialog.h"

#include "utilities.h"
#include "elements/rod.h"

SetEAIGlobalDialog::SetEAIGlobalDialog(GraphicsScene *graphicsScene, QMainWindow *parent) :
    QDialog(parent, Qt::WindowTitleHint),
    ui(new Ui::SetEAIGlobalDialog),
    scene(graphicsScene)
{
    ui->setupUi(this);
    setWindowTitle("Werteeingabe");
}

SetEAIGlobalDialog::~SetEAIGlobalDialog()
{
    delete ui;
}

void SetEAIGlobalDialog::on_okButton_clicked()
{
    for (auto rod : Utilities::getAllElementsOfType<Rod *>(scene->items())) { // save the entered values if the checkbox corresponding the variable is true
        if (ui->EBox->isChecked()) {
            rod->setE(ui->EInput->text().toDouble());
        }
        if (ui->ABox->isChecked()) {
            rod->setA(ui->AInput->text().toDouble());
        }
        if (ui->IBox->isChecked()) {
            rod->setI(ui->IInput->text().toDouble());
        }
    }
    close();
}

void SetEAIGlobalDialog::on_cancelButton_clicked()
{
    close();
}
