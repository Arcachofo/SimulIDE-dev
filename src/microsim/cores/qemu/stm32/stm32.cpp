/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>
#include <QPainter>
#include <QFileInfo>

#include "stm32.h"
//#include "ustm32pin.h"
#include "stm32port.h"
#include "stm32usart.h"
#include "stm32twi.h"
#include "stm32spi.h"
#include "stm32timer.h"

#define tr(str) simulideTr("Stm32",str)

#define IOMEM_BASE 0x40000000
#define IOMEM_SIZE 0x00023400
#define GPIOA_BASE 0x40010800 // Port A
#define GPIO_SIZE  0x00000400


Stm32::Stm32( QString type, QString id, QString device )
     : QemuDevice( type, id )
{
    m_area = QRect(-32,-32, 64, 64 );

    m_device = device;
    QString model = device.right( 3 );

    QStringList pkgs = { "T", "C", "R", "V", "Z" };
    uint32_t pkg = pkgs.indexOf( model.mid(1,1) );
    switch( pkg ){
        case 0: m_packageFile = "stm32_dip36.package";  break; // T
        case 1: m_packageFile = "stm32_dip48.package";  break; // C
        case 2: m_packageFile = "stm32_dip64.package";  break; // R
        case 3: m_packageFile = "stm32_dip100.package"; break; // V
        //case 4: m_packageFile = "stm32_dip144.package"; break; // Z

        default: m_packageFile = "stm32_dip64.package";  break;
    }

    QStringList vars = { "4", "6", "8", "B", "C", "D", "E", "F", "G" };
    uint32_t var = vars.indexOf( model.right(1) );

    switch( var )                                              //   Flash/RAM
    {
        case 0: {                                              // 4   16k/6k
            //m_FLASH_SIZE  = 16 * 1024;
            //m_SRAM_SIZE   =  6 * 1024;
            m_portN = 4; m_usartN = 2; m_i2cN = 1; m_spiN = 1;
        } break;
        case 1: {                                              // 6   32k/10k
            //m_FLASH_SIZE  = 32 * 1024;
            //m_SRAM_SIZE   = 10 * 1024;
            m_portN = 4; m_usartN = 2; m_i2cN = 1; m_spiN = 1;
        } break;
        case 2:{                                               // 8   64k/20k
            //m_FLASH_SIZE  = 64 * 1024;
            //m_SRAM_SIZE   = 20 * 1024;
            m_portN = 5; m_usartN = 3; m_i2cN = 2; m_spiN = 2;
        } break;
        case 3: {                                              // B  128k/20k
            //m_FLASH_SIZE  = 128 * 1024;
            //m_SRAM_SIZE   =  20 * 1024;
            m_portN = 5; m_usartN = 3; m_i2cN = 2; m_spiN = 2;
        } break;
        case 4: {                                              // C  256k/48k
            //m_FLASH_SIZE  = 256 * 1024;
            //m_SRAM_SIZE   =  48 * 1024;
            m_portN = 7; m_usartN = 5; m_i2cN = 2; m_spiN = 3;
            } break;
        case 5: {                                              // D  384k/64k
            //m_FLASH_SIZE  = 384 * 1024;
            //m_SRAM_SIZE   =  64 * 1024;
            m_portN = 7; m_usartN = 5; m_i2cN = 2; m_spiN = 3;
        } break;
        case 6: {                                              // E  512k/64k
            //m_FLASH_SIZE  = 512 * 1024;
            //m_SRAM_SIZE   =  64 * 1024;
            m_portN = 7; m_usartN = 5; m_i2cN = 2; m_spiN = 3;
        } break;
        //else if( var == "F" ) m_usartN = 3; m_timerN = 4; m_i2cN = 2; // F  768k/96k
        //else if( var == "G" ) m_usartN = 3; m_timerN = 4; m_i2cN = 2; // G    1M/96k
        default: break;
    }

    uint32_t fam = model.left(1).toInt();

    m_model = fam << 16 | pkg << 8 | var;
    qDebug() << "Stm32::Stm32 model" << device << m_model;
    m_executable = "./data/bin/qemu-system-arm";

    m_firmware ="";

    m_ioMem.resize( IOMEM_SIZE, 0 );
    m_ioMemStart = IOMEM_BASE;

    createPorts();

    m_i2cs.resize( m_i2cN );
    if( m_i2cN > 0 ) m_i2cs[0] = new Stm32Twi( this, "I2C1", 0, &m_apb1, 0x00005400, 0x000057FF );
    if( m_i2cN > 1 ) m_i2cs[1] = new Stm32Twi( this, "I2C2", 1, &m_apb1, 0x00005800, 0x00005BFF );

    m_spis.resize( m_spiN );
    if( m_spiN > 0 ) m_spis[0] = new Stm32Spi( this, "I2C1", 0, &m_apb1, 0x00013000, 0x000133FF );
    if( m_spiN > 1 ) m_spis[1] = new Stm32Spi( this, "I2C2", 1, &m_apb1, 0x00003800, 0x00003BFF );
    if( m_spiN > 2 ) m_spis[2] = new Stm32Spi( this, "I2C3", 2, &m_apb1, 0x00003C00, 0x00003FFF );

    m_usarts.resize( m_usartN );
    if( m_usartN > 0 ) m_usarts[0] = new Stm32Usart( this, "Usart1", 0, &m_apb2, 0x00013800, 0x00013BFF );
    if( m_usartN > 1 ) m_usarts[1] = new Stm32Usart( this, "Usart2", 1, &m_apb1, 0x00004400, 0x000047FF );
    if( m_usartN > 2 ) m_usarts[2] = new Stm32Usart( this, "Usart3", 2, &m_apb1, 0x00004800, 0x00004BFF );
    if( m_usartN > 3 ) m_usarts[3] = new Stm32Usart( this, "Uart4" , 3, &m_apb1, 0x00004C00, 0x00004FFF );
    if( m_usartN > 4 ) m_usarts[4] = new Stm32Usart( this, "Uart5" , 4, &m_apb1, 0x00005000, 0x000053FF );

    m_timerN = 4;
    m_timers.resize( m_timerN );
    m_timers[0] = new Stm32Timer( this, "Timer1", 0, &m_apb2, 0x00012C00, 0x00012FFF );
    m_timers[1] = new Stm32Timer( this, "Timer2", 1, &m_apb2, 0x00000000, 0x000003FF );
    m_timers[2] = new Stm32Timer( this, "Timer3", 2, &m_apb2, 0x00000400, 0x000007FF );
    m_timers[3] = new Stm32Timer( this, "Timer4", 3, &m_apb2, 0x00000800, 0x00000BFF );
    for( int i=0; i<m_timerN; ++i ) timerRemap( i, 0 );

    m_dummyModule = new QemuModule( this, "UnMapped", 0, nullptr, 0, IOMEM_SIZE );
}
Stm32::~Stm32(){}


bool Stm32::createArgs()
{
    //QFileInfo fi = QFileInfo( m_firmware );

    /*if( fi.size() != 1048576 )
    {
        qDebug() << "Error firmware file size:" << fi.size() << "must be 1048576";
        return false;
    }*/
    m_arena->regData = m_model;

    m_arguments.clear();

    m_arguments << m_shMemKey;          // Shared Memory key

    m_arguments << "qemu-system-arm";

    QStringList extraArgs = m_extraArgs.split(",");
    for( QString arg : extraArgs )
    {
        if( arg.isEmpty() ) continue;
        m_arguments << arg;
    }

    //m_arguments << "-d";
    //m_arguments << "in_asm";

    //m_arguments << "-machine";
    //m_arguments << "help";

    m_arguments << "-M";
    m_arguments << "stm32-f10xx";

    m_arguments << "-drive";
    m_arguments << "file="+m_firmPath+",if=pflash,format=raw";

    //m_arguments << "-kernel";  // Does not work
    //m_arguments <<  m_firmPath ;

    //m_arguments << "-accel";
    //m_arguments << "tcg,tb-size=100";

    //m_arguments << "-rtc";
    //m_arguments <<"clock=vm";

    m_arguments << "-icount";
    m_arguments <<"shift=0,align=off,sleep=off";

    return true;
}

void Stm32::stamp()
{
    if( m_i2cN > 0 ) m_i2cs[0]->setPins( m_ports[1]->getPin(6) , m_ports[1]->getPin(7)  );
    if( m_i2cN > 1 ) m_i2cs[1]->setPins( m_ports[1]->getPin(10), m_ports[1]->getPin(11) );

    if( m_spiN > 0 ) m_spis[0]->setPins( m_ports[0]->getPin(7) , m_ports[0]->getPin(6) , m_ports[0]->getPin(5) , m_ports[0]->getPin(4)  );
    if( m_spiN > 1 ) m_spis[1]->setPins( m_ports[1]->getPin(15), m_ports[1]->getPin(14), m_ports[1]->getPin(13), m_ports[1]->getPin(12) );
    if( m_spiN > 2 ) m_spis[2]->setPins( m_ports[1]->getPin(5) , m_ports[1]->getPin(4) , m_ports[1]->getPin(3) , m_ports[0]->getPin(15) );

    if( m_usartN > 0 ) m_usarts[0]->setPins({m_ports[0]->getPin(9) , m_ports[0]->getPin(10)}); // No Remap (TX/PB6, RX/PB7)
    if( m_usartN > 1 ) m_usarts[1]->setPins({m_ports[0]->getPin(2) , m_ports[0]->getPin(3) }); // No remap (CTS/PA0, RTS/PA1, TX/PA2, RX/PA3, CK/PA4), Remap (CTS/PD3, RTS/PD4, TX/PD5, RX/PD6, CK/PD7)
    if( m_usartN > 2 ) m_usarts[2]->setPins({m_ports[1]->getPin(10), m_ports[1]->getPin(11)});
    if( m_usartN > 3 ) m_usarts[3]->setPins({m_ports[2]->getPin(10), m_ports[2]->getPin(11)});
    if( m_usartN > 4 ) m_usarts[4]->setPins({m_ports[2]->getPin(12), m_ports[3]->getPin(2) });

    QemuDevice::stamp();
}

void Stm32::createPorts()
{
    m_ports.resize( m_portN );

    uint64_t start = GPIOA_BASE-IOMEM_BASE;
    uint64_t end   = start + GPIO_SIZE -1;

    for( int p=0; p<m_portN; ++p )
    {
        QString pId = QChar('A'+p);
        Stm32Port* port = new Stm32Port( this, "Port"+pId, p, &m_apb2, start, end );
        m_ports[p] = port;

        start += GPIO_SIZE;
        end   += GPIO_SIZE;

        //createPort( &m_ports[i], i+1, QString('A'+i), 16 );
        for( int i=0; i<16; ++i )
        {
            //qDebug() << "Stm32::createPort" << m_id+"-P"+pId+QString::number(i);
            Stm32Pin* pin = port->createPin( i, m_id+"-P"+pId+QString::number(i), this );
            pin->setVisible( false );
        }
    }

    setPackageFile("./data/STM32/"+m_packageFile);
    Chip::setName( m_device );
}

void Stm32::updtFrequency()
{
    m_apb1 = m_arena->regAddr;
    m_apb2 = m_arena->regData;

    for( QemuModule* module : m_modules ) module->freqChanged();
}

Pin* Stm32::addPin( QString id, QString type, QString label,
                   int n, int x, int y, int angle, int length, int space )
{
    IoPin* pin = nullptr;
    //qDebug() << "Stm32::addPin" << id;
    if( type.contains("rst") )
    {
        pin = new IoPin( angle, QPoint(x, y), m_id+"-"+id, n-1, this, input );
        m_rstPin = pin;
        m_rstPin->setOutHighV( 3.3 );
        m_rstPin->setPullup( 1e5 );
        m_rstPin->setInputHighV( 0.65 );
        m_rstPin->setInputLowV( 0.65 );
    }
    else{
        //qDebug() << "Stm32::addPin"<<id;
        uint n = id.right(2).toInt();
        QString portStr = id.at(1);
        Stm32Port* port = nullptr;
        if     ( portStr == "A") port = m_ports.at(0);
        else if( portStr == "B") port = m_ports.at(1);
        else if( portStr == "C") port = m_ports.at(2);
        else if( portStr == "D") port = m_ports.at(3);
        else if( portStr == "E") port = m_ports.at(4);

        if( !port ) return nullptr;

        if( n >= port->size() ) return nullptr;

        pin = port->getPin( n );
        if( !pin ) return nullptr;

        pin->setPos( x, y );
        pin->setPinAngle( angle );
        pin->setVisible( true );
    }
    QColor color = Qt::black;
    if( !m_isLS ) color = QColor( 250, 250, 200 );

    //if( type.startsWith("inv") ) pin->setInverted( true );

    pin->setLength( length );
    pin->setSpace( space );
    pin->setLabelText( label );
    pin->setLabelColor( color );
    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, true );
    return pin;
}

void Stm32::timerRemap( int number, uint8_t value )
{
    //printf("stm32_timer_remap %i %i\n", number, value ); fflush( stdout );
    if( number >= m_timerN ) return;

    Stm32Timer* timer = (Stm32Timer*)m_timers[number];
    if( !timer ) return;

    switch( number )
    {
    case 0:{
        switch ( value ) {
        case 0:{       //00: No remap (ETR/PA12, CH1/PA8, CH2/PA9, CH3/PA10, CH4/PA11, BKIN/PB12, CH1N/PB13, CH2N/PB14, CH3N/PB15)
            timer->setOcPins( m_ports[0]->getPin(8), m_ports[0]->getPin(9), m_ports[0]->getPin(10), m_ports[0]->getPin(11) );
        }break;
        case 1:{       //01: Partial remap (ETR/PA12, CH1/PA8, CH2/PA9, CH3/PA10, CH4/PA11, BKIN/PA6, CH1N/PA7, CH2N/PB0, CH3N/PB1)
            timer->setOcPins( m_ports[1]->getPin(8), m_ports[1]->getPin(9), m_ports[1]->getPin(10), m_ports[1]->getPin(11) );
        }break;
        case 2: break; //10: not used
        case 3:{       //11: Full remap (ETR/PE7, CH1/PE9, CH2/PE11, CH3/PE13, CH4/PE14, BKIN/PE15, CH1N/PE8, CH2N/PE10, CH3N/PE12)
            timer->setOcPins( m_ports[5]->getPin(9), m_ports[5]->getPin(11), m_ports[5]->getPin(13), m_ports[5]->getPin(14) );
        }break;
        }
    }break;
    case 1:{
        switch ( value ) {
        case 0:{       //00: No remap (CH1/ETR/PA0, CH2/PA1, CH3/PA2, CH4/PA3)
            timer->setOcPins( m_ports[0]->getPin(0), m_ports[0]->getPin(1), m_ports[0]->getPin(2), m_ports[0]->getPin(3) );
        }break;
        case 1:{       //01: Partial remap (CH1/ETR/PA15, CH2/PB3, CH3/PA2, CH4/PA3)
            timer->setOcPins( m_ports[0]->getPin(15), m_ports[1]->getPin(3), m_ports[0]->getPin(2), m_ports[0]->getPin(3) );
        }break;
        case 2:{       //10: Partial remap (CH1/ETR/PA0, CH2/PA1, CH3/PB10, CH4/PB11)
            timer->setOcPins( m_ports[0]->getPin(0), m_ports[0]->getPin(1), m_ports[1]->getPin(10), m_ports[1]->getPin(11) );
        }break;
        case 3:{       //11: Full remap (CH1/ETR/PA15, CH2/PB3, CH3/PB10, CH4/PB11)
            timer->setOcPins( m_ports[0]->getPin(15), m_ports[1]->getPin(3), m_ports[1]->getPin(10), m_ports[1]->getPin(11) );
        }break;
        }
    }break;
    case 2:{
        switch ( value ) {
        case 0:{       //00: No remap (CH1/PA6, CH2/PA7, CH3/PB0, CH4/PB1)
            timer->setOcPins( m_ports[0]->getPin(6), m_ports[0]->getPin(7), m_ports[1]->getPin(0), m_ports[1]->getPin(1) );
        }break;
        case 1: break; //01: Not used
        case 2:{       //10: Partial remap (CH1/PB4, CH2/PB5, CH3/PB0, CH4/PB1)
            timer->setOcPins( m_ports[1]->getPin(4), m_ports[1]->getPin(5), m_ports[1]->getPin(0), m_ports[1]->getPin(1) );
        }break;
        case 3:{       //11: Full remap (CH1/PC6, CH2/PC7, CH3/PC8, CH4/PC9)
            timer->setOcPins( m_ports[2]->getPin(6), m_ports[2]->getPin(7), m_ports[2]->getPin(8), m_ports[2]->getPin(9) );
        }break;
        }
    }break;
    case 3:{
        switch ( value ) {
        case 0:{       // No remap (CH1/PB6, CH2/PB7, CH3/PB8, CH4/PB9)
            timer->setOcPins( m_ports[1]->getPin(6), m_ports[1]->getPin(7), m_ports[1]->getPin(8), m_ports[1]->getPin(9) );
        }break;
        case 1:{       // Full remap (CH1/PD12, CH2/PD13, CH3/PD14, CH4/PD15)
            timer->setOcPins( m_ports[3]->getPin(12), m_ports[3]->getPin(13), m_ports[3]->getPin(14), m_ports[3]->getPin(15) );
        }break;
        }
    }break;
    }
}
