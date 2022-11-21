/***************************************************************************
 *   Copyright (C) 2010 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LOGICCOMPONENT_H
#define LOGICCOMPONENT_H

#include "iocomponent.h"
#include "e-clocked_device.h"

class IoPin;

class MAINMODULE_EXPORT LogicComponent : public IoComponent, public eClockedDevice
{
    public:
        LogicComponent( QObject* parent, QString type, QString id );
        ~LogicComponent();

        virtual void stamp() override;

        void createOePin ( QString d, QString id ) { setOePin( createPin( d, id ) ); }
        void setOePin( IoPin* pin );
        void enableOutputs( bool en );
        void updateOutEnabled();
        bool outputEnabled();

        virtual void setInputHighV( double volt ) override;
        virtual void setInputLowV( double volt ) override;
        virtual void setInputImp( double imp ) override;

        bool tristate() { return m_tristate; }
        virtual void setTristate( bool t );

        QString triggerStr() { return m_enumUids.at( (int)m_trigger ); }
        void setTriggerStr( QString t );

        virtual std::vector<Pin*> getPins() override;

        virtual void remove() override;

    protected:
        bool m_outEnable;
        bool m_tristate;

        IoPin*  m_oePin;
};

#endif
