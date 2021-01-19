/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#ifndef SUBPACKAGE_H
#define SUBPACKAGE_H

#include "chip.h"

class LibraryItem;

class MAINMODULE_EXPORT SubPackage : public Chip
{
    Q_OBJECT
    Q_PROPERTY( QString  Package_File  READ package    WRITE setPackage  DESIGNABLE true  USER true )
    Q_PROPERTY( int      Width         READ width      WRITE setWidth    DESIGNABLE true USER true )
    Q_PROPERTY( int      Height        READ height     WRITE setHeight   DESIGNABLE true USER true )
    Q_PROPERTY( QString  Background    READ backGround WRITE setBackground DESIGNABLE true USER true )
    
    public:

        SubPackage( QObject* parent, QString type, QString id );
        ~SubPackage();
        
    static Component* construct( QObject* parent, QString type, QString id );
    static LibraryItem *libraryItem();
        
        virtual QList<propGroup_t> propGroups() override;

        int width();
        void setWidth( int width );
        
        int height();
        void setHeight( int height );
        
        QString  package();
        void setPackage( QString package );

        void savePackage( QString fileName );

        virtual void setLogicSymbol( bool ls );
        virtual void remove() override;
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        void invertPin( bool invert );
        void setPinId( QString id );
        void setPinName( QString name );
        void boardMode();
        void savingCirc();
        void mainComp();
    
    private slots:
        void loadPackage();
        void movePin();
        void editPin();
        void deleteEventPin();
        void unusePin( bool unuse );
        void pointPin( bool point );
        void slotSave();
        void editFinished( int r );
        
        
    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void hoverMoveEvent( QGraphicsSceneHoverEvent* event ) ;
        void hoverLeaveEvent( QGraphicsSceneHoverEvent* event );
        
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );

    private:
        QString pinEntry( Pin* pin, int pP );

        void setBoardMode();

 static QString m_lastPkg;

        bool m_movePin;
        bool m_fakePin; // Data for drawing pin when hovering
        
        int m_angle;  
        int m_p1X;
        int m_p1Y;
        int m_p2X;
        int m_p2Y;

        bool m_circPosSaved;
        bool m_boardMode;
        QAction* m_boardModeAction;
        //bool m_hideGrComp;
        //bool m_parentGrComp;
        //QAction* m_hideAction;

        Pin* m_eventPin;
};

class EditDialog : public QDialog
{
    Q_OBJECT

public:
    EditDialog( SubPackage* pack, Pin* eventPin, QWidget* parent = 0 );

private slots:
    void invertPin( bool invert );

private:
    SubPackage* m_package;

    QLabel*    m_nameLabel;
    QLineEdit* m_nameLineEdit;

    QLabel*    m_idLabel;
    QLineEdit* m_idLineEdit;

    QCheckBox* m_invertCheckBox;
    QCheckBox* m_unuseCheckBox;
    QCheckBox* m_pointCheckBox;
};

#endif

