/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#ifndef AUDIOOUT_H
#define AUDIOOUT_H

#include <QAudioOutput>

#include "e-resistor.h"
#include "component.h"

class LibraryItem;

class AudioOut : public Component, public eResistor
{
        Q_OBJECT
    public:
        AudioOut( QObject* parent, QString type, QString id );
        ~AudioOut();
    
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void initialize() override;
        virtual void runEvent() override;

        bool buzzer() { return m_buzzer; }
        void setBuzzer( bool b ) { m_buzzer = b; }
        
        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
        
    private:
        QAudioDeviceInfo m_deviceinfo;
        QAudioFormat     m_format;  
        
        QAudioOutput* m_audioOutput;
        QIODevice*    m_auIObuffer;
        QByteArray    m_dataBuffer;

        int m_dataSize;
        int m_dataCount;

        bool m_buzzer;
};

#endif


