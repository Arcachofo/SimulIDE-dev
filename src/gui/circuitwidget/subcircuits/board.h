/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef BOARD_H
#define BOARD_H

#include "subcircuit.h"

class ShieldSubc;

class BoardSubc : public SubCircuit
{
    public:
        BoardSubc( QString type, QString id );
        ~BoardSubc();

        void attachShield( ShieldSubc* shield );
        void detachShield( ShieldSubc* shield ) { m_shields.removeAll( shield); }

        BoardSubc* parentBoard() { return m_parentBoard; }

        virtual QString toString() override;

    protected:
        QList<ShieldSubc*> m_shields; // A list of shields attached to this
        BoardSubc* m_parentBoard;     // A board this is attached to (this is a shield)

};
#endif
