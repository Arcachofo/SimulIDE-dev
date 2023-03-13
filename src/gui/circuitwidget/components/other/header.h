/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef HEADER_H
#define HEADER_H

#include "connbase.h"

class LibraryItem;

class MAINMODULE_EXPORT Header : public ConnBase
{
    public:
        Header( QObject* parent, QString type, QString id );
        ~Header();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        bool pointDown() { return m_pointDown; }
        void setPointDown( bool down );

    private:
        virtual void updatePixmap() override;

        bool m_pointDown;
};

#endif
