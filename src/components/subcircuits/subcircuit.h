/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SUBCIRCUIT_H
#define SUBCIRCUIT_H

#include "chip.h"
#include "embedcircuit.h"

class Tunnel;
class LibraryItem;

class SubCircuit : public Chip, public EmbedCircuit
{
    public:
        SubCircuit( QString type, QString id );
        ~SubCircuit();

 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void remove() override;

        virtual void setLogicSymbol( bool ls ) override;

        virtual QString toString() override;

        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    protected:
        virtual Pin* addPin( QString id, QString type, QString label,
                            int pos, int xpos, int ypos, int angle, int length=8, int space=0 ) override;

        virtual Pin* updatePin( QString id, QString type, QString label,
                                int xpos, int ypos, int angle, int length=8, int space=0 ) override;

        void loadGraphProps();


};
#endif

