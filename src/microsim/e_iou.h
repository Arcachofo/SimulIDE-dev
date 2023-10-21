/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef EIOU_H
#define EIOU_H

#include <QHash>

#include "e-element.h"

class CpuBase;
class Mcu;
class IoPort;
class IoPin;
class Watcher;

class eIou : public eElement
{
    public:
        eIou( Mcu* comp, QString id );
        ~eIou();

        void reset();

        IoPort* getIoPort( QString name );
        IoPin*  getIoPin( QString pinName );

        Watcher* getWatcher() { return m_watcher; }
        void createWatcher( CpuBase* cpu );

        Mcu* component() { return m_component; }
        CpuBase* cpu()   { return m_cpu; }

    protected:
        Mcu* m_component;

        CpuBase* m_cpu;

        IoPin*  m_clkPin;

        Watcher* m_watcher;

        QHash<QString, IoPort*> m_ioPorts;  // Access ioPORTS by name
};
#endif
