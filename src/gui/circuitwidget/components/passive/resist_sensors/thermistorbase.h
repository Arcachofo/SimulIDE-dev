/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef THERMISTORBASE_H
#define THERMISTORBASE_H

#include "varresbase.h"

class MAINMODULE_EXPORT ThermistorBase : public VarResBase
{
    public:
        ThermistorBase( QObject* parent, QString type, QString id );
        ~ThermistorBase();

        bool ptc()  { return m_ptc; }
        void setPtc( bool ptc );

        virtual void setMaxVal( double max ) override;
        virtual void setMinVal( double min ) override;

    protected:
        bool m_ptc;
};

#endif
