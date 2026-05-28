/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "qemudevice.h"
#include "ioport.h"

class Stm32Port;

class Stm32 : public QemuDevice
{
    public:
        Stm32( QString type, QString id, QString device );
        ~Stm32();

        void stamp() override;

        void timerRemap( int number, uint8_t value );

    protected:
        bool createArgs() override;

        Pin* addPin( QString id, QString type, QString label,
                    int n, int x, int y, int angle, int length=8, double space=0 ) override;

        void createPorts();
        //void doAction() override;

        void updtFrequency() override;

        uint32_t m_apb1;
        uint32_t m_apb2;


        std::vector<Stm32Port*> m_ports;

        uint32_t m_model;
};
