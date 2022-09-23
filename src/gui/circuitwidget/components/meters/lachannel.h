/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LACHANNEL_H
#define LACHANNEL_H

#include "datachannel.h"
#include "logicanalizer.h"


class MAINMODULE_EXPORT LaChannel : public DataChannel
{
        friend class LAnalizer;

    public:

        LaChannel( LAnalizer* la, QString id );
        ~LaChannel();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

    private:
        void addReading( double v );

        LAnalizer* m_analizer;
};

#endif

