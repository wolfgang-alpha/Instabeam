#-------------------------------------------------
#
# Project created by QtCreator 2018-06-05T14:56:37
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TrussCalculator
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS _USE_MATH_DEFINES

CONFIG += c++14 \
        object_parallel_to_source

CONFIG(release): CONFIG += -O3

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    elements/rope.cpp \
    elements/curvedbeam.cpp \
        main.cpp \
    pen.cpp \
    color.cpp \
    utilities.cpp \
    graphicsscene.cpp \
    calculator.cpp \
    widgets/mainwindow.cpp \
    widgets/graphicsview.cpp \
    widgets/lineedit.cpp \
    widgets/seteaiglobaldialog.cpp \
    widgets/settings.cpp \
    widgets/elementselectiondialog.cpp \
    widgets/easychange/easychangedialog.cpp \
    widgets/easychange/nodedialog.cpp \
    widgets/easychange/roddialog.cpp \
    widgets/easychange/curvedbeamdialog.cpp \
    widgets/easychange/bearingdialog.cpp \
    widgets/easychange/singleforcedialog.cpp \
    elements/node.cpp \
    elements/rod.cpp \
    elements/trusselement.cpp \
    elements/bearing.cpp \
    elements/singleforce.cpp \
    factories/nodeadder.cpp \
    factories/rodadder.cpp \
    factories/curvedbeamadder.cpp \
    factories/bearingadder.cpp \
    factories/forceadder.cpp \
    widgets/systemdefinitiondialog.cpp \
    elements/label.cpp \
    widgets/easychange/labeldialog.cpp \
    factories/dimensionadder.cpp \
    elements/dimension.cpp \
    widgets/easychange/dimensiondialog.cpp \
    factories/labeladder.cpp

HEADERS += \
        color.h \
    elements/rope.h \
    elements/curvedbeam.h \
    pen.h \
    utilities.h \
    graphicsscene.h \
    calculator.h \
    widgets/mainwindow.h \
    widgets/graphicsview.h \
    widgets/lineedit.h \
    widgets/seteaiglobaldialog.h \
    widgets/settings.h \
    widgets/elementselectiondialog.h \
    widgets/easychange/easychangedialog.h \
    widgets/easychange/nodedialog.h \
    widgets/easychange/roddialog.h \
    widgets/easychange/curvedbeamdialog.h \
    widgets/easychange/bearingdialog.h \
    widgets/easychange/singleforcedialog.h \
    elements/node.h \
    elements/rod.h \
    elements/trusselement.h \
    elements/bearing.h \
    elements/singleforce.h \
    factories/nodeadder.h \
    factories/rodadder.h \
    factories/curvedbeamadder.h \
    factories/bearingadder.h \
    factories/forceadder.h \
    libs/Eigen/Eigen/Eigen \
    widgets/systemdefinitiondialog.h \
    elements/label.h \
    widgets/easychange/labeldialog.h \
    jsonkeys.h \
    factories/dimensionadder.h \
    elements/dimension.h \
    widgets/easychange/dimensiondialog.h \
    factories/labeladder.h

FORMS += \
        widgets/mainwindow.ui \
        widgets/seteaiglobaldialog.ui \
        widgets/systemdefinitiondialog.ui

RESOURCES += \
    resources/resources.qrc
