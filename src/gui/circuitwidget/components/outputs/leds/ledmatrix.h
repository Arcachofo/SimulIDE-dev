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

#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include "component.h"
#include "e-element.h"
#include "ledsmd.h"

class LibraryItem;

class MAINMODULE_EXPORT LedMatrix : public Component, public eElement
{
        Q_OBJECT
    public:
        LedMatrix( QObject* parent, QString type, QString id );
        ~LedMatrix();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        void setColorStr( QString c );
        QString colorStr() { return m_ledColor; }
        
        int  rows() { return m_rows; }
        void setRows( int rows );
        
        int  cols() { return m_cols; }
        void setCols( int cols );
        
        bool verticalPins() { return m_verticalPins; }
        void setVerticalPins( bool v );
        
        double threshold() { return m_threshold; }
        void   setThreshold( double threshold );

        double maxCurrent() { return m_maxCurr; }
        void   setMaxCurrent( double current );
        
        double res() { return m_resist; }
        void  setRes( double resist );

        virtual void attach() override;
        virtual void remove() override;

        virtual void setHidden( bool hid , bool hidLabel=false );

        virtual QStringList getEnums( QString e ) override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        void setupMatrix( int rows, int cols );
        void createMatrix();
        void deleteMatrix();
        
        std::vector<std::vector<LedSmd*>> m_led;
        std::vector<Pin*> m_rowPin;
        std::vector<Pin*> m_colPin;
        
        bool m_verticalPins;

        QString m_ledColor;

        double m_resist;
        double m_maxCurr;
        double m_threshold;
        
        int m_rows;
        int m_cols;
};

#endif
