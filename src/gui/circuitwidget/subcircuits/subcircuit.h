/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SUBCIRCUIT_H
#define SUBCIRCUIT_H

#include "chip.h"

class Tunnel;
class LibraryItem;

class MAINMODULE_EXPORT SubCircuit : public Chip
{
    public:
        SubCircuit( QObject* parent, QString type, QString id );
        ~SubCircuit();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void remove() override;

        virtual void setLogicSymbol( bool ls ) override;

        QList<Tunnel*> getPinTunnels() { return m_pinTunnels.values(); }
        Component* getMainComp( QString name="" );

        virtual QString toString() override;

        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

    protected:
        void loadSubCircuit( QString file );

        virtual Pin* addPin( QString id, QString type, QString label,
                            int pos, int xpos, int ypos, int angle, int length=8 ) override;

        virtual Pin* updatePin( QString id, QString type, QString label,
                                int xpos, int ypos, int angle, int length=8  ) override;

        QHash<QString, Component*> m_mainComponents;
        static QString m_subcDir;      // Subcircuit Path

        QList<Component*>       m_compList;
        QList<Tunnel*>          m_subcTunnels;
        QHash<QString, Tunnel*> m_pinTunnels;
};
#endif

