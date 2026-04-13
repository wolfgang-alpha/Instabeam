#ifndef CURVEDBEAMDIALOG_H
#define CURVEDBEAMDIALOG_H

#include "easychangedialog.h"

#include <QLabel>

class CurvedBeam;

class CurvedBeamDialog : public EasyChangeDialog
{
    Q_OBJECT

public:
    CurvedBeamDialog(QWidget *parent, CurvedBeam *elementCurvedBeam);
    ~CurvedBeamDialog() = default;
    CurvedBeamDialog(const CurvedBeamDialog &) = delete;
    CurvedBeamDialog(CurvedBeamDialog &&) = delete;
    CurvedBeamDialog &operator =(const CurvedBeamDialog &) = delete;
    CurvedBeamDialog &operator =(CurvedBeamDialog &&) = delete;

    void setE();
    void setA();
    void setI();
    void setR();

private:
    CurvedBeam *curvedBeam; // weak ptr
    QLabel *rodForceLabel;
    QLabel *arcLengthLabel;
    QLabel *openingAngleLabel;
    LineEdit *EInput;
    LineEdit *AInput;
    LineEdit *IInput;
    LineEdit *RInput;

    // EasyChangeDialog interface
public:
    void okPressed(bool) override;
    void updateDialog() override;
};

#endif // CURVEDBEAMDIALOG_H
