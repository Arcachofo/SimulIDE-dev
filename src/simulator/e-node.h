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

#ifndef ENODE_H
#define ENODE_H

#include<QHash>

class ePin;
class eElement;

class MAINMODULE_EXPORT eNode
{
    public:
        eNode( QString id );
        ~eNode();

        QString itemId() { return m_id; }

        void addEpin( ePin* epin );
        void remEpin( ePin* epin );
        void clear();

        void voltChangedCallback( eElement* el );
        void remFromChangedCallback( eElement* el );

        void addToNoLinList( eElement* el );
        void remFromNoLinList( eElement* el );

        void stampCurrent( ePin* epin, double current );
        void stampAdmitance( ePin* epin, double admit );

        int  getNodeNumber() { return m_nodeNum; }
        void setNodeNumber( int n ) { m_nodeNum = n; }

        double getVolt() { return m_volt; }
        void   setVolt( double volt );
        bool voltchanged() { return m_voltChanged; }
        void setVoltChanged( bool changed ){ m_voltChanged = changed; }

        void solveSingle();

        void initialize();
        void stampMatrix();

        void setSingle( bool single ) { m_single = single; } // This eNode can calculate it's own Volt
        void setSwitched( bool switched ){ m_switched = switched; } // This eNode has switches attached

        QList<ePin*> getEpins() { return m_ePinList; }

        void addConnection( ePin* epin, int enodeNum );
        QList<int> getConnections();

        eNode* nextCH;

    private:
        QString m_id;

        QList<ePin*> m_ePinList;

        QList<eElement*> m_changedFast;
        QList<eElement*> m_nonLinear;

        QHash<ePin*, double> m_admitList;
        QHash<ePin*, double> m_currList;
        QHash<ePin*, int>    m_nodeList;

        QHash<int, double>   m_admit;

        double m_totalCurr;
        double m_totalAdmit;
        double m_volt;

        int m_nodeNum;

        bool m_currChanged;
        bool m_admitChanged;
        bool m_voltChanged;
        bool m_changed;
        bool m_single;
        bool m_switched;
};
#endif


