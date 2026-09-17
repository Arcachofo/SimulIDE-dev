/***************************************************************************
 *   Copyright (C) 2026 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "esp32pin.h"
#include "qemumodule.h"
#include "e-element.h"

class Esp32Gpio : public QemuModule, public eElement
{
    friend class Esp32;

    public:
        Esp32Gpio( QemuDevice* mcu, QString name, int n, uint32_t* clk=nullptr, uint64_t memStart=0, uint64_t memEnd=0 );
        ~Esp32Gpio();

        void reset() override;

        uint32_t readPort( int in );

        uint32_t strapMode();

        void writeIoMuxReg( uint8_t pin, uint16_t value );

        //Esp32Pin* getPin( int i ) { return m_pins.at(i); }
        //uint size(){ return m_pins.size(); }

    protected:
        void writeRegister() override;
        void readRegister()  override;

        Esp32Pin* createPin( int i, QString id , QemuDevice* mcu );

        void createIoMux();

        //void cofigPort( uint32_t config, uint8_t shift );
        //void setPortState( uint16_t state );
        void matrixInChanged( int func );
        void matrixOutChanged( int pin );
        void setGpioState( uint32_t newState );
        void setGpioDir( uint32_t newEnable );
        void clearStatus( int i );

        std::vector<Esp32Pin*> m_pins;
        Esp32Pin* m_espPad[40];
        Esp32Pin* m_dummyPin;

        uint16_t m_pinState;

        uint32_t m_gpioState;
        uint32_t m_gpioEnable;
        uint32_t m_strapMode;
        uint32_t m_gpioStatus[2];

        //uint32_t gpio_in[2];

        //uint32_t gpio_pcpu_int[2];
        //uint32_t gpio_acpu_int[2];

        // uint32_t m_gpioPin[40]; // Managed by Pin

        //uint32_t m_gpioInFunc[256];
        //uint32_t m_gpioOutFunc[256];

        funcPin m_matrixIn[256];  // Matrix created in Esp32::createMatrix()
        funcPin m_matrixOut[256]; // Matrix created in Esp32::createMatrix()
};
