/***************************************************************************
 *   Copyright (C) 2019 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef SUBPACKAGE_H
#define SUBPACKAGE_H

#include "chip.h"
#include "linker.h"

class LibraryItem;
class QAction;

class SubPackage : public Chip, public Linker
{
        friend class Circuit;

    public:
        SubPackage( QString type, QString id );
        ~SubPackage();
        
 static Component* construct( QString type, QString id );
 static LibraryItem* libraryItem();

        int width() { return m_width; }
        void setWidth( int width );

        int height() { return m_height; }
        void setHeight( int height );

        QString packageFile() { return m_pkgeFile; }
        void setPackageFile( QString package );

        QString bckGndData() { return m_BckGndData; }
        virtual void setBckGndData( QString data ) override;
        virtual void setBackground( QString bck ) override;

        virtual void setLogicSymbol( bool ls ) override;

        QString packagePins();
        void setPackagePins( QString pinsStr );

        void setEventPin( Pin* pin ) { m_eventPin = pin; }

        void savePackage( QString fileName );

        QString subcTypeStr() { return m_enumUids.at( (int)m_subcType ); }
        void setSubcTypeStr( QString s );

        virtual void compSelected( Component* comp ) override;  // Use link mechanism to select main components

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* o, QWidget* w ) override;

        void invertPin( bool invert );
        void setPinId( QString id );
        void setPinName( QString name );
        void setPinAngle( int i );
        void setPinSpace( double space );
        void boardModeSlot();
        void setBoardMode( bool mode );
        void mainComp() { Linker::startLinking(); }
        void unusePin( bool unuse );
        void setPointPin( bool point );
        void setBusPin( bool bus );
        void editPin();
        void deleteEventPin();
    
    protected:
        void loadPackage();
        void slotSave();
        void editFinished( int r );

        void mousePressEvent( QGraphicsSceneMouseEvent* event ) override;
        void hoverMoveEvent( QGraphicsSceneHoverEvent* event ) override;
        void hoverLeaveEvent( QGraphicsSceneHoverEvent* event ) override;
        
        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu ) override;

        virtual void addNewPin( QString id, QString type, QString label,
                            int pos, int xpos, int ypos, int angle, int length=8, int space=0 ) override;

    private:
        QString pinEntry( Pin* pin );
        QString pinStrEntry( Pin* pin );
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

        QString m_pkgeFile;     // file containig package defs

        QString m_BckGndData;   // Embedded background png data

        Pin* m_eventPin;
        QList<Pin*> m_pkgePins;
};

#include <QDialog>

class QLabel;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;

class EditDialog : public QDialog
{
    public:
        EditDialog( SubPackage* pack, Pin* eventPin, QWidget* parent = 0 );

    private:
        void setPointPin( bool p );
        void setBusPin( bool b );

        SubPackage* m_package;

        QLabel*    m_nameLabel;
        QLineEdit* m_nameLineEdit;

        QLabel*    m_idLabel;
        QLineEdit* m_idLineEdit;

        QLabel*    m_spaceLabel;
        QDoubleSpinBox* m_spaceBox;

        QLabel*    m_angleLabel;
        QComboBox* m_angleBox;

        QCheckBox* m_invertCheckBox;
        QCheckBox* m_unuseCheckBox;
        QCheckBox* m_pointCheckBox;
        QCheckBox* m_busCheckBox;
};

#endif
