/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef UI_COMPPLUGIN_H
#define UI_COMPPLUGIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>

QT_BEGIN_NAMESPACE

class Ui_ComponentPlugins
{
public:
    QGridLayout *gridLayout;
    QListWidget *compList;

    void setupUi(QDialog *ComponentPlugins)
    {
        if (ComponentPlugins->objectName().isEmpty())
            ComponentPlugins->setObjectName(QStringLiteral("ComponentPlugins"));
        ComponentPlugins->resize(312, 246);
        gridLayout = new QGridLayout(ComponentPlugins);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        compList = new QListWidget(ComponentPlugins);
        compList->setObjectName(QStringLiteral("compList"));
        compList->setMaximumSize(QSize(10000, 10000));
        compList->setEditTriggers(QAbstractItemView::NoEditTriggers);
        compList->setProperty("showDropIndicator", QVariant(false));
        compList->setSelectionMode(QAbstractItemView::NoSelection);
        compList->setResizeMode(QListView::Adjust);
        compList->setLayoutMode(QListView::SinglePass);

        gridLayout->addWidget(compList, 0, 0, 1, 1);


        retranslateUi(ComponentPlugins);

        QMetaObject::connectSlotsByName(ComponentPlugins);
    } // setupUi

    void retranslateUi(QDialog *ComponentPlugins)
    {
        ComponentPlugins->setWindowTitle(QApplication::translate("ComponentPlugins", "Manage Components", 0));
    } // retranslateUi

};

namespace Ui {
    class ComponentPlugins: public Ui_ComponentPlugins {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMPPLUGIN_H
