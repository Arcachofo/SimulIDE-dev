/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EPIN_H
#define EPIN_H

#include <QString>

#include "e-node.h"

class eElement;

class MAINMODULE_EXPORT ePin
{
    public:
        ePin( QString id, int index );
        virtual ~ePin();

        bool isConnected() { return (m_enode!=NULL); }

        double getVoltage();

        eNode* getEnode() { return m_enode; }
        void   setEnode( eNode* enode );
        void   setEnodeComp( eNode* enode ); // The enode at other side of component

        void changeCallBack( eElement* el , bool cb=true );

        bool inverted() { return m_inverted; }
        virtual void setInverted( bool inverted ) { m_inverted = inverted; }

        inline void stampAdmitance( double data ) { if( m_enode ) m_enode->stampAdmitance( this, data ); }

        void addSingAdm( int node, double admit );
        void stampSingAdm( double admit );

        void createCurrent();
        inline void stampCurrent( double data ) { if( m_enode ) m_enode->stampCurrent( this, data ); }

        
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
