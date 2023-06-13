/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUPORT_H
#define MCUPORT_H

#include <vector>

#include "mcumodule.h"
#include "mcupin.h"

class Mcu;
class eMcu;
class McuPin;
class Component;

class MAINMODULE_EXPORT McuPort : public McuModule
{
        friend class McuCreator;
        friend class McuPorts;

    public:
        McuPort( eMcu* mcu, QString name );
        ~McuPort();

        virtual void reset() override;
        virtual void pinChanged( uint8_t pinMask, uint8_t val );

        void setPullups( uint8_t puMask );
        void setAllPullups( uint8_t val );
        void clearAllPullups( uint8_t val );

        McuPin* getPinN( uint8_t i );
        McuPin* getPin( QString pinName );

        QString name() { return m_name; }

        virtual void outChanged( uint8_t val );
        virtual void dirChanged( uint8_t val );
        void intChanged( uint8_t val );
        virtual void setIntMask( uint8_t val) { m_intMask = val; }
        virtual void rstIntMask( bool rst) { m_rstIntMask = rst; }

        virtual void readPort( uint8_t );

        void controlPort( bool outCtrl, bool dirCtrl ); // Direct control
        void setDirection( uint val );       // Direct control over pins
        void setOutState( uint val );        // Direct control over pins
        uint getInpState();                  // Direct control over pins
        void setPinMode( pinMode_t mode );   // Direct control over pins

        uint16_t getOutAddr() { return m_outAddr; }
        uint16_t getInAddr() { return m_inAddr; }

        static void registerScript( asIScriptEngine* engine );

    protected:
        void createPins( Mcu* mcuComp, QString pins, uint32_t pinMask );
        virtual McuPin* createPin( int i, QString id , Component* mcu );

        QString m_shortName;

        std::vector<McuPin*> m_pins;
        uint8_t m_numPins;
        uint8_t m_pinState;

        regBits_t m_intBits;
        uint8_t m_intMask;

        bool m_dirInv;
        bool m_rstIntMask;

        uint8_t* m_outReg; // Pointer to m_ram[m_outAddr]
        uint8_t* m_inReg;  // Pointer to m_ram[m_inAddr]
        uint8_t* m_dirReg; // Pointer to m_ram[m_dirAddr]

        uint16_t m_outAddr;
        uint16_t m_inAddr;
        uint16_t m_dirAddr;
};

#endif
