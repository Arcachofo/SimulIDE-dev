/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "updatable.h"
#include "simulator.h"

Updatable::Updatable(){}
Updatable::~Updatable()
{
    Simulator::self()->remFromUpdateList( this );
}

