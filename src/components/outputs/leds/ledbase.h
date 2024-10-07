/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LEDBASE_H
#define LEDBASE_H

#include "e-led.h"
#include "component.h"
#include "e-node.h"

class eNode;

class LedBase : public Component, public eLed
{
    public:
        LedBase( QString type, QString id );
        ~LedBase();

        enum ledColor_t {
            yellow = 0,
            red,
            green,
            blue,
            orange,
            purple,
            white
        };

        QString colorStr() { return m_ledColorStr; }
        void setColorStr( QString color );

        bool grounded() { return m_grounded; }
        void setGrounded( bool grounded );

        virtual void initialize() override;
        virtual void updateStep() override;

 static QColor getColor( ledColor_t c, int bright );
 static QString getColorList();

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    protected:
        virtual void drawBackground( QPainter* p )=0;
        virtual void drawForeground( QPainter* p )=0;
        
        bool   m_grounded;

 static int m_overBright;
        
        QString m_ledColorStr;
        ledColor_t m_ledColor;

        static eNode m_gndEnode;
};

#endif
