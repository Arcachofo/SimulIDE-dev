/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         */

#include <QDialog>

#include "ui_appprop.h"

#ifndef APPDIALOG_H
#define APPDIALOG_H


class AppProp : public QDialog, private Ui::AppProp
{
    Q_OBJECT
    
    public:
        AppProp( QWidget* parent=0 );

    public slots:

        void on_helpButton_toggled( bool show );
        void on_language_currentIndexChanged( int index );
        void on_fontScale_valueChanged( double scale );
};


#endif
