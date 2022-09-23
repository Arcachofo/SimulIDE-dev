/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MUX_H
#define MUX_H

#include "logiccomponent.h"

class LibraryItem;

class MAINMODULE_EXPORT Mux : public LogicComponent
{
        Q_OBJECT
    public:
        QRectF boundingRect() const { return m_area; }
    
        Mux( QObject* parent, QString type, QString id );
        ~Mux();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override{ IoComponent::runOutputs(); }

        int addrBits() { return m_addrBits; }
        void setAddrBits( int bits );
        
        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        int m_addrBits;

};

#endif
