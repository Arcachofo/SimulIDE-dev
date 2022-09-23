/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         */

#include <QDialog>

#include "ui_editorprop.h"

#ifndef EDITORDIALOG_H
#define EDITORDIALOG_H

class EditorProp : public QDialog, private Ui::EditorProp
{
    Q_OBJECT
    
    public:
        EditorProp( QWidget* parent );

    public slots:

        void on_fontSize_valueChanged( int size );
        void on_tabSize_valueChanged( int size );
        void on_tabSpaces_toggled( bool spaces );
        void on_showSpaces_toggled( bool show );
};

#endif
