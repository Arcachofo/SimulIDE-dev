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

#ifndef LDR_H
#define LDR_H

#include "varresbase.h"

class LibraryItem;

class MAINMODULE_EXPORT Ldr : public VarResBase
{
    Q_OBJECT
    Q_PROPERTY( int Min_Lux   READ minVal WRITE setMinVal DESIGNABLE true USER true )
    Q_PROPERTY( int Max_Lux   READ maxVal WRITE setMaxVal DESIGNABLE true USER true )
    Q_PROPERTY( int    Lux    READ getVal WRITE setVal    DESIGNABLE true USER true )
    Q_PROPERTY( double Gamma  READ gamma  WRITE setGamma  DESIGNABLE true USER true )
    Q_PROPERTY( int    R1     READ r1     WRITE setR1     DESIGNABLE true USER true )

    public:
        Ldr( QObject* parent, QString type, QString id );
        ~Ldr();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual QList<propGroup_t> propGroups() override;

        int r1() { return m_r1; }
        void setR1( int r1 );

        double gamma() { return m_gamma; }
        void setGamma( double ga );

        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget *widget );

    private:
        int m_r1;

        double m_gamma;
};

#endif
