/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BOARD_H
#define BOARD_H

#include "subcircuit.h"

class ShieldSubc;

class MAINMODULE_EXPORT BoardSubc : public SubCircuit
{
    public:
        BoardSubc( QObject* parent, QString type, QString id );
        ~BoardSubc();

        void attachShield( ShieldSubc* shield );
        void detachShield( ShieldSubc* shield ) { m_shields.removeAll( shield); }

        virtual void remove() override;

    protected:
        QList<ShieldSubc*> m_shields; // A shield attached to this

};
#endif
