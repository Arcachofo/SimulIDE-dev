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

#ifndef DIODE_H
#define DIODE_H

#include "e-diode.h"
#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT Diode : public Component, public eDiode
{
    Q_OBJECT
    Q_PROPERTY( QString Model      READ model     WRITE setModel     DESIGNABLE true USER true )
    Q_PROPERTY( double  Threshold  READ threshold WRITE setThreshold DESIGNABLE true USER true )
    Q_PROPERTY( double  BrkDownV   READ brkDownV  WRITE setBrkDownV  DESIGNABLE true USER true )
    Q_PROPERTY( double  SatCur_nA  READ satCur_nA WRITE setSatCur_nA DESIGNABLE true USER true )
    Q_PROPERTY( double  EmCoef     READ emCoef    WRITE setEmCoef    DESIGNABLE true USER true )
    Q_PROPERTY( double  MaxCurrent READ maxCurrent WRITE setMaxCurrent DESIGNABLE true USER true )
    Q_PROPERTY( double  Resistance READ res       WRITE setRes       DESIGNABLE true USER true )

    Q_PROPERTY( double Zener_Volt READ zenerV    WRITE setZenerV    DESIGNABLE true USER true ) // Compatibility with old circuits.

    public:
        Diode( QObject* parent, QString type, QString id, bool zener=false );
        ~Diode();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        virtual void initialize() override;
        virtual void updateStep() override;

        double zenerV(){ return m_bkDown; }
        void   setZenerV( double zenerV );

        virtual QStringList getEnums() override { return m_diodes.keys(); }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        bool m_isZener;
};

#endif
