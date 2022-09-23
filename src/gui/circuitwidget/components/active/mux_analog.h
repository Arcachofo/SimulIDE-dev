/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MUXANALOG_H
#define MUXANALOG_H

#include "component.h"
#include "e-element.h"

class eResistor;
class LibraryItem;

class MAINMODULE_EXPORT MuxAnalog : public Component, public eElement
{
        Q_OBJECT
    public:
        MuxAnalog( QObject* parent, QString type, QString id );
        ~MuxAnalog();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;
        virtual void remove() override;

        int addrBits() { return m_addrBits; }
        void setAddrBits( int bits );

        double impedance() { return 1/m_admit; }
        void setImpedance( double i ) { m_admit = 1/i; }
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
        
    private:
        void createAddrBits( int c );
        void deleteAddrBits( int d );
        void createResistors( int c );
        void deleteResistors( int d );

        int m_addrBits;
        int m_channels;
        int m_address;

        double m_admit;

        bool m_enabled;

        Pin* m_zPin;
        Pin* m_enPin;

        std::vector<eResistor*> m_resistor;
        std::vector<Pin*> m_addrPin;
        std::vector<Pin*> m_chanPin;
};

#endif
