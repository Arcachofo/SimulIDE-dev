/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CONNBASE_H
#define CONNBASE_H

#include "component.h"
#include "e-element.h"
#include "pin.h"


class MAINMODULE_EXPORT ConnBase : public Component, public eElement
{
    public:
        ConnBase( QObject* parent, QString type, QString id );
        ~ConnBase();


        int  size() { return m_size; }
        void setSize( int size );

        virtual void registerEnode( eNode*, int n=-1 ) override;

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        void createPins( int c );
        void deletePins( int d );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected:
        virtual void updatePixmap(){;}

        int m_size;

        std::vector<Pin*> m_connPins;
        std::vector<Pin*> m_sockPins;

        Pin::pinType_t m_pinType;

        QPixmap m_pinPixmap;
};

#endif
