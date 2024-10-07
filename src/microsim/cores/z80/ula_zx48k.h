/***************************************************************************
 *   Copyright (C) 2023 by Jarda Vrana                                     *
 *   jarda.vrana@gmail.com                                                 *
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

#ifndef ULA_ZX48K_H
#define ULA_ZX48K_H

#include "cpubase.h"
#include "e-element.h"

class ULA_ZX48k : public CpuBase, public eElement
{
    public:
        ULA_ZX48k( eMcu* mcu );
        ~ULA_ZX48k();

        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;
        virtual void runEvent() override;

        virtual void runStep() override;

        virtual int getCpuReg( QString reg ) override;

        QString type() { return m_types.at((int)m_type); }
        void setType( QString producer );

        bool isScreen() { return m_isSrceen; }
        void setScreen( bool s );

    private:
        enum eType { ula5c102e = 0, ula5c112e, ula6c001e6, ula6c001e7, ula6c011e };
        eType m_type;
        QStringList m_types;

        void setParameters(eType type);
        void clk7FallingEdge();
        void clk7RisingEdge();
        inline void increaseCounters();
        inline void updateVideo();
        inline void readVideoData();
        inline void generatePhicpu();
        inline void portIO();

        bool m_isSrceen;
        uint64_t m_vidCasDelayFall;
        uint64_t m_vidCasDelayFallFirst;
        uint64_t m_vidCasDelayRise;
        uint16_t m_hSyncFirst;
        uint16_t m_hSyncLast;
        uint16_t m_scanLines;
        uint16_t m_vSyncFirst;
        uint16_t m_vSyncLast;

        bool m_clk7;
        uint16_t m_C;
        uint16_t m_V;
        uint8_t m_flashClock;
        bool m_tclka;
        bool m_tclkb;
        bool m_int;

        bool m_border;
        bool m_vidRas;
        bool m_vidCas;
        uint8_t m_dataLatch;
        uint8_t m_shiftReg;
        uint8_t m_attrDataLatch;
        uint8_t m_attrOutLatchInk;
        uint8_t m_attrOutLatchPaper;
        uint8_t m_attrOutLatchFlBr;
        uint8_t m_borderColour;
        uint8_t m_screen[448][312];
        bool m_evenScanLine;
        static const float m_yTable[16];
        static const float m_uTable[8];
        static const float m_vTable[8];
        static const float m_vInvTable[8];
        static const float m_micSp5CTable[4];
        static const float m_micSp6CTable[4];
        static const float m_micSpImpTable[4];
        static int m_colours[16];

        bool m_a14;
        bool m_a15;
        bool m_mreqn;
        bool m_iorqn;
        bool m_rdn;
        bool m_wrn;

        bool m_mreqT23;
        bool m_iorqTW3;
        bool m_cpuClk;

        bool m_romcsn;
        bool m_rasEn;
        bool m_ram16;
        bool m_cpuCas;
        bool m_dramWe;
        bool m_portRd;
        bool m_portWr;
        bool m_mic;

        bool m_aeDelayed;

        IoPin* m_rasPin;
        IoPin* m_casPin;
        IoPin* m_dramwePin;
        IoPin* m_mreqPin;
        IoPin* m_iorqPin;
        IoPin* m_rdPin;
        IoPin* m_wrPin;
        IoPin* m_a14Pin;
        IoPin* m_a15Pin;
        IoPin* m_intPin;
        IoPin* m_romcsPin;
        IoPin* m_phicpuPin;
        IoPin* m_uPin;
        IoPin* m_vPin;
        IoPin* m_yPin;
        IoPin* m_micTapePin;
        IoPin* m_dmaPort0;
        IoPin* m_kbPort0;

        IoPort* m_dmaPort;
        IoPort* m_dPort;
        IoPort* m_kbPort;
};

#endif
