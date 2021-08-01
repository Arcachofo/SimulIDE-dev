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

#ifndef PROBE_H
#define PROBE_H

#include "component.h"
#include "e-element.h"

class IoPin;
class Connector;
class LibraryItem;

class MAINMODULE_EXPORT Probe : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( bool   Show_volt READ showVal  WRITE setShowVal  DESIGNABLE true USER true )
    Q_PROPERTY( double Threshold READ trigVolt WRITE setTrigVolt DESIGNABLE true USER true )

    public:
        Probe( QObject* parent, QString type, QString id );
        ~Probe();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        double trigVolt() { return m_voltTrig; }
        void setTrigVolt( double volt ) { m_voltTrig = volt; update();}
        
        void setVolt( double volt );
        double getVolt();

        virtual void updateStep() override;
        virtual void remove() override;

        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private: 
        double m_voltIn;
        double m_voltTrig;

        QColor m_plotterColor;

        IoPin*     m_inputPin;
        Pin*       m_readPin;
        Connector* m_readConn;
};


#endif
