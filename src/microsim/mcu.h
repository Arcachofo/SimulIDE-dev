/***************************************************************************
 *   Copyright (C) 2020 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MCU_H
#define MCU_H

//#include <QHash>

#include "e_mcu.h"
#include "chip.h"
#include "linkable.h"

enum deviceType_t{
    typeNONE=0,
    typeIOU,
    typeMPU,
    typeMCU
};

class LibraryItem;
class MCUMonitor;
class ScriptCpu;

class MAINMODULE_EXPORT Mcu : public Chip, public Linkable
{
        friend class McuCreator;

    public:
        Mcu( QObject* parent, QString type, QString id );
        ~Mcu();

 static Mcu* self() { return m_pSelf; }
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        void setup( QString type );

        virtual bool setPropStr( QString prop, QString val ) override;

        QString program() { return m_eMcu.getFileName(); }
        void setProgram( QString pro );

        bool autoLoad() { return m_autoLoad; }
        void setAutoLoad( bool al ) { m_autoLoad = al; }

        double extFreq() { return m_extFreq; }
        void setExtFreq( double freq ) { m_extFreq = freq; setFreq( freq ); }

        double freq() { return m_eMcu.m_freq; }
        void setFreq( double freq ) { m_eMcu.setFreq( freq ); }

        bool rstPinEnabled();
        void enableRstPin( bool en );

        bool extOscEnabled();
        void enableExtOsc( bool en );

        bool wdtEnabled();
        void enableWdt( bool en );

        bool clockOut();
        void setClockOut( bool clkOut );

        QString varList();
        void setVarList( QString vl );

        QString cpuRegs();
        void setCpuRegs( QString vl );

        void setEeprom( QString eep );
        QString getEeprom();

        bool saveEepr() { return m_eMcu.m_saveEepr; }
        void setSaveEepr( bool s ) { m_eMcu.m_saveEepr = s; }

        int serialMon();
        void setSerialMon( int s );

        virtual void initialize() override;
        virtual void stamp() override;
        virtual void updateStep() override;
        virtual void voltChanged() override;

        QString device() { return m_device; }
        bool isScripted() { return m_scripted; }
        CpuBase* cpu() { return m_eMcu.cpu; }

        void reset() { m_eMcu.hardReset( true ); }

        bool load( QString fileName );

        virtual Pin* addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle , int length=8) override;

        //void createCfgWord( QString name, uint16_t addr, uint16_t v );
        void setScriptLinkable( ScriptCpu* l) { m_scriptLink = l; } // Called from script
        void setLinkedVal( int index, int v, int i=0 );             // Called from script
        void setLinkedStr( int index, QString str, int i=0 );       // Called from script

        virtual void setLinkedValue( int v, int i=0  ) override;
        virtual void setLinkedString( QString str, int i=0 ) override;

        virtual QStringList getEnumUids( QString ) override;
        virtual QStringList getEnumNames( QString ) override;

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void slotmain();
        void slotLoad();
        void slotReload();
        void slotOpenTerm( int num );
        void slotOpenMcuMonitor();
        void slotLinkComp();

        void loadEEPROM();
        void saveEEPROM();

    protected:
 static Mcu* m_pSelf;

        virtual void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

        deviceType_t m_deviceType;

        bool m_autoLoad;
        bool m_scripted;
        bool m_resetPol;

        double m_extFreq;

        int m_serialMon;

        QString m_lastFirmDir;  // Last firmware folder used
        QString m_dataFile;
        QString m_device;       // Name of device

        eMcu m_eMcu;

        IoPin*  m_resetPin;
        McuPin* m_portRstPin;

        QList<Pin*> m_pinList;

        MCUMonitor* m_mcuMonitor;

        ScriptCpu* m_scriptLink;
};

#endif
