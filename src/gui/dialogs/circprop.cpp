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

#include "circprop.h"
#include "circuit.h"
#include "simulator.h"

CircProp::CircProp( QWidget* parent )
        : QDialog( parent )
{
    setupUi(this);
    drawGrid->setChecked( Circuit::self()->drawGrid() );
    showScroll->setChecked( Circuit::self()->showScroll() );
    animate->setChecked( Circuit::self()->animate() );
    fps->setValue( Simulator::self()->fps() );
    backup->setValue( Circuit::self()->autoBck() );
}

void CircProp::on_helpButton_toggled( bool show )
{
    if( show ) setGeometry( pos().x(), pos().y(), maximumWidth(), maximumHeight() );
    else       setGeometry( pos().x(), pos().y(), minimumWidth(), minimumHeight() );
}

void CircProp::on_drawGrid_toggled( bool draw )
{
    Circuit::self()->setDrawGrid( draw );
}

void CircProp::on_showScroll_toggled( bool show )
{
    Circuit::self()->setShowScroll( show );
}

void CircProp::on_animate_toggled( bool ani )
{
    Circuit::self()->setAnimate( ani );
}

void CircProp::on_fps_valueChanged( int fps )
{
    Simulator::self()->setFps( fps );
}

void CircProp::on_backup_valueChanged( int secs )
{
    Circuit::self()->setAutoBck( secs );
}
