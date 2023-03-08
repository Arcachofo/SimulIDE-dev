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

 static ConfigWord* createCfgWord( eMcu* mcu, QString name, QString type );
};

class PicConfigWord00 : public PicConfigWord, public eElement
{
    public:
        PicConfigWord00( eMcu* mcu, QString name );
        ~PicConfigWord00();

        virtual bool setCfgWord( uint16_t addr, uint16_t data ) override;
};
#endif
