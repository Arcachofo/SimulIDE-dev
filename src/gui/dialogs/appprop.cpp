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
 *                                                                         *
 ***************************************************************************/

#include "appprop.h"
#include "mainwindow.h"

AppProp::AppProp( QWidget* parent )
        : QDialog( parent )
{
    setupUi(this);
    language->setCurrentIndex( (int)MainWindow::self()->lang() );
    fontScale->setValue( MainWindow::self()->fontScale() );
}

void AppProp::on_helpButton_toggled( bool show )
{
    if( show ) setGeometry( pos().x(), pos().y(), maximumWidth(), maximumHeight() );
    else       setGeometry( pos().x(), pos().y(), minimumWidth(), minimumHeight() );
}

void AppProp::on_language_currentIndexChanged( int index )
{
    MainWindow::self()->setLang( (Langs)index );
}

void AppProp::on_fontScale_valueChanged( double scale )
{
    MainWindow::self()->setFontScale( scale );
}
