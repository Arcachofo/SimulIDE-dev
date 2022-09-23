/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCUVREF_H
#define MCUVREF_H

#include <QList>

#include "mcumodule.h"
#include "e-element.h"

class McuPin;

class MAINMODULE_EXPORT McuVref : public McuModule, public eElement
{
        friend class McuCreator;

    public:
        McuVref( eMcu* mcu, QString name );
        ~McuVref();

        virtual void initialize() override;

        double getVref() { return m_vref; }

        void callBack( McuModule* mod, bool call );

    protected:
        //virtual void setMode( uint8_t mode );

        bool m_enabled;
        uint8_t m_mode;

        double m_vref;

        McuPin* m_pinOut;

        QList<McuModule*> m_callBacks;
};

#endif
