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

#ifndef SEVENSEGMENT_H
#define SEVENSEGMENT_H

#include "component.h"
#include "e-element.h"
#include "ledsmd.h"

class LibraryItem;

class MAINMODULE_EXPORT SevenSegment : public Component, public eElement
{
        Q_OBJECT
    public:
        SevenSegment( QObject* parent, QString type, QString id );
        ~SevenSegment();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        QString colorStr() { return m_ledColor; }
        void setColorStr( QString c );

        int numDisplays() { return m_numDisplays; }
        void setNumDisplays( int dispNumber );

        bool verticalPins() { return m_verticalPins; }
        void setVerticalPins( bool v );
        
        bool isComCathode() { return m_commonCathode; }
        void setComCathode( bool isCommonCathode );
        
        double threshold() { return m_threshold; }
        void   setThreshold( double threshold );
        
        double maxCurrent() { return m_maxCurrent; }
        void   setMaxCurrent( double current );

        double resistance() { return m_resistance; }
        void   setResistance( double res );

        virtual void stamp() override;

        virtual QStringList getEnums( QString e ) override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    private:
        void createDisplay( int n );
        void deleteDisplay( int n );
        void resizeData( int displays );

        bool m_commonCathode;
        bool m_verticalPins;
        
        int  m_numDisplays;
        double m_threshold;
        double m_maxCurrent;
        double m_resistance;

        QString m_ledColor;

        std::vector<Pin*>   m_commonPin;
        std::vector<ePin*>  m_cathodePin;
        std::vector<ePin*>  m_anodePin;
        std::vector<LedSmd*> m_segment;
        eNode* m_enode[8];
};

#endif

