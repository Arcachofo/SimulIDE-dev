
#ifndef Z80IO_H
#define Z80IO_H

#include "e-element.h"

class eMcu;
class McuPort;
class McuPin;

class MAINMODULE_EXPORT Z80io : public eElement
{
    public:
        Z80io( eMcu* mcu );
        ~Z80io();

        virtual void stamp() override;
        virtual void runEvent() override;

    protected:
        void risingEdgeDelayed();
        void fallingEdgeDelayed();
        void releaseBus( bool rel );


        ///--------------------------------------
        /// Should not be here: -----------------

        uint8_t sm_TState;
        uint8_t m_iReg;

        uint8_t regI;
        uint8_t regR;

        bool normalReset;
        bool IFF1;

        enum ePrefix{
            noPrefix=0,
            prefixCB,
            prefixED,
            prefixIX,
            prefixIY
        };
        ePrefix m_iSet;

        ///--------------------------------------
        ///--------------------------------------



        enum eBusOperation { oNone, oM1, oM1Int, oMemRead, oMemWrite, oIORead, oIOWrite };
        eBusOperation mc_busOp;
        eBusOperation m_lastBusOp;

        bool m_nextClock;
        //bool m_releaseBus;
        //bool m_ownBus;
        bool highImpedanceBus;

        uint64_t m_delay;
        McuPort* m_dataPort;
        McuPort* m_addrPort;

        uint8_t sm_autoWait;
        bool sm_waitTState;
        bool sWait;
        bool sBusReq;
        bool sBusAck;

        // Z80Core sampled bus signals
        bool sNMI;
        bool sInt;

        uint8_t sDI;
        uint8_t sDO;
        uint16_t sAO;

            // Output pins
        McuPin* m_m1Pin;
        McuPin* m_mreqPin;
        McuPin* m_iorqPin;
        McuPin* m_rdPin;
        McuPin* m_wrPin;
        McuPin* m_rfshPin;
        McuPin* m_haltPin;
        McuPin* m_busacPin;
           // Input Pins
        McuPin* m_waitPin;
        McuPin* m_intPin;
        McuPin* m_nmiPin;
        McuPin* m_busreqPin;
        McuPin* m_resetPin;
};
#endif
