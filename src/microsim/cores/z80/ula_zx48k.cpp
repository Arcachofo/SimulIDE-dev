/***************************************************************************
 *   Copyright (C) 2023 by Jarda Vrana                                     *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "ula_zx48k.h"
#include "display.h"
#include "simulator.h"
#include "ioport.h"
#include "mcu.h"
#include "watcher.h"
#include "iopin.h"

#include "stringprop.h"
#include "boolprop.h"

// The source of information is the book The ZX Spectrum ULA - How To Design A Microcomputer written by Chris Smith, ISBN 978-0-9565071-0-5
const float ULA_ZX48k::m_yTable[16]   = { 2.449, 2.169, 1.896, 1.617, 1.370, 1.091, 0.818, 0.539
                                        , 2.449, 2.082, 1.724, 1.356, 1.033, 0.666, 0.308, 0.259 };
const float ULA_ZX48k::m_uTable[8]    = { 2.016, 1.013, 2.358, 1.347, 2.692, 1.681, 3.027, 2.016 };
const float ULA_ZX48k::m_vTable[8]    = { 1.925, 2.153, 0.495, 0.737, 3.099, 3.341, 1.684, 1.925 };
const float ULA_ZX48k::m_vInvTable[8] = { 1.911, 1.684, 3.341, 3.099, 0.737, 0.495, 2.153, 1.911 };

const float ULA_ZX48k::m_micSp5CTable[4]  = { 0.391, 0.728, 3.653, 3.790 };
const float ULA_ZX48k::m_micSp6CTable[4]  = { 0.342, 0.652, 3.591, 3.753 };
const float ULA_ZX48k::m_micSpImpTable[4] = { 470.9, 1604.0, 28.7, 30.0 };

int ULA_ZX48k::m_colours[16] = { 0x000000, 0x00007f, 0x7f0000, 0x7f007f,\
                                 0x007f00, 0x007f7f, 0x7f7f00, 0x7f7f7f,\
                                 0x000000, 0x0000ff, 0xff0000, 0xff00ff,\
                                 0x00ff00, 0x00ffff, 0xffff00, 0xffffff };

ULA_ZX48k::ULA_ZX48k( eMcu* mcu )
         : CpuBase( mcu )
         , eElement( mcu->getId()+"-Z80Core" )
{
    m_type = ula6c001e7;                                            // default type

    m_enumUids = QStringList() // initialization list of types
        <<"ULA 5C102E"
        <<"ULA 5C112E"
        <<"ULA 6C001E6"
        <<"ULA 6C001E7"
        <<"ULA 6C011E";

    m_enumNames = m_enumUids;

    mcu->createWatcher( this );
    Watcher* watcher = mcu->getWatcher();

    watcher->addRegister( "Horizontal Counter", "uint16" );
    watcher->addRegister( "Vertical Counter", "uint16" );
    watcher->addRegister( "Border Colour"  , "uint8" );
    watcher->addRegister( "Data Latch"     , "uint8" );
    watcher->addRegister( "Atr. Data Latch", "uint8" );
    watcher->addRegister( "Shift Register" , "uint8" );

    m_rasPin    = mcu->getIoPin("RAS");
    m_casPin    = mcu->getIoPin("CAS");
    m_dramwePin = mcu->getIoPin("DRAMWE");
    m_mreqPin   = mcu->getIoPin("MREQ");
    m_iorqPin   = mcu->getIoPin("IORQ");
    m_rdPin     = mcu->getIoPin("RD");
    m_wrPin     = mcu->getIoPin("WR");
    m_a14Pin    = mcu->getIoPin("A14");
    m_a15Pin    = mcu->getIoPin("A15");
    m_intPin    = mcu->getIoPin("INT");
    m_romcsPin  = mcu->getIoPin("ROMCS");
    m_phicpuPin = mcu->getIoPin("PHICPU");
    m_uPin      = mcu->getIoPin("U");
    m_vPin      = mcu->getIoPin("V");
    m_yPin      = mcu->getIoPin("Y");
    m_micTapePin = mcu->getIoPin("MICTAPE");
    
    m_dmaPort = mcu->getIoPort("PORTA");
    m_dPort   = mcu->getIoPort("PORTD");
    m_kbPort  = mcu->getIoPort("PORTK");

    mcu->component()->addProperty( QObject::tr("Main"),
new StrProp <ULA_ZX48k>( "Type"  , QObject::tr("Type")  , "", this, &ULA_ZX48k::type     , &ULA_ZX48k::setType,0,"enum" ) );

    mcu->component()->addProperty( QObject::tr("Main"),
new BoolProp<ULA_ZX48k>( "Screen", QObject::tr("Screen"), "", this, &ULA_ZX48k::isScreen , &ULA_ZX48k::setScreen, 0 ) );
}
ULA_ZX48k::~ULA_ZX48k(){ }

int ULA_ZX48k::getCpuReg( QString reg ) // Called by Mcu Monitor to get Integer values
{
    int value = -1;
    if     ( reg == "Horizontal Counter") value = m_C;
    else if( reg == "Vertical Counter" ) value = m_V;
    else if( reg == "Border Colour"  ) value = m_borderColour;
    else if( reg == "Data Latch"     ) value = m_dataLatch;
    else if( reg == "Atr. Data Latch") value = m_attrDataLatch;
    else if( reg == "Shift Register" ) value = m_shiftReg;
    return value;
}

void ULA_ZX48k::setType( QString type ) // Setter for type
{
    int p = m_enumUids.indexOf( type );
    m_type = (eType)p;

    // When the type of ULA is changed then set default frequency of that type
    if( m_type == ula6c011e ) m_mcu->setFreq( 14110000 ); // ULA with NTSC video standard - clock 14.11 MHz
    else                      m_mcu->setFreq( 14000000 ); // ULA with PAL  video standard - clock 14 MHz
}

void ULA_ZX48k::setScreen( bool s )
{
    if( m_isSrceen == s ) return;
    m_isSrceen = s;

    if( s ) m_display->setSize( 320, 240 ); // setOffset( 32+32, 60+2 );
    else    m_display->setSize( 448, 312 ); // setOffset( 0, 0 );
}

void ULA_ZX48k::updateStep()
{
    if( !m_display ) return;

    if( m_isSrceen ){                     // 320x240 screen, no beam
        for( int x=0; x<448; x++ )
        {
            int sx = x + 19;
            if( sx >= 448 ) sx -= 448;
            if( sx >= 320 ) continue;
            for( int y=0; y<312; y++ )
            {
                int sy = y + 23;
                if( sy >= 312 ) sy -= 312;
                if( sy >= 240 ) continue;
                m_display->setPixel( sx, sy, m_colours[ m_screen[x][y] & 0x0f ] );
            }
        }
    }else{                               // Video memory with beam
        for( int x=0; x<448; x++ )
            for( int y=0; y<312; y++ )
                m_display->setPixel( x, y, m_colours[ m_screen[x][y] & 0x0f ] );

        if( m_C > 1 && m_V > 1 )     m_display->setPixel( m_C-1, m_V-1, Qt::black );
        if( m_C > 1 )                m_display->setPixel( m_C-1, m_V  , Qt::black );
        if( m_C > 1 && m_V < 311 )   m_display->setPixel( m_C-1, m_V+1, Qt::black );
        if( m_V > 1 )                m_display->setPixel( m_C  , m_V-1, Qt::black );
        if( m_V < 311 )              m_display->setPixel( m_C  , m_V+1, Qt::black );
        if( m_C < 447 && m_V > 1 )   m_display->setPixel( m_C+1, m_V-1, Qt::black );
        if( m_C < 447 )              m_display->setPixel( m_C+1, m_V  , Qt::black );
        if( m_C < 447 && m_V < 311 ) m_display->setPixel( m_C+1, m_V+1, Qt::black );
    }
}

void ULA_ZX48k::stamp()
{
    // Initialization clock signals and counters
    m_clk7 = false;
    m_C = 0;
    m_V = 0;
    m_flashClock = 0;
    m_tclka = false;
    m_tclkb = false;

    m_int = false;
    m_border = false;
    m_vidRas = false;
    m_vidCas = false;
    m_cpuCas = false;
    m_aeDelayed = false;
    m_dmaDelayed = 0xff;

    m_ram16  = false;
    m_rasEn  = false;
    m_dramWe = false;
    m_romcsn = false;
    m_portRd = false;
    m_portWr = false;

    m_borderColour = 0;
    m_evenScanLine = false;

    m_a14Pin->setPinMode( input );
    m_a15Pin->setPinMode( input );

    m_mreqPin->setPinMode( input );
    m_iorqPin->setPinMode( input );
    m_rdPin->setPinMode( input );
    m_wrPin->setPinMode( input );
    
    m_rasPin->setPinMode( output );
    m_casPin->setPinMode( output );
    m_dramwePin->setPinMode( output );
    m_romcsPin->setPinMode( output );
    m_intPin->setPinMode( output );
    m_phicpuPin->setPinMode( output );

    m_uPin->setPinMode( output );
    m_vPin->setPinMode( output );
    m_yPin->setPinMode( output );

    m_micTapePin->setPinMode( input );
    m_micTapePin->setInputHighV( 0.714 );
    m_micTapePin->setInputLowV( 0.714 );
    
    m_dmaPort->setPinMode( input );
    m_dPort->setPinMode( input );
    m_kbPort->setPinMode( input );
}

void ULA_ZX48k::runStep()  // Internal Clock signal
{
    m_mcu->cyclesDone = 1;
    m_clk7 = !m_clk7;
    if( m_clk7 ) clk7RisingEdge();
    else         clk7FallingEdge();
}

void ULA_ZX48k::runEvent()
{
    m_dmaPort->setPinMode( m_aeDelayed ? output : input ); // Delayed activation/deactivation DMA bus
}

void ULA_ZX48k::clk7RisingEdge()
{
    bool vidCas = vidCasSignal();        // Video CAS (Figure 13-5 and 14-2)
    if( vidCas != m_vidCas ) {
        m_vidCas = vidCas;
        uint64_t delay;
        if( m_type == ula6c001e7 || m_type == ula6c011e )
             delay = 58000;                                      // Delay of video CAS for ULA6C001e-7
        else delay = 30000;                                      // Delay of video CAS for ULA6C001e-6
        m_casPin->scheduleState( !m_cpuCas && !m_vidCas, delay );
    }
}

void ULA_ZX48k::clk7FallingEdge()
{
    bool a14   = m_a14Pin->getInpState();
    bool a15   = m_a15Pin->getInpState();
    bool mreqn = m_mreqPin->getInpState();
    bool iorqn = m_iorqPin->getInpState();
    bool rdn   = m_rdPin->getInpState();
    bool wrn   = m_wrPin->getInpState();

    bool tclka = iorqn || mreqn ||  rdn || !wrn;            // Test signal TCLKA (Figure 23-1)
    if( tclka != m_tclka ) {
        m_tclka = tclka;
        if( !m_tclka && ( m_C & 0x020 ) == 0x020 ) m_C += 63; // Upper Counter Stage Test Clock (Figure 10-3 and Figure 23-1)
    }
    bool tclkb = iorqn || mreqn || !rdn ||  wrn;            // Test signal TCLKB (Figure 23-1)
    if( tclkb != m_tclkb ) {
        m_tclkb = tclkb;
        if( !m_tclkb && ( m_V & 0x100 ) == 0x100 ) m_flashClock++; // Upper Counter Stage Test Clock (Figure 10-3 and Figure 23-1)
        m_kbPort->getPinN(0)->setOutState( m_tclkb || ( m_V & 0x100 ) == 0x000 ); // V8 output at K0 pin (Figure 23-1)
    }

    increaseCounters();
    updateVideo();
    readVideoData();
    generatePhicpu( a14, a15, mreqn, iorqn );

    bool interrupt = m_V == 248 && ( m_C & 0x1c0) == 0x000;     // Maskable interrupt (Figure 21-2)
    if( interrupt != m_int ) {
        m_int = interrupt;
        m_intPin->setOutStatFast( !interrupt );
    }

    bool romcsn = a14 || a15;                                   // ROM chip select signal (Figure 17-7)
    if( romcsn != m_romcsn ) {
        m_romcsn = romcsn;
        m_romcsPin->setOutStatFast( m_romcsn );
    }

    /*bool rasEn = mreqn && a14 && a15 && !m_border;             // RAS enable signal (Figure 23-2)
    if( rasEn != m_rasEn ) {
        m_rasEn = rasEn;
        m_rasPin->setStateZ( m_rasEn );
        m_rasPin->setOutStatFast( !m_ram16 && !m_vidRas );
    }*/

    bool ram16 = !mreqn && a14 && !a15;                       // Video ram select signal (Figure 17-7)
    if( ram16 != m_ram16 ) { // Delay of RAS after MREQ is 47 ns during activation and 34 ns during deactivation
        m_ram16 = ram16;
        m_rasPin->scheduleState( !m_ram16 && !m_vidRas, ram16 ? 47000 : 34000 );
    }
    // Delay of CAS after RD is 83 ns during activation and 53 ns during deactivation for ULA6C001e-7
    // Delay of CAS after WR is 94 ns during activation and 91 ns during deactivation for ULA6C001e-7
    // Delay is shorter about 33 ns for ULA6C001e-6 and older
    bool cpuCas = ram16 && ( !rdn || !wrn );                            // CPU CAS (Figure 17-7)
    if ( cpuCas != m_cpuCas ) {
        m_cpuCas = cpuCas;
        uint64_t delay;
        if( m_type == ula6c001e7 || m_type == ula6c011e )
             delay = cpuCas ? 83000 : 53000;
        else delay = cpuCas ? 50000 : 20000;
        m_casPin->scheduleState( !m_cpuCas && !m_vidCas, delay );
    }

    bool dramWe = ram16 && !wrn;
    if( dramWe != m_dramWe) {
        m_dramWe = dramWe;   // Delay of DRAMWE after WR is 31 ns during activation and 21 ns during deactivation
        m_dramwePin->scheduleState( !dramWe, dramWe ? 31000 : 21000 );
    }

    portIO( iorqn, rdn, wrn );
}

void ULA_ZX48k::portIO( bool iorqn, bool rdn, bool wrn )
{
    bool portOp = iorqn | !m_iorqTW3 | m_dmaPort->getPinN(0)->getInpState(); // Operation in I/O port ??

    bool portRd = portOp | rdn ;                        // Read from I/O port (Figure 19-7)
    if( portRd != m_portRd ){
        m_portRd = portRd;
        if( portRd ) m_dPort->setPinMode( input );
        else {
            m_dPort->setPinMode( openCo );
            uint8_t state = m_kbPort->getInpState();          // Copy keyboard signal to data bus (Figure 19-3)
            if( m_micTapePin->getInpState() ) state |= 1<<6;  // Copy MIC/TAPE input  to data bus (Figure 19-5)
            m_dPort->setOutState( state );
        }
    }
    bool portWr = portOp | wrn;                         // Write to I/O port (Figure 19-7)
    if( portWr != m_portWr ) {
        m_portWr = portWr;
        if( !portWr ){
            uint8_t dState = m_dPort->getInpState();
            m_borderColour = dState & 0x07;             // Write value from data bus to border colour register (Figure 19-4)

            uint8_t index = (dState & 0b00011000) >> 3;
            if( m_type == ula5c102e || m_type == ula5c112e )
                 m_micTapePin->setVoltage( m_micSp5CTable[ index ] );    // Set MIC/TAPE output (Figure 19-6)
            else m_micTapePin->setVoltage( m_micSp6CTable[ index ] );    // Set MIC/TAPE output (Figure 19-6)

            m_micTapePin->setImp( m_micSpImpTable[ index ] );
        }
    }
}

bool ULA_ZX48k::vidCasSignal()
{
    return !m_border && m_C & 0x008 && ( ( m_C & 0x001 ) != 0x001 || !m_clk7 );  // Video CAS (Figure 13-5 and 14-2)
}

void ULA_ZX48k::increaseCounters()
{
    m_C++;                                                      // Master counter increase (Figure 10-3)
    if( m_C >= 448 ) {
        m_C -= 448;
        m_V++;                                                  // Vertical line counter increase (Figure 10-4 and Figure 10-5)
        if( m_V >= ( ( m_type == ula6c011e ) ? 264 : 312 ) ) {  // 312 scan lines for PAL and 264 scan lines for NTSC (Table 11-2 and Table 11-3)
            m_V = 0;
            m_flashClock++;                                     // Flash clock increase (Figure 14-3)
        }
    }
}

void ULA_ZX48k::updateVideo()
{
    if( ( m_C & 0x007 ) == 0x005 ){                                     // Load attribute output latch and shift register when video is enabled (Figure 14-2)
        bool videoEn = m_C >= 8 && m_C < 264 && m_V < 192;              // Video Enable signal (Figure 14-2), calculated differently
        if( videoEn ) {                                                 // Border or attribute multiplexer (Figure 12-6 and Figure 12-8)
            m_attrOutLatch = m_attrDataLatch;                           // Load attribute output latch from attribute data latch (Figure 12-6 and Figure 12-10)
            m_shiftReg     = m_dataLatch;                               // Load shift register (Figure 12-2 and Figure 12-7)

            if( m_attrOutLatch & 0x80 && m_flashClock & 0x10 )          // Invert pixel stream when flash is enabled (Figure 12-9)
                m_shiftReg ^= 0xff;                                     // The whole pixel stream is inverted instead of inverting pixel by pixel (Figure 12-9)
        }
        else m_attrOutLatch = ( m_borderColour << 3 );                  // Load attribute output latch from border register (Figure 12-6)
    }
    bool hBlank = m_C >=320 && m_C <= 415;                                   // Blanking period (Figure 11-5, Table 11-1 and Table 16-4)
    bool hSync;
    if( m_type == ula5c102e || m_type == ula5c112e ) {                  // Horizontal sync (Figure 11-5, Table 11-1 and Table 16-4)
        hSync = m_C >= 336 && m_C <= 367;
        if( m_C == 336 ) m_evenScanLine = m_V & 0x001;                  // Even scan line (Figure 16-12)
    } else {
        hSync = m_C >= 344 && m_C <= 375;
        if( m_C == 344 ) m_evenScanLine = m_V & 0x001;
    }
    bool burst = m_C >= 384 && m_C <= 399;                                   // Colour burst (Table 16-4);
    bool vSync;
    if( m_type == ula6c011e ) vSync = m_V >= 216 && m_V <= 219;         // Vertical sync NTSC (Figure 11-6 and Table 11-3)
    else                      vSync = m_V >= 248 && m_V <= 251 ;        // Vertical sync PAL (Figure 11-5 and Table 11-2)

    m_border = m_C >= 256 || m_V >= 192;                                // Border (Figure 11-5, Table 11-1 and 11-2)
    uint8_t FlBrGRB;
    if( hBlank || vSync ) FlBrGRB = 0;                                  // Blanking video signals (Figure 12-10)
    else FlBrGRB = ( m_shiftReg & 0x80 ) ? ( m_attrOutLatch & 0x07 ) : ( ( m_attrOutLatch & 0x38 ) >> 3 );// Colour Ink or Paper (Figure 12-6 and Figure 12-10)

    FlBrGRB |= ( m_attrOutLatch & 0xc0 ) >> 3;                          // Add bright and flash to video signals (Figure 12-10)
    m_shiftReg <<= 1;                                                   // Shift shift register one bit left (Figure 12-2 and Figure 12-7)
    m_screen[m_C][m_V] = FlBrGRB;                                       // Store pixel color into screen

    m_yPin->setVoltage( ( hSync || vSync ) ? 4.3 : m_yTable[FlBrGRB & 0x0f] ); // Set luminance output Y (Table 16-1)
    if( m_type == ula6c011e ) {
        m_uPin->setVoltage( burst ? 2.926 : m_uTable[FlBrGRB & 0x07] );        // Set NTSC chrominance output U (Table 16-2)
        m_vPin->setVoltage( m_vTable[FlBrGRB & 0x07] );                        // Set NTSC chrominance output V (Table 16-3)
    } else {
        m_uPin->setVoltage( burst ? 2.926 : m_uTable[FlBrGRB & 0x07] );        // Set PAL chrominance output U (Table 16-2)
        if( m_evenScanLine )
            m_vPin->setVoltage( burst ? 0.966 : m_vTable[FlBrGRB & 0x07] );    // Set PAL chrominance output V (Table 16-3)
        else
            m_vPin->setVoltage( burst ? 3.112 : m_vInvTable[FlBrGRB & 0x07] ); // Set PAL inverted chrominance output V (Table 16-3)
    }
}

void ULA_ZX48k::readVideoData()
{
    bool ae = !m_border && ( m_C & 0x00f ) >= 0x007;                                 // Address enable (Figure 15-6)
    if( ae ) {
        uint8_t dma = m_dmaDelayed;

        switch( m_C & 0x00f ) {                                                                           // Select address to DMA bus (Figure 15-6 and Figure 15-8)
            case 0x007 : dma = ( m_C & 0x0f8 ) >> 3 | ( m_V & 0x018 ) << 2;                        break; // Row address (Figure 15-6 and Figure 15-7)
            case 0x008 :
            case 0x009 : dma = ( m_V & 0x0c0 ) >> 2 | ( m_V & 0x007 ) << 1 | ( m_V & 0x020 ) >> 5; break; // Display column address (Figure 15-6 and Figure 15-7)
            case 0x00a : dma = 0x30 | ( m_V & 0x0e0 ) >> 5;                                        break; // Attribute column address (Figure 15-6 and Figure 15-7)
            case 0x00b : dma = ( m_C & 0x0f8 ) >> 3 | ( m_V & 0x018 ) << 2;                        break; // Row address (Figure 15-6 and Figure 15-7)
            case 0x00c :
            case 0x00d : dma = ( m_V & 0x0c0 ) >> 2 | ( m_V & 0x007 ) << 1 | ( m_V & 0x020 ) >> 5; break; // Display column address (Figure 15-6 and Figure 15-7)
            case 0x00e :
            case 0x00f : dma = 0x30 | ( m_V & 0x0e0 ) >> 5;                                        break; // Attribute column address (Figure 15-6 and Figure 15-7)
            default    : break;
        }
        if( dma != m_dmaDelayed ) { // Delay of setting DMA bus is 20 ns
            m_dmaDelayed = dma;
            m_dmaPort->scheduleState( m_dmaDelayed, 20000 );
        }
    }
    
    if( ae != m_aeDelayed ) { // Delay of enabling and disabling DMA bus is 20 ns
        m_aeDelayed = ae;
        Simulator::self()->addEvent( 20000, this );
    }

    bool vidRas = !m_border && m_C & 0x008 && ( m_C & 0x003 ) != 0x003;   // Video RAS (Figure 13-5 and 14-2)
    if( vidRas != m_vidRas ) {
        m_vidRas = vidRas;
        m_rasPin->setOutStatFast( !m_ram16 && !m_vidRas ); // RAS signal without any delay
    }

    bool vidCas = vidCasSignal();                                         // Video CAS (Figure 13-5 and 14-2)
    if( vidCas != m_vidCas ) {
        m_vidCas = vidCas;
        uint64_t delay;
        if( ( m_type == ula6c001e7 ) || ( m_type == ula6c011e) )
             delay = ( m_C & 0x002 ) ? 58000 : 78000;     // Delay of first video CAS activation is 78 ns and delay of the rest of edges is 53 ns for ULA6C001e-7
        else delay = ( m_C & 0x002 ) ? 30000 : 50000;     // Delay of first video CAS activation is 50 ns and delay of the rest of edges is 30 ns for ULA6C001e-6
        m_casPin->scheduleState( !m_cpuCas && !m_vidCas, delay );
    }

    if( !m_border && m_C & 0x008 && m_C & 0x001 ){
        if( m_C & 0x002 ) m_attrDataLatch = m_dPort->getInpState();  // Load attribute data latch (Figure 12-6, Figure 12-8 and Figure 14-2)
        else              m_dataLatch     = m_dPort->getInpState();  // Load data latch (Figure 12-2, Figure 12-7 and Figure 14-2)
    }
}

void ULA_ZX48k::generatePhicpu( bool a14, bool a15, bool mreqn, bool iorqn )
{
    bool memContention;
    bool ioContention;

    switch( m_type ) {
        case ula5c102e :
            memContention = a14 && !a15 && ( m_C & 0x00c) != 0x000 && !m_border && !m_mreqT23 && !m_iorqTW3 && m_cpuClk;           // Memory contention signal (Figure 18-7)
            ioContention = ( m_C & 0x00c) != 0x000 && !m_border && !m_cpuClk && !m_iorqTW3 && ( m_C & 0x00e) != 0x008 && !iorqn;       // I/O contention signal (Figure 18-7)
            break;
        case ula5c112e :
            memContention = ( !iorqn || ( a14 && !a15 ) ) && ( m_C & 0x00c) != 0x000 && !m_border && !m_mreqT23 && !m_iorqTW3 && m_cpuClk; // Memory contention signal (Figure 18-12)
            ioContention = ( m_C & 0x00c) != 0x000 && !m_border && !m_cpuClk && !m_iorqTW3 && ( m_C & 0x00e) != 0x008 && !iorqn;       // I/O contention signal (Figure 18-12)
            break;
        default :
            memContention = ( !iorqn || ( a14 && !a15 ) ) && ( m_C & 0x00c) != 0x000 && !m_border && !m_mreqT23 && !m_iorqTW3 && m_cpuClk; // Memory contention signal (Figure 18-15)
            ioContention = ( m_C & 0x00c) != 0x000 && !m_border && !m_cpuClk && !m_iorqTW3 && !iorqn;                                  // I/O contention signal (Figure 18-15)
            break;
    }
    
    if( !memContention && !ioContention ){
        m_cpuClk = m_C & 0x001;
        m_phicpuPin->setOutStatFast( !m_cpuClk );
        if( ( m_C & 0x001 ) == 0x001 ) {
            m_mreqT23 = !mreqn;
            m_iorqTW3 = !iorqn;
        }
    }
}
