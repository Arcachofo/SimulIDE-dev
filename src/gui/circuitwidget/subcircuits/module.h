/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MODULE_H
#define MODULE_H

#include "shield.h"

class MAINMODULE_EXPORT ModuleSubc : public ShieldSubc
{
    public:
        ModuleSubc( QObject* parent, QString type, QString id );
        ~ModuleSubc();
        
        double zVal() { return zValue(); }
        void setZVal( double v);

    protected:
        virtual void attachToBoard() override;
        virtual void renameTunnels() override;
};
#endif
