/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PUSHBASE_H
#define PUSHBASE_H

#include "switch_base.h"

class MAINMODULE_EXPORT PushBase : public SwitchBase
{
    public:
        PushBase( QObject* parent, QString type, QString id );
        ~PushBase();

    public slots:
        void onbuttonPressed();
        void onbuttonReleased();
        virtual void keyEvent( QString key, bool pressed );
};

#endif
