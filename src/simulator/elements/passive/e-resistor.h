/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef ERESISTOR_H
#define ERESISTOR_H

#include "e-element.h"

class eResistor : public eElement
{
    public:
        eResistor( QString id );
        ~eResistor();

        virtual void stamp() override;

        virtual double resistance() { return 1/m_admit; }
        virtual void setResistance( double resist );

        virtual void setResSafe( double resist );
        
        double admit() { return m_admit; }
        void setAdmit( double admit );
        void stampAdmit();

        double current();

    protected:
        virtual void updateVI();

        double m_admit;
        double m_current;
};

#endif
