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

#include "e-pin.h"
#include "e-element.h"

class DataChannel;

class MAINMODULE_EXPORT eNode
{
    public:
        eNode( QString id );
        ~eNode();

        QString itemId();

        void addEpin( ePin* epin );
        void remEpin( ePin* epin );

        void voltChangedCallback( eElement* el );
        void remFromChangedCallback( eElement* el );

        void addToNoLinList( eElement* el );
        void remFromNoLinList( eElement* el );

        void stampCurrent( ePin* epin, double data );
        void stampAdmitance( ePin* epin, double data );

        void pinChanged( ePin* epin, int enodeNum );

        int  getNodeNumber();
        void setNodeNumber( int n );

        double getVolt();
        void  setVolt( double volt );
        bool  voltchanged() { return m_voltChanged; }
        void setVoltChanged( bool changed ){ m_voltChanged = changed; }

        void solveSingle();

        void initialize();
        void stampMatrix();
        void stampAdmit();
        void stampCurr();

        void setSingle( bool single );// This eNode can calculate it's own Volt
        bool isSingle();

        void setSwitched( bool switched ); // This eNode has switches attached
        bool isSwitched();

        void setIsBus( bool bus );
        bool isBus() { return m_isBus; }
        void createBus();
        void addBusPinList( QList<ePin*> list, int line );

        void addToPlotterList( DataChannel* el );
        void remFromPlotterList( DataChannel* el );
        void saveData(); // Plotters will read data

        QList<ePin*> getEpins();
        QList<int> getConnections();

        eNode* nextCH;

    private:
        QList<ePin*>     m_ePinList;

        QList<QList<ePin*>> m_eBusPinList;
        QList<eNode*>       m_eNodeList;

        QList<eElement*> m_changedFast;
        QList<eElement*> m_nonLinear;

        QList<DataChannel*> m_plotterList;

        QHash<ePin*, double> m_admitList;
        QHash<ePin*, double> m_currList;
        QHash<ePin*, int>    m_nodeList;

        QHash<int, double>   m_admit;
        QHash<int, double>   m_admitPrev;

        double m_totalCurr;
        double m_totalAdmit;

        double m_volt;
        int   m_nodeNum;
        int   m_numCons;

        QString m_id;

        bool m_currChanged;
        bool m_admitChanged;
        bool m_voltChanged;
        bool m_changed;
        bool m_single;
        bool m_switched;
        bool m_isBus;
};
#endif


