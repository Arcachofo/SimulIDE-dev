/***************************************************************************
 *   Copyright (C) 2016 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef AIP31068_I2C_H
#define AIP31068_I2C_H

#include "hd44780_base.h"
#include "twimodule.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT Aip31068_i2c : public Hd44780_Base, public TwiModule
{
    public:
        Aip31068_i2c( QObject* parent, QString type, QString id );
        ~Aip31068_i2c();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        virtual void startWrite() override;
        virtual void readByte() override;

        void showPins( bool show );

    private:
        int m_controlByte;
        int m_phase;

        IoPin* m_pinSda;
};

#endif
