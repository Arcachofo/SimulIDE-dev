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
        Q_OBJECT
    public:
        ResistorDip( QObject* parent, QString type, QString id );
        ~ResistorDip();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        int size() { return m_size; }
        void setSize( int size );

        double getRes() { return m_resist; }
        void setRes( double resist );

        void createResistors( int c );
        void deleteResistors( int d );

        virtual void remove() override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    private:
        double m_resist;
        int m_size;

        std::vector<Pin*> m_pin;
        std::vector<eResistor*> m_resistor;
};

#endif
