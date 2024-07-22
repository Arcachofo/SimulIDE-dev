/***************************************************************************
 *   Copyright (C) 2021 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef IOCOMPONENT_H
#define IOCOMPONENT_H

#include <queue>

#include "component.h"
#include "logicfamily.h"

class eElement;
class IoPin;

class IoComponent : public Component, public LogicFamily
{
    public:
        IoComponent( QString type, QString id );
        ~IoComponent();

        QList<ComProperty*> inputProps();
        QList<ComProperty*> outputProps();
        QList<ComProperty*> outputType();
        QList<ComProperty*> edgeProps();

        virtual void setup() override;

        void initState();
        void runOutputs();
        void scheduleOutPuts( eElement* el );

        void updtProperties();

        virtual void setSupplyV( double v ) override;
        virtual void setInpHighV( double volt ) override;
        virtual void setInpLowV( double volt ) override;
        virtual void setOutHighV( double volt ) override;
        virtual void setOutLowV( double volt ) override;
        virtual void setInputImp( double imp ) override;
        virtual void setOutImp( double imp ) override;
        virtual void setRiseTime( double time ) override;
        virtual void setFallTime( double time ) override;

        bool invertOuts() { return m_invOutputs; }
        void setInvertOuts( bool invert );

        bool invertInps() { return m_invInputs; }
        virtual void setInvertInps( bool invert );

        QString invertedPins();
        void setInvertPins( QString p );

        int  numInps() { return m_inPin.size(); }
        virtual void setNumInps( uint pins, QString label="I", int bit0=0, int id0=0 );

        int  numOuts() { return m_outPin.size(); }
        virtual void setNumOuts( uint pins, QString label="O", int bit0=0, int id0=0 );

        bool openCol() { return m_openCol; }
        void setOpenCol( bool op );

        void init( QStringList pins );
        void initPin( IoPin* pin );

        virtual std::vector<Pin*> getPins() override;

        virtual void remove() override;

        virtual void setHidden( bool hid, bool hidArea=false, bool hidLabel=false ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

    protected:
        virtual void slotProperties() override;

        IoPin* createPin( QString data, QString id );
        void setupPin( IoPin *pin, QString data );
        void setNumPins( std::vector<IoPin*>* pinList, uint pins, QString label, int bit0, bool out, int id0 );
        void deletePins( std::vector<IoPin*>* pinList, int pins );

        virtual void updtOutPins();
        virtual void updtInPins();

        uint m_outValue;
        uint m_nextOutVal;
        std::queue<uint> m_outQueue;
        std::queue<uint64_t> m_timeQueue;

        bool m_openCol;
        bool m_invOutputs;
        bool m_invInputs;

        //bool m_familyAdded;

        QString m_invertPins;

        uint m_width;
        uint m_height;

        eElement* m_eElement;

        std::vector<IoPin*> m_inPin;
        std::vector<IoPin*> m_outPin;
        std::vector<IoPin*> m_otherPin;
};

#endif
