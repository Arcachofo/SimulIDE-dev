/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#include "currsource.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "pin.h"

static const char* CurrSource_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Current"),
    QT_TRANSLATE_NOOP("App::Property","Show Amp")
};

Component* CurrSource::construct( QObject* parent, QString type, QString id )
{
    return new CurrSource( parent, type, id );
}

LibraryItem* CurrSource::libraryItem()
{
    return new LibraryItem(
        tr( "Current Source" ),
        tr( "Sources" ),
        "voltsource.png",
        "Current Source",
        CurrSource::construct );
}

CurrSource::CurrSource( QObject* parent, QString type, QString id )
          : VarSource( parent, type, id )
{
    Q_UNUSED( CurrSource_properties );

    m_outPin = new Pin( 0, QPoint(28,16), id+"-outPin", 0, this );
    m_pin.resize(1);
    m_pin[0] = m_outPin;

    m_unit = "A";
    m_button->setText( QString("-- A") );
    setValue( 1 );
    valueChanged( 0 );
}
CurrSource::~CurrSource(){}

QList<propGroup_t> CurrSource::propGroups()
{
    propGroup_t mainGroup { tr("Main") };
    mainGroup.propList.append( {"Value", tr("Max. Current"),"main"} );
    return {mainGroup};
}

void CurrSource::updateStep()
{
    if( m_changed ) 
    {
        double current = 0;
        if( m_button->isChecked() ) current = m_outValue;
        m_outPin->stampCurrent( current );

        m_changed = false;
    }
}

#include "moc_currsource.cpp"
