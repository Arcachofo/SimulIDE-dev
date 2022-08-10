/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/
 
#include "itemlibrary.h"
#include "circuit.h"
#include "appiface.h"

//BEGIN Item includes
#include "ampmeter.h"
#include "adc.h"
#include "aip31068_i2c.h"
#include "audio_out.h"
#include "battery.h"
#include "bcdto7s.h"
#include "bcdtodec.h"
#include "bincounter.h"
#include "bjt.h"
#include "buffer.h"
#include "bus.h"
#include "capacitor.h"
#include "clock.h"
#include "currsource.h"
#include "dac.h"
#include "dcmotor.h"
#include "dectobcd.h"
#include "demux.h"
#include "dht22.h"
#include "diac.h"
#include "diode.h"
#include "ds1307.h"
#include "ds1621.h"
#include "ds18b20.h"
#include "elcapacitor.h"
#include "ellipse.h"
#include "esp01.h"
#include "fixedvolt.h"
#include "flipflopd.h"
#include "flipflopjk.h"
#include "flipfloprs.h"
#include "flipflopt.h"
#include "freqmeter.h"
#include "fulladder.h"
#include "function.h"
#include "gate_and.h"
#include "gate_or.h"
#include "gate_xor.h"
#include "ground.h"
#include "header.h"
#include "hd44780.h"
#include "ili9341.h"
#include "image.h"
#include "i2cram.h"
#include "i2ctoparallel.h"
#include "inductor.h"
#include "keypad.h"
#include "ky023.h"
#include "ky040.h"
#include "ks0108.h"
#include "lamp.h"
#include "latchd.h"
#include "ldr.h"
#include "led.h"
#include "ledbar.h"
#include "ledmatrix.h"
#include "ledrgb.h"
#include "line.h"
#include "lm555.h"
#include "logicanalizer.h"
#include "max72xx_matrix.h"
#include "mcu.h"
#include "memory.h"
#include "mosfet.h"
#include "mux.h"
#include "mux_analog.h"
#include "op_amp.h"
#include "oscope.h"
#include "pcd8544.h"
#include "probe.h"
#include "potentiometer.h"
#include "push.h"
#include "rail.h"
#include "rectangle.h"
#include "relay.h"
#include "resistor.h"
#include "resistordip.h"
#include "rtd.h"
#include "scripted.h"
#include "serialport.h"
#include "servo.h"
#include "sevensegment.h"
#include "sevensegment_bcd.h"
#include "shiftreg.h"
#include "socket.h"
#include "sr04.h"
#include "scr.h"
#include "ssd1306.h"
#include "stepper.h"
#include "strain.h"
#include "subcircuit.h"
#include "subpackage.h"
#include "switch.h"
#include "switchdip.h"
#include "thermistor.h"
#include "textcomponent.h"
#include "touchpad.h"
#include "triac.h"
#include "tunnel.h"
#include "varresistor.h"
#include "voltmeter.h"
#include "volt_reg.h"
#include "voltsource.h"
#include "wavegen.h"
#include "ws2812.h"
#include "zener.h"

#include "z80cpu.h"
//END Item includes

ItemLibrary* ItemLibrary::m_pSelf = NULL;

ItemLibrary::ItemLibrary()
{
    //Q_UNUSED( categories );
    m_pSelf = this;

    loadItems();
    //loadPlugins();
}
ItemLibrary::~ItemLibrary()
{
    for( LibraryItem* item : m_items ) delete item;
}

void ItemLibrary::loadItems()
{
    m_items.clear();
    // Meters
    addItem( Probe::libraryItem() );
    addItem( Voltimeter::libraryItem() );
    addItem( Amperimeter::libraryItem() );
    addItem( FreqMeter::libraryItem() );
    addItem( Oscope::libraryItem() );
    addItem( LAnalizer::libraryItem() );
    // Sources
    addItem( FixedVolt::libraryItem() );
    addItem( Clock::libraryItem() );
    addItem( WaveGen::libraryItem() );
    addItem( VoltSource::libraryItem() );
    addItem( CurrSource::libraryItem() );
    addItem( Battery::libraryItem() );
    addItem( Rail::libraryItem() );
    addItem( Ground::libraryItem() );
    // Switches
    addItem( Push::libraryItem() );
    addItem( Switch::libraryItem() );
    addItem( SwitchDip::libraryItem() );
    addItem( Relay::libraryItem() );
    addItem( KeyPad::libraryItem() );
    // Passive
    addItem( new LibraryItem( QObject::tr("Resistors"), QObject::tr("Passive"), "resistors.png","", NULL ) );
    addItem( Resistor::libraryItem() );
    addItem( ResistorDip::libraryItem() );
    addItem( Potentiometer::libraryItem() );
    addItem( VarResistor::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Resistive Sensors"), QObject::tr("Passive"), "resistorsensors.png","", NULL ) );
    addItem( Ldr::libraryItem() );
    addItem( Thermistor::libraryItem() );
    addItem( RTD::libraryItem() );
    addItem( Strain::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Reactive"), QObject::tr("Passive"), "reactive.png","", NULL ) );
    addItem( Capacitor::libraryItem() );
    addItem( elCapacitor::libraryItem() );
    addItem( Inductor::libraryItem() );
    // Active
    addItem( new LibraryItem( QObject::tr("Rectifiers"), QObject::tr("Active"), "rectifiers.png","", NULL ) );
    addItem( Diode::libraryItem() );
    addItem( Zener::libraryItem() );
    addItem( SCR::libraryItem() );
    addItem( Diac::libraryItem() );
    addItem( Triac::libraryItem() );
    addItem( Mosfet::libraryItem() );
    addItem( BJT::libraryItem() );
    addItem( OpAmp::libraryItem() );
    addItem( VoltReg::libraryItem() );
    addItem( MuxAnalog::libraryItem() );
    // Outputs
    addItem( new LibraryItem( QObject::tr("Leds"), QObject::tr("Outputs"), "leds.png","", NULL ) );
    addItem( Led::libraryItem() );
    addItem( LedRgb::libraryItem() );
    addItem( LedBar::libraryItem() );
    addItem( SevenSegment::libraryItem() );
    addItem( LedMatrix::libraryItem() );
    addItem( Max72xx_matrix::libraryItem() );
    addItem( WS2812::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Displays"), QObject::tr("Outputs"), "displays.png","", NULL ) );
    addItem( Hd44780::libraryItem() );
    addItem( Aip31068_i2c::libraryItem() );
    addItem( Pcd8544::libraryItem() );
    addItem( Ks0108::libraryItem() );
    addItem( Ssd1306::libraryItem() );
    addItem( Ili9341::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Motors"), QObject::tr("Outputs"), "motors.png","", NULL ) );
    addItem( DcMotor::libraryItem() );
    addItem( Stepper::libraryItem() );
    addItem( Servo::libraryItem() );
    addItem( AudioOut::libraryItem() );
    addItem( Lamp::libraryItem() );
    // Micro
    addItem( Z80CPU::libraryItem() );
    addItem( new LibraryItem( "AVR", QObject::tr("Micro"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( "PIC", QObject::tr("Micro"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( "I51", QObject::tr("Micro"), "ic2.png","", NULL ) );
    addItem( new LibraryItem("MCS65",QObject::tr("Micro"), "ic2.png","", NULL ) );
    addItem( Mcu::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Arduino"), QObject::tr("Micro"), "board.png","", NULL ) );
    addItem( new LibraryItem( QObject::tr("Shields"), QObject::tr("Micro"), "shield.png","", NULL ) );
    addItem( new LibraryItem( QObject::tr("Sensors"), QObject::tr("Micro"), "1to2.png","", NULL ) );
    addItem( SR04::libraryItem() );
    addItem( Dht22::libraryItem() );
    addItem( DS1621::libraryItem() );
    addItem( Ds18b20::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Perifericals"), QObject::tr("Micro"), "perif.png","", NULL ) );
    addItem( SerialPort::libraryItem() );
    addItem( TouchPad::libraryItem() );
    addItem( KY023::libraryItem() );
    addItem( KY040::libraryItem() );
    addItem( DS1307::libraryItem() );
    addItem( Esp01::libraryItem() );
    // Logic
    addItem( new LibraryItem( QObject::tr("Gates"), QObject::tr("Logic"), "gates.png","", NULL ) );
    addItem( Buffer::libraryItem() );
    addItem( AndGate::libraryItem() );
    addItem( OrGate::libraryItem() );
    addItem( XorGate::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Arithmetic"), QObject::tr("Logic"), "2to2.png","", NULL ) );
    addItem( BinCounter::libraryItem() );
    addItem( FullAdder::libraryItem() );
    addItem( ShiftReg::libraryItem() );
    addItem( Function::libraryItem() );
    //addItem( Scripted::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Memory"), QObject::tr("Logic"), "subc.png","", NULL ) );
    addItem( FlipFlopD::libraryItem() );
    addItem( FlipFlopT::libraryItem() );
    addItem( FlipFlopRS::libraryItem() );
    addItem( FlipFlopJK::libraryItem() );
    addItem( LatchD::libraryItem() );
    addItem( Memory::libraryItem() );
    addItem( I2CRam::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Converters"), QObject::tr("Logic"), "1to2.png","", NULL ) );
    addItem( Mux::libraryItem() );
    addItem( Demux::libraryItem() );
    addItem( BcdToDec::libraryItem() );
    addItem( DecToBcd::libraryItem() );
    addItem( BcdTo7S::libraryItem() );
    addItem( I2CToParallel::libraryItem() );
    addItem( new LibraryItem( QObject::tr("Other Logic"), QObject::tr("Logic"), "2to3.png","", NULL ) );
    addItem( ADC::libraryItem() );
    addItem( DAC::libraryItem() );
    addItem( SevenSegmentBCD::libraryItem() );
    addItem( Lm555::libraryItem() );
    // Subcircuits
    /*addItem( new LibraryItem( "IC 74", QObject::tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( "IC CD", QObject::tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( "USSR IC", QObject::tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( QObject::tr("Other IC"), QObject::tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( QObject::tr("Keys"), QObject::tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( QObject::tr("Ternary"), QObject::tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( QObject::tr("Tools"), QObject::tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( QObject::tr("Led display"), QObject::tr("Logic"), "ic2.png","", NULL ) );*/
    addItem( SubCircuit::libraryItem() );
    // Connectors
    addItem( Bus::libraryItem() );
    addItem( Tunnel::libraryItem() );
    addItem( Socket::libraryItem() );
    addItem( Header::libraryItem() );
    // Graphic
    addItem( Image::libraryItem() );
    addItem( TextComponent::libraryItem() );
    addItem( Rectangle::libraryItem() );
    addItem( Ellipse::libraryItem() );
    addItem( Line::libraryItem() );
    // Other
    addItem( SubPackage::libraryItem() );
}

void ItemLibrary::addItem( LibraryItem* item )
{
    if (!item) return;
    m_items.append(item);
}

/*void ItemLibrary::loadPlugins()
{
    m_plugins.clear();
    QDir pluginsDir( qApp->applicationDirPath() );

    pluginsDir.cd( "data/plugins" );

    qDebug() << "\n    Loading App plugins at:\n"<<pluginsDir.absolutePath()<<"\n";

    QString pluginName = "*plugin.*";
    pluginsDir.setNameFilters( QStringList(pluginName) );

    for( pluginName : pluginsDir.entryList( QDir::Files ) )
    {
        QPluginLoader pluginLoader( pluginsDir.absoluteFilePath( pluginName ) );
        QObject *plugin = pluginLoader.instance();

        pluginName = pluginName.split(".").first().remove("lib").remove("plugin").toUpper();

        if( plugin )
        {
            AppIface* item = qobject_cast<AppIface*>( plugin );

            item->initialize() override;
            if( item && !(m_plugins.contains(pluginName)) )
            {
                m_plugins.append(pluginName);
                qDebug()<< "        Loaded plugin\t" << pluginName;
            }
        }
        else
        {
            QString errorMsg = pluginLoader.errorString();
            qDebug()<< "        " << pluginName << "\tplugin FAILED: " << errorMsg;

            if( errorMsg.contains( "libQt5SerialPort" ) )
                errorMsg = " Qt5SerialPort is not installed in your system\n\n    Mcu SerialPort will not work\n    Just Install libQt5SerialPort package\n    To have Mcu Serial Port Working";

            QMessageBox::warning( 0,"App Plugin Error:", errorMsg );
        }
    }
    qDebug() << "\n";
}*/


const QList<LibraryItem *> ItemLibrary::items() const
{
    return m_items;
}

LibraryItem *ItemLibrary::itemByName(const QString name) const
{
    for( LibraryItem* item : m_items )
    {
        if( item->name() == name ) return item;
    }
    return NULL;
}

LibraryItem* ItemLibrary::libraryItem(const QString type ) const
{
    for( LibraryItem* item : m_items )
    {
        if( item->type() == type ) return item;
    }
    return NULL;
}


// CLASS LIBRARYITEM *********************************************************

LibraryItem::LibraryItem( const QString &name,
                          const QString &category,
                          const QString &iconName,
                          const QString type,
                          createItemPtr _createItem )
{
    m_name      = name;
    m_category  = category;
    m_iconfile  = iconName;
    m_type      = type;
    createItem  = _createItem;

}

LibraryItem::~LibraryItem() { }

