/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef LDR_H
#define LDR_H

#include "varresbase.h"

class LibraryItem;

class Ldr : public VarResBase
{
    public:
        Ldr( QString type, QString id );
        ~Ldr();

        static Component* construct( QString type, QString id );
        static LibraryItem* libraryItem();

        int r1() { return m_r1; }
        void setR1( int r1 );

        double gamma() { return m_gamma; }
        void setGamma( double ga );

        virtual void updateStep() override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    private:
        int m_r1;

        double m_gamma;
};

#endif
