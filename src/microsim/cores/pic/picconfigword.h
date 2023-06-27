/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef PICCONFIGWORD_H
#define PICCONFIGWORD_H

#include "mcuconfigword.h"
#include "e-element.h"

class McuPin;

class PicConfigWord : public ConfigWord
{
    public:
        PicConfigWord( eMcu* mcu, QString name );
        ~PicConfigWord();

        void configClock( uint8_t fosc, bool wdte, bool mcrl );

 static ConfigWord* createCfgWord( eMcu* mcu, QString name, QString type );
};

class PicConfigWord00 : public PicConfigWord
{
    public:
        PicConfigWord00( eMcu* mcu, QString name );
        ~PicConfigWord00();

        virtual bool setCfgWord( uint16_t addr, uint16_t data ) override;
};

class PicConfigWord01 : public PicConfigWord
{
    public:
        PicConfigWord01( eMcu* mcu, QString name );
        ~PicConfigWord01();

        virtual bool setCfgWord( uint16_t addr, uint16_t data ) override;
};

class PicConfigWord02 : public PicConfigWord
{
    public:
        PicConfigWord02( eMcu* mcu, QString name );
        ~PicConfigWord02();

        virtual bool setCfgWord( uint16_t addr, uint16_t data ) override;
};

class PicConfigWord03 : public PicConfigWord
{
    public:
        PicConfigWord03( eMcu* mcu, QString name );
        ~PicConfigWord03();

        virtual bool setCfgWord( uint16_t addr, uint16_t data ) override;
};

class PicConfigWord04 : public PicConfigWord
{
    public:
        PicConfigWord04( eMcu* mcu, QString name );
        ~PicConfigWord04();

        virtual bool setCfgWord( uint16_t addr, uint16_t data ) override;
};
#endif
