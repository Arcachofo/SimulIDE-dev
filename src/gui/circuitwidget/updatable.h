/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef UPDATABLE_H
#define UPDATABLE_H

class MAINMODULE_EXPORT Updatable
{
    public:
        Updatable();
        ~Updatable();

        virtual void updateStep(){;}
};

#endif
