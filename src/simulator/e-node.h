/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

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
        //void remFromNoLinList( eElement* el );

        //void createAdmitance( ePin* epin, int node );
        void stampAdmitance( ePin* epin, double admit );

        void createCurrent( ePin* epin );
        void stampCurrent( ePin* epin, double current );

        int  getNodeNumber() { return m_nodeNum; }
        void setNodeNumber( int n ) { m_nodeNum = n; }

        double getVolt() { return m_volt; }
        void   setVolt( double volt );
        bool voltchanged() { return m_voltChanged; }
        void setVoltChanged( bool changed ){ m_voltChanged = changed; }

        void initialize();
        void stampMatrix();

        void setSingle( bool single ) { m_single = single; } // This eNode can calculate it's own Volt
        void setSwitched( bool switched ){ m_switched = switched; } // This eNode has switches attached

        QList<ePin*> getEpins() { return m_ePinList; }

        void addConnection( ePin* epin, int enodeNum );
        QList<int> getConnections();

        eNode* nextCH;

    private:
        class Connection
        {
            public:
                Connection( ePin* e, int n=0, double v=0 ){ epin = e; node = n; value = v; }
                ~Connection(){;}

                Connection* next;
                ePin*  epin;
                int    node;
                double value;
        };
        class LinkedElement
        {
            public:
                LinkedElement( eElement* el ) { element = el; }
                ~LinkedElement(){;}

                LinkedElement* next;
                eElement* element;
        };

        inline void solveSingle();

        void clearElmList( LinkedElement* first );
        void clearConnList( Connection* first );

        QString m_id;

        QList<ePin*> m_ePinList;

        LinkedElement* m_voltChEl;
        LinkedElement* m_nonLinEl;
        //QList<eElement*> m_changedFast;
        //QList<eElement*> m_nonLinear;

        Connection* m_firstAdmit;
        Connection* m_firstCurrent;
        Connection* m_nodeAdmit;

        //QHash<ePin*, double> m_admitList;
        //QHash<ePin*, double> m_currList;
        QHash<ePin*, int>    m_nodeList;
        //QHash<int, double>   m_admit;

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

