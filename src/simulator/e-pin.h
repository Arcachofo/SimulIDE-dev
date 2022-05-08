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

#ifndef EPIN_H
#define EPIN_H

#include <QString>

class eNode;
class eElement;

class MAINMODULE_EXPORT ePin
{
    public:
        ePin( QString id, int index );
        virtual ~ePin();

        bool isConnected() { return (m_enode!=NULL); }

        double getVolt();

        eNode* getEnode() { return m_enode; }
        void   setEnode( eNode* enode );

        //eNode* getEnodeComp() { return m_enodeComp; }
        void   setEnodeComp( eNode* enode ); // The enode at other side of component

        void changeCallBack( eElement* el , bool cb=true );

        bool inverted() { return m_inverted; }
        virtual void setInverted( bool inverted ) { m_inverted = inverted; }

        void stampAdmitance( double data );
        void stampCurrent( double data );
        
        QString getId()  { return m_id; }
        void setId( QString id );

        void setIndex( int i ) { m_index = i; }

    protected:
        eNode* m_enode;     // My eNode
        eNode* m_enodeComp; // eNode at other side of my component

        QString m_id;
        int m_index;

        bool m_inverted;
};

#endif
