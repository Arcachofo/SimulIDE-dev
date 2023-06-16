/***************************************************************************
 *   Copyright (C) 2018 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef FUNCTION_H
#define FUNCTION_H

#include "iocomponent.h"
#include "scriptmodule.h"

class LibraryItem;
class QPushButton;
class QGraphicsProxyWidget;

class MAINMODULE_EXPORT Function : public IoComponent, public ScriptModule
{
    public:
        Function( QObject* parent, QString type, QString id );
        ~Function();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void voltChanged() override;
        virtual void runEvent() override { IoComponent::runOutputs(); }

        QString functions() { return m_funcList.join(","); }
        void setFunctions( QString f );

        virtual void remove() override;
        
        void setNumInputs( int inputs );
        void setNumOutputs( int outs );

        bool   getInputState( int pin );
        double getInputVoltage( int pin );
        void   setOutputState( int pin, bool s );
        void   setOutputVoltage( int pin, double v );
        double getOutputVoltage( int pin );
        
    public slots:
        void onbuttonclicked();
        void loadData();
        void saveData();
        
    protected:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    private:
        void updateFunctions();
        void updateArea( uint ins, uint outs );

        asIScriptFunction* m_voltChanged;
        QStringList m_funcList;

        QList<QPushButton*> m_buttons;
        QList<QGraphicsProxyWidget*> m_proxys;

        QString m_lastDir;
};

#endif
