/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef RESISTORDIP_H
#define RESISTORDIP_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class eResistor;
class Pin;

class MAINMODULE_EXPORT ResistorDip : public Component, public eElement
{
    public:
        ResistorDip( QObject* parent, QString type, QString id );
        ~ResistorDip();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        virtual void stamp() override;
        virtual void updateStep() override;

        int size() { return m_size; }
        void setSize( int size );

        double getRes() { return m_resist; }
        void setRes( double resist );

        bool pullUp() { return m_pullUp; }
        void setPullUp( bool p );

        double puVolt() { return m_puVolt; }
        void setPuVolt( double pv ) { m_puVolt = pv; }

        void createResistors( int c );
        void deleteResistors( int d );

        virtual void remove() override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    protected slots:
        virtual void slotProperties() override;

    private:
        double m_resist;
        int m_size;

        bool m_pullUp;
        double m_puVolt;

        std::vector<Pin*> m_pin;
        std::vector<eResistor*> m_resistor;

        static eNode m_puEnode;
};

#endif
