/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LACHANNEL_H
#define LACHANNEL_H

#include "datachannel.h"
#include "logicanalizer.h"

class eNode;
class Pin;

class MAINMODULE_EXPORT LaChannel : public DataChannel
{
        friend class LAnalizer;

    public:

        LaChannel( LAnalizer* la, QString id );
        ~LaChannel();

        virtual void initialize() override;
        virtual void stamp() override;
        //virtual void updateStep() override;
        virtual void voltChanged() override;

        void setPin( Pin* p );

        virtual void setIsBus( bool b ) override;
        void registerEnode( eNode* enode, int n=-1 );

    private:
        void addReading( double v );

        double m_busValue;

        QMap<int, eNode*> m_busNodes;

        LAnalizer* m_analizer;
};

#endif

