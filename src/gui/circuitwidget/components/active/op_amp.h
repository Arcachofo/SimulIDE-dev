/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2016 by santiago González                               *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef OPAMP_H
#define OPAMP_H

#include "component.h"
#include "e-element.h"

class LibraryItem;
class IoPin;

class MAINMODULE_EXPORT OpAmp : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( double Gain       READ gain          WRITE setGain      DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Imped  READ outImp        WRITE setOutImp    DESIGNABLE true USER true )
    Q_PROPERTY( double Volt_Pos   READ voltPos       WRITE setVoltPos   DESIGNABLE true USER true )
    Q_PROPERTY( double Volt_Neg   READ voltNeg       WRITE setVoltNeg   DESIGNABLE true USER true )
    Q_PROPERTY( bool   Power_Pins READ hasPowerPins  WRITE setPowerPins DESIGNABLE true USER true )
    Q_PROPERTY( bool   Switch_Pins READ switchPins  WRITE setSwitchPins DESIGNABLE true USER true )
    
    public:

        OpAmp( QObject* parent, QString type, QString id );
        ~OpAmp();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        virtual QList<propGroup_t> propGroups() override;

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void voltChanged() override;

        double gain() {return m_gain;}
        void setGain( double gain ) {m_gain = gain;}

        double outImp() const { return m_outImp; }
        void  setOutImp( double imp );

        double voltPos(){ return m_voltPosDef; }
        void setVoltPos( double volt ){ m_voltPosDef = volt; }

        double voltNeg(){ return m_voltNegDef; }
        void setVoltNeg( double volt ){ m_voltNegDef = volt; }

        bool hasPowerPins(){return m_powerPins;}
        void setPowerPins( bool set );

        bool switchPins() { return m_switchPins; }
        void setSwitchPins( bool s );

        virtual QPainterPath shape() const;
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    protected:
        bool m_powerPins;
        bool m_switchPins;

        double m_accuracy;
        double m_gain;
        double m_k;
        double m_voltPos;
        double m_voltNeg;
        double m_voltPosDef;
        double m_voltNegDef;
        double m_lastOut;
        double m_lastIn;
        double m_outImp;

        IoPin* m_inputP;
        IoPin* m_inputN;
        IoPin* m_output;
};

#endif
