/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LOGICCOMPONENT_H
#define LOGICCOMPONENT_H

#include "iocomponent.h"
#include "e-clocked_device.h"

class IoPin;

class LogicComponent : public IoComponent, public eClockedDevice
{
    public:
        LogicComponent( QString type, QString id );
        ~LogicComponent();

        virtual void stamp() override;

        void createOePin ( QString d, QString id ) { setOePin( createPin( d, id ) ); }
        void setOePin( IoPin* pin );
        void enableOutputs( bool en );
        void updateOutEnabled();
        bool outputEnabled();

        virtual void setInpHighV( double volt ) override;
        virtual void setInpLowV( double volt ) override;
        virtual void setInputImp( double imp ) override;

        bool tristate() { return m_tristate; }
        virtual void setTristate( bool t );

        QString triggerStr() { return m_triggerStr; }
        void setTriggerStr( QString t );

        virtual std::vector<Pin*> getPins() override;

        virtual void remove() override;

 static const QString m_triggerList;

    protected:
        bool m_outEnable;
        bool m_tristate;

        QString m_triggerStr;

        IoPin* m_oePin;
};

#endif
