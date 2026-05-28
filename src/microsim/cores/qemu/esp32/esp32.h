/***************************************************************************
 *   Copyright (C) 2025 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "qemudevice.h"
#include "qemutwi.h"
#include "esp32pin.h"

class LibraryItem;
//class esp32Pin;
class Esp32Gpio;
class Esp32IoMux;
class Esp32Led;

class Esp32 : public QemuDevice
{
    public:
        Esp32( QString type, QString id, QString device );
        ~Esp32();

        void stamp() override;

    protected:
        Pin* addPin( QString id, QString type, QString label,
                    int n, int x, int y, int angle, int length=8, double space=0 ) override;

        bool createArgs() override;

         void updtFrequency() override;

        void createMatrix();

        uint32_t m_cpuFreq;
        uint32_t m_apbFreq;

        Esp32Gpio* m_gpio;
        Esp32IoMux* m_iomux;

        Esp32Led* m_leds;
};
