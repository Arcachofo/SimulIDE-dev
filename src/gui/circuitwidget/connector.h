/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "compbase.h"

class ConnectorLine;
class Pin;

class MAINMODULE_EXPORT Connector : public CompBase
{
    Q_OBJECT

    public:
        Connector( QObject* parent, QString type, QString id, Pin* startpin, Pin* endpin = NULL );
        ~Connector();

        QString itemID() { return m_id; }

        QString pListStr() { return m_pointList.join(","); }
        QStringList pointList() { refreshPointList(); return m_pointList; }
        void setPointListStr( QString pl );
        void setPointList( QStringList pl );

        void dummySetter( QString ) {;}

        QString startPinId();
        QString endPinId();
        Pin* startPin() { return m_startPin;}
        void setStartPin( Pin* pin) { m_startPin = pin; }
        Pin* endPin() { return m_endPin; }
        void setEndPin( Pin* pin) { m_endPin = pin; }

        double getVoltage();

        QList<ConnectorLine*>* lineList() { return &m_conLineList; }

        void incActLine() { if( m_actLine < m_conLineList.size()-1 ) m_actLine += 1; }

        ConnectorLine* addConLine( int x1, int y1, int x2, int y2, int index );
        void addConLine( ConnectorLine* line, int index );
        void remNullLines();
        void refreshPointList();
        void updateConRoute( Pin* nod, QPointF this_point );
        void closeCon( Pin* endpin );
        void splitCon( int index, Pin* pin0, Pin* pin2 );

        void updateLines();

        void setVisib(  bool vis );
        void setSelected( bool selected );
        
        void setIsBus( bool bus );
        bool isBus() { return m_isBus; }
        
        void move( QPointF delta );

        void remove();
        void remLines();

        bool m_freeLine;

    private:
        void remConLine( ConnectorLine* line  );
        void updateCon();
        void connectLines( int index1, int index2 );
        void disconnectLines( int index1, int index2 );

        int m_actLine;
        int m_lastindex;
        
        bool m_isBus;

        Pin*    m_startPin;
        Pin*    m_endPin;

        QStringList  m_pointList;

        QList<ConnectorLine*> m_conLineList;
};

#endif
