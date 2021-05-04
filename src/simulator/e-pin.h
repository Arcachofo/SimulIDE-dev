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

enum pinState_t{
    undef_state=0,
    out_open,
    out_low,
    out_high,
    input_low,
    input_high
};

class eNode;

class MAINMODULE_EXPORT ePin
{
    public:
        ePin( QString id, int index );
        virtual ~ePin();

        bool isConnected() { return m_connected; }
        void setConnected( bool connected );

        double getVolt();

        eNode* getEnode();
        void   setEnode( eNode* enode );

        eNode* getEnodeComp() { return m_enodeCon; }
        void   setEnodeComp( eNode* enode ); // The enode at other side of component

        bool inverted() { return m_inverted; }
        void setInverted( bool inverted );

        void stampAdmitance( double data );
        void stampCurrent( double data );

        virtual void setPinState( pinState_t st ) { m_pinState = st; }

        void reset();
        
        QString getId();
        void setId( QString id );

    protected:
        eNode* m_enode;
        eNode* m_enodeCon;

        QString m_id;
        int m_index;

        pinState_t m_pinState;

        bool m_connected;
        bool m_inverted;
};

#endif
