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

#ifndef CHIP_H
#define CHIP_H

#include "component.h"
#include "e-element.h"

class QDomElement;

class MAINMODULE_EXPORT Chip : public Component, public eElement
{
    public:
        Chip( QObject* parent, QString type, QString id );
        ~Chip();

        enum subcType_t{
            None=0,
            Logic,
            Board,
            Shield,
            Module
        };
        
        bool logicSymbol() { return m_isLS; }
        virtual void setLogicSymbol( bool ls );

        QString background() { return m_background; }
        virtual void setBackground( QString bck ) override;

        QString name() { return m_name; }
        virtual void setName( QString name );

        subcType_t subcType() { return m_subcType; }

        QString subcTypeStr() { return m_enumUids.at( (int)m_subcType ); }
        virtual void setSubcTypeStr( QString s ){;}


        int pkgWidth() { return m_width; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    protected:
        virtual void addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle, int length=8);

        virtual void updatePin( QString id, QString type, QString label,
                                int pos, int xpos, int ypos, int angle ){;}

        void deletePin( Pin* pin );

        virtual void initChip();
        virtual void initPackage_old( QDomElement root );
        virtual void initPackage( QDomElement root );

        int m_numpins;
        int m_width;
        int m_height;
        
        bool m_isLS;
        bool m_initialized;

        subcType_t m_subcType;

        QColor m_lsColor;
        QColor m_icColor;

        QString m_name;
        QString m_pkgeFile;     // file containig package defs
        
        QList<Pin*> m_pins;

        QPixmap* m_BackPixmap;
        QGraphicsTextItem m_label;
};

#endif
