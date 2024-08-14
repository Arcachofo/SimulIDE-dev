/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SR04_H
#define SR04_H

#include "e-element.h"
#include "component.h"

class LibraryItem;
class IoPin;

class SR04 : public Component, public eElement
{
    public:
        SR04( QString type, QString id );
        ~SR04();

        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void stamp() override;
        virtual void initialize() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w );

    private:
        uint64_t m_lastStep;
        bool     m_lastTrig;

        int m_echouS;
        
        Pin* m_inpin;
        Pin* m_trigpin;
        
        IoPin* m_echo;
};

#endif
