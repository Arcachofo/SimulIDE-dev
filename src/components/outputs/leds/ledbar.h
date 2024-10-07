/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LEDBAR_H
#define LEDBAR_H

#include "component.h"
#include "ledsmd.h"

class LibraryItem;

class LedBar : public Component
{
    public:
        LedBar( QString type, QString id );
        ~LedBar();

        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();

        QString colorStr();
        void setColorStr( QString color );

        int  size() { return m_size; }
        void setSize( int size );
        
        double threshold();
        void  setThreshold( double threshold );

        double maxCurrent();
        void  setMaxCurrent( double current );
        
        double resistance();
        void  setResistance( double resist );
        
        bool grounded();
        void setGrounded( bool grounded );
        
        void createLeds( int c );
        void deleteLeds( int d );

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        virtual void remove() override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    private:
        std::vector<LedSmd*> m_led;
        
        int m_size;
};

#endif
