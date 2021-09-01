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
#include "simuapi_apppath.h"
#include "appiface.h"

//BEGIN Item includes
#include "amperimeter.h"
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
#include "diode.h"
#include "elcapacitor.h"
#include "ellipse.h"
#include "flipflopd.h"
#include "flipflopjk.h"
#include "flipfloprs.h"
#include "frequencimeter.h"
#include "fulladder.h"
#include "function.h"
#include "gate_and.h"
#include "gate_or.h"
#include "gate_xor.h"
#include "ground.h"
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
#include "latchd.h"
#include "ldr.h"
#include "led.h"
#include "ledbar.h"
#include "ledmatrix.h"
#include "line.h"
#include "lm555.h"
#include "logicanalizer.h"
#include "logicinput.h"
#include "max72xx_matrix.h"
#include "mcu.h"
#include "memory.h"
#include "mosfet.h"
#include "mux.h"
#include "mux_analog.h"
#include "op_amp.h"
#include "oscope.h"
#include "piccomponent.h"
#include "pcd8544.h"
#include "probe.h"
#include "potentiometer.h"
#include "push.h"
#include "rail.h"
#include "rectangle.h"
#include "relay-spst.h"
#include "resistor.h"
#include "resistordip.h"
#include "rtd.h"
#include "scripted.h"
#include "serialport.h"
#include "serialterm.h"
#include "servo.h"
#include "sevensegment.h"
#include "sevensegment_bcd.h"
#include "shiftreg.h"
#include "sr04.h"
#include "ssd1306.h"
#include "stepper.h"
#include "strain.h"
#include "subcircuit.h"
#include "subpackage.h"
#include "switch.h"
#include "switchdip.h"
#include "thermistor.h"
#include "textcomponent.h"
#include "tunnel.h"
#include "varresistor.h"
#include "voltimeter.h"
#include "volt_reg.h"
#include "voltsource.h"
#include "wavegen.h"
#include "ws2812.h"
//END Item includes

ItemLibrary* ItemLibrary::m_pSelf = 0l;

ItemLibrary::ItemLibrary()
{
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
    addItem( Frequencimeter::libraryItem() );
    addItem( Oscope::libraryItem() );
    addItem( LAnalizer::libraryItem() );
    // Sources
    addItem( LogicInput::libraryItem() );
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
    addItem( RelaySPST::libraryItem() );
    addItem( KeyPad::libraryItem() );
    // Passive
    addItem( new LibraryItem( tr("Resistors"),tr("Passive"), "resistors.png","", 0l ) );
    addItem( Resistor::libraryItem() );
    addItem( ResistorDip::libraryItem() );
    addItem( Potentiometer::libraryItem() );
    addItem( VarResistor::libraryItem() );
    addItem( new LibraryItem( tr("Resistive Sensors"),tr("Passive"), "resistorsensors.png","", 0l ) );
    addItem( Ldr::libraryItem() );
    addItem( Thermistor::libraryItem() );
    addItem( RTD::libraryItem() );
    addItem( Strain::libraryItem() );

    addItem( Capacitor::libraryItem() );
    addItem( elCapacitor::libraryItem() );
    addItem( Inductor::libraryItem() );
    // Active
    addItem( Diode::libraryItem() );
    addItem( VoltReg::libraryItem() );
    addItem( OpAmp::libraryItem() );
    addItem( Mosfet::libraryItem() );
    addItem( BJT::libraryItem() );
    addItem( MuxAnalog::libraryItem() );
    // Outputs
    addItem( new LibraryItem( tr("Leds"),tr("Outputs"), "leds.png","", 0l ) );
    addItem( Led::libraryItem() );
    addItem( LedBar::libraryItem() );
    addItem( SevenSegment::libraryItem() );
    addItem( LedMatrix::libraryItem() );
    addItem( Max72xx_matrix::libraryItem() );
    addItem( WS2812::libraryItem() );
    addItem( new LibraryItem( tr("Displays"),tr("Outputs"), "displays.png","", 0l ) );
    addItem( Hd44780::libraryItem() );
    addItem( Aip31068_i2c::libraryItem() );
    addItem( Pcd8544::libraryItem() );
    addItem( Ks0108::libraryItem() );
    addItem( Ssd1306::libraryItem() );
    addItem( Ili9341::libraryItem() );
    addItem( new LibraryItem( tr("Motors"),tr("Outputs"), "motors.png","", 0l ) );
    addItem( DcMotor::libraryItem() );
    addItem( Stepper::libraryItem() );
    addItem( Servo::libraryItem() );
    addItem( AudioOut::libraryItem() );
    // Micro
    addItem( new LibraryItem( tr("AVR"),tr("Micro"), "ic2.png","", 0l ) );
    addItem( PICComponent::libraryItem() );
    addItem( new LibraryItem( tr("I51"),tr("Micro"), "ic2.png","", 0l ) );
    addItem( new LibraryItem( tr("Arduino"),tr("Micro"), "board.png","", 0l ) );
    addItem( new LibraryItem( tr("Shields"),tr("Micro"), "shield.png","", 0l ) );
    addItem( new LibraryItem( tr("Sensors"),tr("Micro"), "1to2.png","", 0l ) );
    addItem( SR04::libraryItem() );
    addItem( new LibraryItem( tr("Perifericals"),tr("Micro"), "perif.png","", 0l ) );
    addItem( KY023::libraryItem() );
    addItem( KY040::libraryItem() );
    addItem( SerialPort::libraryItem() );
    addItem( SerialTerm::libraryItem() );
    // Logic
    addItem( new LibraryItem( tr("Gates"),tr("Logic"), "gates.png","", 0l ) );
    addItem( Buffer::libraryItem() );
    addItem( AndGate::libraryItem() );
    addItem( OrGate::libraryItem() );
    addItem( XorGate::libraryItem() );
    addItem( new LibraryItem( tr("Arithmetic"),tr("Logic"), "2to2.png","", 0l ) );
    addItem( FlipFlopD::libraryItem() );
    addItem( FlipFlopRS::libraryItem() );
    addItem( FlipFlopJK::libraryItem() );
    addItem( LatchD::libraryItem() );
    addItem( Memory::libraryItem() );
    addItem( I2CRam::libraryItem() );
    addItem( new LibraryItem( tr("Memory"),tr("Logic"), "subc.png","", 0l ) );
    addItem( BinCounter::libraryItem() );
    addItem( FullAdder::libraryItem() );
    addItem( ShiftReg::libraryItem() );
    addItem( Function::libraryItem() );
    addItem( Scripted::libraryItem() );
    addItem( new LibraryItem( tr("Converters"),tr("Logic"), "1to2.png","", 0l ) );
    addItem( Mux::libraryItem() );
    addItem( Demux::libraryItem() );
    addItem( BcdToDec::libraryItem() );
    addItem( DecToBcd::libraryItem() );
    addItem( BcdTo7S::libraryItem() );
    addItem( I2CToParallel::libraryItem() );
    addItem( new LibraryItem( tr("Other Logic"),tr("Logic"), "2to3.png","", 0l ) );
    addItem( ADC::libraryItem() );
    addItem( DAC::libraryItem() );
    addItem( Bus::libraryItem() );
    addItem( SevenSegmentBCD::libraryItem() );
    addItem( Lm555::libraryItem() );
    // Subcircuits
    addItem( new LibraryItem( "IC 74",tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( "IC CD",tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( "USSR IC",tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( tr("Other IC"),tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( tr("Keys"),tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( tr("Ternary"),tr("Logic"), "ic2.png","", NULL ) );
    addItem( new LibraryItem( tr("Tools"),tr("Logic"), "ic2.png","", NULL ) );
    addItem( SubCircuit::libraryItem() );

    // Other
    addItem( Image::libraryItem() );
    addItem( TextComponent::libraryItem() );
    addItem( Rectangle::libraryItem() );
    addItem( Ellipse::libraryItem() );
    addItem( Line::libraryItem() );
    addItem( Tunnel::libraryItem() );
    addItem( SubPackage::libraryItem() );

    addItem( new LibraryItem( tr("PICs"),tr("NEW_MCU"), "ic2.png","", 0l ) );
    addItem( Mcu::libraryItem() );
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
    return 0l;
}

LibraryItem* ItemLibrary::libraryItem(const QString type ) const
{
    for( LibraryItem* item : m_items )
    {
        if( item->type() == type ) return item;
    }
    return 0l;
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

