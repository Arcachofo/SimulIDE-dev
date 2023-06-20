/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SUBPACKAGE_H
#define SUBPACKAGE_H

#include "chip.h"
#include "linkable.h"

class LibraryItem;
class QAction;

class MAINMODULE_EXPORT SubPackage : public Chip, public Linkable
{
        friend class Circuit;

    public:
        SubPackage( QObject* parent, QString type, QString id );
        ~SubPackage();
        
    static Component* construct( QObject* parent, QString type, QString id );
    static LibraryItem* libraryItem();

        int width() { return m_width; }
        void setWidth( int width );
        
        int height() { return m_height; }
        void setHeight( int height );
        
        QString  package();
        void setPackage( QString package );

        void setEventPin( Pin* pin ) { m_eventPin = pin; }

        void savePackage( QString fileName );
        virtual void setSubcTypeStr( QString s ) override;
        virtual void setLogicSymbol( bool ls ) override;
        virtual void remove() override;

        virtual void compSelected( Component* comp ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;

    public slots:
        void invertPin( bool invert );
        void setPinId( QString id );
        void setPinName( QString name );
        void setPinAngle( int i );
        void boardModeSlot();
        void setBoardMode( bool mode );
        void mainComp() { Linkable::startLinking(); }
        void unusePin( bool unuse );
        void pointPin( bool point );
        void editPin();
        void deleteEventPin();
    
    private slots:
        void loadPackage();
        void slotSave();
        void editFinished( int r );

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
        void hoverMoveEvent( QGraphicsSceneHoverEvent* event ) override;
        void hoverLeaveEvent( QGraphicsSceneHoverEvent* event ) override;
        
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

        virtual Pin* addPin( QString id, QString type, QString label,
                            int pos, int xpos, int ypos, int angle, int length=8 ) override;

    private:
        QString pinEntry( Pin* pin );
        QString adjustSize( QString str, int size );

 static QString m_lastPkg;

        bool m_fakePin; // Data for drawing pin when hovering
        
        int m_angle;  
        int m_p1X;
        int m_p1Y;
        int m_p2X;
        int m_p2Y;

        bool m_circPosSaved;
        bool m_boardMode;
        QAction* m_boardModeAction;

        Pin* m_eventPin;
        QList<Pin*> m_pkgePins;
};

#include <QDialog>

class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;

class EditDialog : public QDialog
{
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

        QLabel*    m_angleLabel;
        QComboBox* m_angleBox;

        QCheckBox* m_invertCheckBox;
        QCheckBox* m_unuseCheckBox;
        QCheckBox* m_pointCheckBox;
};

#endif

