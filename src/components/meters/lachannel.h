/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include "datachannel.h"
#include "logicanalizer.h"

class eNode;
class Pin;

class LaChannel : public DataChannel
{
        friend class LAnalizer;

    public:
        LaChannel( LAnalizer* la, QString id );
        ~LaChannel();

        void initialize() override;
        void stamp() override;
        void voltChanged() override;

        void setPin( Pin* p );

        void setIsBus( bool b ) override;
        void registerEnode( eNode* enode, int n=-1 );

        int bitLength() { return m_bitLength; }

    private:
        void addReading( double v );

        double m_busValue;
        int m_bitLength;

        QMap<int, eNode*> m_busNodes;

        LAnalizer* m_analizer;
};
