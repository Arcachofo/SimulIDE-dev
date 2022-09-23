/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CPUEXTMEM_H
#define CPUEXTMEM_H

#include "e-element.h"

class eMcu;
class McuPort;
class McuPin;

enum memOpType_t{
    mNONE  = 0,
    mREAD  = 1,
    mWRITE = 1<<1,
    mMEM   = 1<<2,
    mIO    = 1<<3,
    m_ReadMEM  = (mREAD | mMEM),
    m_ReadIO   = (mREAD | mIO),
    m_WriteMEM = (mWRITE | mMEM),
    m_WriteIO  = (mWRITE | mIO),
};
struct memOp_t
{
    memOpType_t type;
    uint32_t*   addr;
};

class MAINMODULE_EXPORT CpuExtMem : public eElement
{
    public:
        CpuExtMem( eMcu* mcu );
        ~CpuExtMem();

        void addMemOp( memOpType_t type, uint32_t* addr );

        uint8_t getData() { return m_data; }

    protected:
        void clock( bool rising );

        void readMem( uint32_t* addr );
        void writeMem( uint32_t* addr );

        memOp_t m_memOp;
        memOp_t m_memOpList[10];

        int m_opStep;
        int m_nOps;                // Number of Operations
        int m_opIndex;             // Current Operation index

        uint8_t  m_data;           // Data readen from Memory
        //uint16_t m_addr;

        McuPort* m_dataPort;
        McuPort* m_addrPort;

};
#endif
