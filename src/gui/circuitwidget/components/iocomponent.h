/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef IOCOMPONENT_H
#define IOCOMPONENT_H

#include <queue>

#include "component.h"

class eElement;
class IoPin;

class MAINMODULE_EXPORT IoComponent : public Component
{
    public:
        IoComponent( QObject* parent, QString type, QString id );
        ~IoComponent();

        QList<ComProperty*> inputProps();
        QList<ComProperty*> outputProps();
        QList<ComProperty*> outputType();
        QList<ComProperty*> edgeProps();

        void initState();
        void runOutputs();
        void sheduleOutPuts( eElement* el );

        double inputHighV() { return m_inHighV; }
        virtual void setInputHighV( double volt );

        double inputLowV() { return m_inLowV; }
        virtual void setInputLowV( double volt );

        double outHighV() { return m_ouHighV; }
        void  setOutHighV( double volt );

        double outLowV() { return m_ouLowV; }
        void  setOutLowV( double volt );

        double inputImp() { return m_inImp; }
        virtual void setInputImp( double imp );

        double outImp() { return m_ouImp; }
        void  setOutImp( double imp );

        bool invertOuts() { return m_invOutputs; }
        void setInvertOuts( bool inverted );

        bool invertInps() { return m_invInputs; }
        virtual void setInvertInps( bool invert );

        double propSize() { return m_propSize; }
        void setPropSize( double g ) { m_propSize = g; }

        double propDelay() { return m_propDelay*1e-12; }
        void setPropDelay( double pd ) { m_propDelay = pd*1e12; }

        double riseTime() { return m_timeLH*1e-12; }
        void setRiseTime( double time );

        double fallTime() { return m_timeHL*1e-12; }
        void setFallTime( double time );

        int  numInps() { return m_inPin.size(); }
        virtual void setNumInps( uint pins, QString label="I", int bit0=0, bool number=true );

        int  numOuts() { return m_outPin.size(); }
        virtual void setNumOuts( uint pins, QString label="O", int bit0=0, bool number=true );

        bool openCol() { return m_openCol; }
        void setOpenCol( bool op );

        //bool rndPD() { return m_rndPD; }
        //void setRndPD( bool r ) { m_rndPD = r; }

        void init( QStringList pins );
        void initPin( IoPin* pin );

        virtual std::vector<Pin*> getPins() override;

        virtual void remove() override;

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    protected:
        IoPin* createPin( QString data, QString id );
        void setNumPins( std::vector<IoPin*>* pinList, uint pins, QString label, int bit0, bool out, bool number );
        void deletePins( std::vector<IoPin*>* pinList, uint pins );

        uint m_outValue;
        uint m_nextOutVal;
        std::queue<uint> m_outQueue;
        std::queue<uint64_t> m_timeQueue;
        //uint m_nextOutDir;

        uint64_t m_propDelay; // Propagation delay
        uint64_t m_timeLH;    // Time for Output voltage to switch from 10% to 90% (1 gate)
        uint64_t m_timeHL;    // Time for Output voltage to switch from 90% to 10% (1 gate)
        double m_propSize;    // Nunmber of gates for total Propagation delay
        //bool m_rndPD;         // Randomize Propagation Delay

        double m_inHighV;  // currently in eClockedDevice
        double m_inLowV;  // currently in eClockedDevice
        double m_ouHighV;
        double m_ouLowV;

        double m_inImp;  // currently in eClockedDevice
        double m_ouImp;

        bool m_openCol;
        bool m_invOutputs;
        bool m_invInputs;

        uint m_width;
        uint m_height;

        eElement* m_eElement;

        std::vector<IoPin*> m_inPin;
        std::vector<IoPin*> m_outPin;
        std::vector<IoPin*> m_otherPin;
};

#endif
