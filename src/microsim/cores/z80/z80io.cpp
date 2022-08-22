
#include "z80io.h"
#include "e_mcu.h"
#include "mcupin.h"

Z80io::Z80io( eMcu* mcu )
     : eElement( mcu->getId()+"-z80io" )
{
    m_dataPort = mcu->getPort("PORTD");
    m_addrPort = mcu->getPort("PORTA");

    m_m1Pin     = mcu->getPin("M1");
    m_mreqPin   = mcu->getPin("MREQ");
    m_iorqPin   = mcu->getPin("IORQ");
    m_wrPin     = mcu->getPin("WR");
    m_rdPin     = mcu->getPin("RD");
    m_rfshPin   = mcu->getPin("RFSH");
    m_haltPin   = mcu->getPin("HALT");
    m_waitPin   = mcu->getPin("WAIT");
    m_intPin    = mcu->getPin("INT");
    m_nmiPin    = mcu->getPin("NMI");
    m_busreqPin = mcu->getPin("BUSRQ");
    m_busacPin  = mcu->getPin("BUSAK");
}
Z80io::~Z80io(){}

void Z80io::stamp()
{
    m_m1Pin->controlPin( true, true );
    m_mreqPin->controlPin( true, true );
    m_iorqPin->controlPin( true, true );
    m_wrPin->controlPin( true, true );
    m_rdPin->controlPin( true, true );
    m_rfshPin->controlPin( true, true );
    m_haltPin->controlPin( true, true );
    m_waitPin->controlPin( true, true );
    m_intPin->controlPin( true, true );
    m_nmiPin->controlPin( true, true );
    m_busreqPin->controlPin( true, true );
    m_busacPin->controlPin( true, true );

    m_mreqPin->setOutState( true );
    m_iorqPin->setOutState( true );
    m_rdPin->setOutState( true );
    m_wrPin->setOutState( true );
    m_rfshPin->setOutState( true );

    m_m1Pin->setPinMode( output );
    m_m1Pin->setOutState( false );
    m_haltPin->setPinMode( output );
    m_haltPin->setOutState( false );
    m_waitPin->setPinMode( input );
    m_intPin->setPinMode( input );
    m_nmiPin->setPinMode( input );
    m_resetPin->setPinMode( input );
    m_busreqPin->setPinMode( input );
    m_busacPin->setPinMode( output );
    m_busacPin->setOutState( true );
}

void Z80io::runEvent()
{
    if( !m_nextClock ) risingEdgeDelayed();  // Rising edge
    else               fallingEdgeDelayed(); // Falling edge
}

void Z80io::risingEdgeDelayed()
{
    // When bus is requested by other device by BUSRQ signal then signal BUSACK is set and bus is set to high impedance
    // or when normal reset is requested by RESET signal the bus is set to high impedance
    if( ( sBusAck || normalReset ) && !highImpedanceBus )  /// At T1 after BUSREQ ???
    {
        /// confusion betwen sBusAck and sBusReq
        m_busacPin->setOutState( false );               // set BUSACK
        releaseBus( true );
    }
    // Setting bus only when not requested by BUSRQ and acknowledged by signal BUSACK or normal reset is requested by RESET signal
    if( !sBusAck && !normalReset  )                      /// At T1 after BUSREQ ???
    {
        if( highImpedanceBus ) releaseBus( false ); // If bus is in high impedance then set bus to low impedance

        switch( sm_TState )// Setting bus at clock rising edge of TState 1 (it might repeat when wait states are inserted during interrupt)
        {
        case 1: if( !sm_waitTState  )
                {
                    m_addrPort->setOutState( sAO );
                    if( m_lastBusOp == oMemWrite || m_lastBusOp == oIOWrite ) m_dataPort->setPinMode( input );// If previous bus op. was write data then release data bus
                    if( mc_busOp    == oM1 || mc_busOp    == oM1Int )  m_m1Pin->setOutState( false );// If current bus op. is Fetch (machine cycle 1) then set M1
                    if( m_lastBusOp == oM1 || m_lastBusOp == oM1Int  ) m_rfshPin->setOutState( true );// If previous bus op. was Fetch (machine cycle 1) then signal RFSH is reset
                }
                break;
        case 2: if( sm_waitTState )// Setting bus at clock rising edge of TState 2 (it might repeat when wait states are inserted)
                {
                    if( mc_busOp == oIORead ) {              // If current bus op. is read I/O then set IORQ and RD
                        m_iorqPin->setOutState( false );
                        m_rdPin->setOutState( false );
                    }
                    if( mc_busOp == oIOWrite ) {            // If current bus op. is write I/O then set IORQ and WR
                        m_iorqPin->setOutState( false );
                        m_wrPin->setOutState( false );
                    }
                }
                break;
        case 3: if( mc_busOp == oM1 || mc_busOp == oM1Int ) // If current bus op. is Fetch (machine cycle 1) then reset M1, set RFSH and update address bus
                {
                    m_m1Pin->setOutState( true );
                    m_rfshPin->setOutState( false );

                    /// address value should be already available
                    m_addrPort->setOutState( (regI << 8) | regR );
                    regR = ( (regR + 1) & 0x7f ) + ( regR & 0x80 );
                }
                if( mc_busOp == oM1 ) {// It current bus op. is Fetch without interrupt then reset MREQ and RD
                    m_mreqPin->setOutState( true );
                    m_rdPin->setOutState( true );
                }
                if( mc_busOp == oM1Int ) m_iorqPin->setOutState( true );// If current bus op. is Fetch with interrupt then reset IORQ
                break;
                // Setting bus at clock rising edge of TState 5 in case of longer machine cycle than 4 TStates
        case 5: if( mc_busOp == oM1 || mc_busOp == oM1Int ) // If bus op. is Fetch (machine cycle 1) then address bus is restored and reset RFSH
                {
                    m_addrPort->setOutState( sAO );
                    m_rfshPin->setOutState( true );
                }
        }
    }
}

void Z80io::fallingEdgeDelayed()
{
    if( sBusAck == false ) { // Setting bus only when bus is not requested by signal BUSRQ and acknowledged by signal BUSACK
        switch( sm_TState ) {
                // Setting bus at clock rising edge of TState 1 (it might repeat when wait states are inserted during interrupt)
        case 1: if( sm_waitTState == false )
                {
                    if( mc_busOp == oM1 ) {// If  current bus op. is Fetch without interrupt (machine cycle 1) then set MREQ and M1
                        m_mreqPin->setOutState( false );
                        m_rdPin->setOutState( false );   /// ERROR ???
                    }
                    if( mc_busOp == oMemRead ) {              // If  current bus op. is read  memory then set MREQ and RD
                        m_mreqPin->setOutState( false );
                        m_rdPin->setOutState( false );
                    }
                    if( mc_busOp == oMemWrite) m_mreqPin->setOutState( false ); // If  current bus op. is write memory then set MREQ
                    if( mc_busOp == oMemWrite || mc_busOp == oIOWrite )// If current bus op. is write memory or I/O then set data bus
                        m_dataPort->setOutState( sDO );
                }
                break;
        case 2: if( sm_waitTState == false ) {// Setting bus at clock rising edge of TState 2 (it might repeat when wait states are inserted)
                    // If  current bus op. is Fetch with interrupt (machine cycle 1) then set IORQ
                    // (There is a difference in TStates numbering from datasheet - one automatically inserted wait states is T1 wait)
                    if (mc_busOp == oM1Int) m_iorqPin->setOutState( false );
                    if (mc_busOp == oMemWrite) m_wrPin->setOutState( false ); // If  current bus op. is write memory then set WR
                }
                break;
        case 3: if( mc_busOp == oMemRead || mc_busOp == oIORead )// Sampling data bus for Memory read and Input/Output read cycle
                    sDI = m_dataPort->getInpState();

            /// If  current bus op. is Fetch (machine cycle 1) then reset MREQ
            /// reset or set???
                if( mc_busOp == oM1 || mc_busOp == oM1Int )
                    m_mreqPin->setOutState( false );

                if( mc_busOp == oMemRead ) {                 // If  current bus op. is read memory then reset MREQ and RD
                    m_mreqPin->setOutState( true );
                    m_rdPin->setOutState( true );
                }
                if( mc_busOp == oMemWrite ) {               // If  current bus op. is write memory then reset MREQ and WR
                    m_mreqPin->setOutState( true );
                    m_wrPin->setOutState( true );
                }
                if( mc_busOp == oIORead ) {                 // If  current bus op. is read I/O then reset IORQ and RD
                    m_iorqPin->setOutState( true );
                    m_rdPin->setOutState( true );
                }
                if( mc_busOp == oIOWrite ) {                // If  current bus op. is write I/O then reset IORQ and WR
                    m_iorqPin->setOutState( true );
                    m_wrPin->setOutState( true );
                }
                break;
        case 4: if( mc_busOp == oM1 || mc_busOp == oM1Int ) m_mreqPin->setOutState( true ); // If current bus op. is Fetch (machine cycle 1) then reset MREQ
            /// create Halt variable, state or whatever
                if( m_iReg == 0x76 && m_iSet == noPrefix )  m_haltPin->setOutState( false ); // If  current instruction is HALT then set HALT
                if( sNMI || ( sInt && IFF1 ) )              m_haltPin->setOutState( true ); // If interrupt is requested and enabled then reset HALT
        }
    }/// confusion betwen sBusAck and sBusReq:
    else if( !sBusReq ) m_busacPin->setOutState( true ); // If bus is not requested any more by BUSRQ signal then signal BUSACK is reset
}

void Z80io::releaseBus( bool rel )
{
    pinMode_t mode = rel ? input : output;

    m_mreqPin->setPinMode( mode );                          // MREQ to low impedance
    m_iorqPin->setPinMode( mode );                          // IORQ to low impedance
    m_rdPin->setPinMode( mode );                            // RD   to low impedance
    m_wrPin->setPinMode( mode );                            // WR   to low impedance
    m_rfshPin->setPinMode( mode );                          // RFSH to low impedance

    //for( int i=0; i<16; ++i ) m_outPin[i]->setStateZ(  mode ); // set address bus to low impedance
    m_addrPort->setPinMode( mode );
    //releaseDataBus();                                       // set data bus to high impedance
    m_dataPort->setPinMode( input );

    highImpedanceBus = rel;                                 // reset flag that bus is in high impedance
}
