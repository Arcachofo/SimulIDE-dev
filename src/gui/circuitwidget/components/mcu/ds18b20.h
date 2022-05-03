/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#ifndef DS18B20_H
#define DS18B20_H

#include <QFont>

#include "e-element.h"
#include "component.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT Ds18b20 : public Component , public eElement
{
    Q_OBJECT

    public:
        Ds18b20( QObject* parent, QString type, QString id );
        ~Ds18b20();

        enum w1State_t{
            W1_IDLE=0,
            W1_COMMAND,
            W1_DATA,
            W1_MATCH,
            W1_SEARCH
        };

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        double tempInc() { return m_tempInc; }
        void setTempInc( double inc ) { m_tempInc = trim( inc ); }

        double temp() { return m_temp; }
        void setTemp( double t );

        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void upbuttonclicked();
        void downbuttonclicked();

    private:
        void dataReceived();
        void dataSent();
        void sendData( uint8_t data, int size=8);
        void writeBit();
        void readBit( uint8_t bit );
        void pulse( uint64_t time, uint64_t witdth ); // Time in us
        void command( uint8_t cmd );

        // Commands
        void skipROM();         // CCh
        void readROM();         // 33h
        void convertTemp();     // 44h
        void readPowerSupply(); // B4h
        void readScratchpad();  // BEh
        void matchROM();        // 55h
        void searchROM();       // F0h

        void sendSearchBit();
        bool bitROM( uint bitIndex );

        w1State_t m_state;
        uint8_t m_lastCommand;

        // Trim to 4 decimals, not one! For 12-bit resolution step is 0.0625
        // For 9-12-bit resolution, steps are acutually 0.5°C, 0.25°C, 0.125°C and 0.0625°C, respectively
        double trim( double data ) { return (double)((int)(data*10000))/10000; }

        bool m_lastIn;
        bool m_write;
        bool m_pullDown;
        bool m_bitROM;
        bool m_parPower;

        uint64_t m_lastTime;
        uint8_t  m_rxReg;
        uint8_t  m_txReg;
        uint8_t  m_lastBit;
        uint8_t  m_bitSearch;
        uint64_t  m_bitIndex;

        int m_pulse;

        double m_temp;
        double m_tempInc;

        QFont m_font;
        IoPin* m_inpin;

    // NEW
        int m_resolution; // 9-12 bits
        uint8_t m_scratchpad[9];
        uint8_t m_ROM[8];

        uint8_t m_TH_reg;  // TH register, alarm trigger register, can be stored in internal EEPROM
        uint8_t m_TL_reg;  // TL register, alarm trigger register, can be stored in internal EEPROM
        uint8_t m_CFG_reg; // Config register, can be stored in internal EEPROM

        std::vector<uint8_t> m_txBuff;
        //std::vector<uint8_t> rx_BUFF;

        ////////////////////////////////////
        // Constants for 1-Wire bit-banging
        ////////////////////////////////////

        // "TIME SLOT", here TSLOT last 60us, minimum
        // For DS1820 is minimum 60us up to 120us
        // Arduino 1-Wire lib uses 60-65us
        const double TSLOT  = 60*1e6;

        // Time to write bit 0, pulling line down
        // It's minimum lenght is the same as TSLOT
        // For DS1820 is minimum 60us up to 120us
        // Arduino 1-Wire lib uses 60-65us
        const double TLOW_0 = TSLOT;

        // TLOW1 is duration to pulling line down for bit 1
        // For DS1820 is between 1 to 15us
        // Arduino 1-Wire lib uses 10us
        const double TLOW_1 = 1*1e6;

        // TREC is recovery time between pulses,
        // pulling up line minimum time
        // For DS1820 is minimum 1us up to 15us
        // Arduino 1-Wire lib uses 5us
        const double TREC = 5*1e6;

        //const double TRVD
};

#endif
