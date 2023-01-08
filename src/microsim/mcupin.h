/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUPIN_H
#define MCUPIN_H

#include "iopin.h"
#include "mcutypes.h"

class McuPort;
class eMcu;
class Interrupt;

class MAINMODULE_EXPORT McuPin : public IoPin
{
    friend class McuPort;
    friend class McuCreator;

    public:
        McuPin( McuPort* port, int i, QString id, Component* mcu );
        ~McuPin();

        enum extIntTrig_t{
            pinLow=0,
            pinChange,
            pinFalling,
            pinRising,
            pinDisabled
        };

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual bool getInpState() override;

        virtual void setOutState( bool state ) override;
        virtual void setPortState( bool state );

        void setDirection( bool out );
        //void setPullupMask( bool up ) { m_puMask = up;}
        void setExtraSource( double vddAdmit, double gndAdmit );
        virtual void controlPin( bool outCtrl, bool dirCtrl ) override;
        void setExtIntTrig( int i ) { m_extIntTrigger = (extIntTrig_t)i; }

        virtual void sheduleState( bool state, uint64_t time ) override;
        virtual void setPullup( bool up ) override;
        virtual void setAnalog( bool an ) {;}
        virtual void ConfExtInt( uint8_t bits );

        void setExtInt( uint mode );

        static void registerScript( asIScriptEngine* engine );

    protected:
        QString m_id;

        McuPort*   m_port;
        Interrupt* m_extInt;

        regBits_t    m_extIntBits;
        extIntTrig_t m_extIntTrigger;

        int m_number;

        bool m_isAnalog;
        bool m_isOut;
        bool m_outMask; // Pin always output
        bool m_inpMask; // Pin always input (inverted: 0 means always input)
        bool m_puMask;  // Pullup always on
        bool m_openColl;
        bool m_changeCB;

        bool m_outCtrl; // Some device controlling out state
        bool m_dirCtrl; // Some device controlling direction
        bool m_portState;
        pinMode_t m_portMode; // Pin mode as set by Port registers

        uint8_t m_pinMask;
};

#endif
