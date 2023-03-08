/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CONFIGWORD_H
#define CONFIGWORD_H

#include <QHash>

class eMcu;

class ConfigWord
{
        friend class McuCreator;
    public:
        ConfigWord( eMcu* mcu, QString name );
        ~ConfigWord();

        virtual bool setCfgWord( uint16_t addr, uint16_t data );
        uint16_t getCfgWord( uint16_t addr=0 );

    protected:
        QString m_name;
        eMcu*   m_mcu;

        QHash<uint16_t, uint16_t> m_cfgWords; // Config words
};
#endif
