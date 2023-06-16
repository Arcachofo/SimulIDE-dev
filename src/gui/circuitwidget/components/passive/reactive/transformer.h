/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include "e-element.h"
#include "component.h"
#include "e-coil.h"

class LibraryItem;

class MAINMODULE_EXPORT Transformer : public Component, public eElement
{
        struct winding_t
        {
            QString definition;
            QString topology;
            QString prefix;
            QList<Pin*>   pins;
            QList<eCoil*> coils;
            int nCoils;
            int start;
            int angle;
            int x;
        };

    public:
        Transformer( QObject* parent, QString type, QString id );
        ~Transformer();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void stamp() override;
        virtual void runEvent() override;

        double baseInd() { return m_baseInd; }
        void setBaseInd( double i );

        QString primary() { return m_primary.definition; }
        void setPrimary( QString p );

        QString secondary() { return m_secondary.definition; }
        void setSecondary( QString s );

        double coupCoeff() { return m_coupCoeff; }
        void setCoupCoeff( double c );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    private:
        void setupInducts( winding_t* w );
        void createCoils();
        int  getLeght( winding_t* w );
        void modify( winding_t* w );
        void create( winding_t* w );

        int m_lenght;

        double m_baseInd;
        double m_coupCoeff;

        uint64_t m_reacStep;

        winding_t m_primary;
        winding_t m_secondary;

        QList<eCoil*> m_coils;

        std::vector<std::vector<double>> m_inva;
};
#endif
