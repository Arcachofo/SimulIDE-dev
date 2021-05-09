/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef ELOGICDEVICE_H
#define ELOGICDEVICE_H

#include "e-clocked_device.h"
#include "e-pin.h"

class MAINMODULE_EXPORT eLogicDevice : public eClockedDevice
{
    public:

        eLogicDevice( QString id );
        ~eLogicDevice();

        int  numInps() const            { return m_numInputs; }
        virtual void setNumInps( int inputs );
                                            
        int  numOuts() const            { return m_numOutputs; }
        void setNumOuts( int outputs );

        double inputHighV() const          { return m_inputHighV; }
        void  setInputHighV( double volt ) { m_inputHighV = volt; }

        double inputLowV() const          { return m_inputLowV; }
        void  setInputLowV( double volt ) { m_inputLowV = volt; }

        double outHighV() const           { return m_outHighV; }
        void  setOutHighV( double volt );

        double outLowV() const            { return m_outLowV; }
        void  setOutLowV( double volt );

        double inputImp() const           { return m_inputImp; }
        void  setInputImp( double imp );

        double outImp() const            { return m_outImp; }
        void  setOutImp( double imp );

        bool inverted() { return m_inverted; }
        void setInverted( bool inverted );

        bool invertInps() { return m_invInputs; }
        void setInvertInps( bool invert );

        void setOutputEnabled( bool enabled );
        void updateOutEnabled();

        virtual uint64_t propDelay() { return m_propDelay; }
        virtual void     setPropDelay( uint64_t pd ) { m_propDelay = pd; }

        uint64_t riseTime() { return m_timeLH; }
        void setRiseTime( uint64_t time );

        uint64_t fallTime() { return m_timeHL; }
        void setFallTime( uint64_t time );

        virtual void stamp() override;
        virtual void initialize() override;
        virtual void runEvent() override;

        void sheduleOutPuts();

        virtual ePin* getEpin( QString pinName );

        bool outputEnabled();

        virtual void createPins( int inputs, int outputs );
        void createOutEnablePin();

        void setInput( int n, eSource* input );

    protected:
        void createOutEnablePin( ePin* epin );
        void createOutEnableeSource( ePin* epin );
        void createInput( ePin* epin );
        void createOutput( ePin* epin );
        
        void createInputs( int inputs );
        void createOutputs( int outputs );
        void deleteInputs( int inputs );
        void deleteOutputs( int inputs );
        void setOut( int num, bool out );
        
        bool getInputState( int input );
        bool getOutputState( int output );

        /// double m_inputHighV;  // currently in eClockedDevice
        /// double m_inputLowV;  // currently in eClockedDevice
        double m_outHighV;
        double m_outLowV;

        uint64_t m_propDelay; // Propagation delay
        uint64_t m_timeLH;    // Time for Output voltage to switch from 10% to 90%
        uint64_t m_timeHL;    // Time for Output voltage to switch from 90% to 10%
        bool m_rndPD;         // Randomize Propagation Delay

        /// double m_inputImp;  // currently in eClockedDevice
        double m_outImp;

        int m_numInputs;
        int m_numOutputs;

        int m_outValue;
        int m_nextOutVal;
        int m_outStep;

        bool m_outEnable;
        bool m_inverted;
        bool m_invInputs;

        eSource* m_outEnSource;

        std::vector<eSource*> m_output;
        std::vector<eSource*> m_input;
        std::vector<bool>     m_inputState;
};

#endif

