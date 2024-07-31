/***************************************************************************
 *   Copyright (C) 2024 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include "logicfamily.h"
#include "mainwindow.h"
#include "utils.h"
#include "proputils.h"

QMap<QString, logicFamily_t> LogicFamily::m_families;

LogicFamily::LogicFamily()
{
    m_blocked = false;
    m_enableSupply = true;

    m_supplyV = 5.0; // Power Supply
    m_inHighVp = 0.5;
    m_inHighV = 2.5;
    m_inLowVp = 0.5;
    m_inLowV  = 2.5;
    m_ouHighVp = 1;
    m_ouHighV = 5;
    m_ouLowVp = 0;
    m_ouLowV  = 0;

    m_inImp = 1e9;
    m_ouImp = 40;

    m_delayMult = 1;
    m_delayBase = 10*1000; // 10 ns
    m_timeLH    = 3000;
    m_timeHL    = 4000;

    m_family = "Custom";
    if( m_families.isEmpty() ) getFamilies();
}
LogicFamily::~LogicFamily(){}

void LogicFamily::setInpHighV( double v )
{
    if( m_blocked ) return;
    m_inHighVp = v/m_supplyV;
}

void LogicFamily::setInpLowV( double v )
{
    if( m_blocked ) return;
    m_inLowVp = v/m_supplyV;
}

void LogicFamily::setOutHighV( double v )
{
    if( m_blocked ) return;
    m_ouHighVp = v/m_supplyV;
}

void LogicFamily::setOutLowV( double v )
{
    if( m_blocked ) return;
    m_ouLowVp = v/m_supplyV;
}

void LogicFamily::setPropDelay( double pd )
{
    if( pd < 0 ) pd = 0;
    if( pd > 1e6   ) pd = 1e6;
    m_delayBase = pd*1e12;
}

void LogicFamily::setRiseTime( double time )
{
    if( time < 1e-12 ) time = 1e-12;
    if( time > 1e6   ) time = 1e6;
    m_timeLH = time*1e12;
}

void LogicFamily::setFallTime( double time )
{
    if( time < 1e-12 ) time = 1e-12;
    if( time > 1e6   ) time = 1e6;
    m_timeHL = time*1e12;
}

void LogicFamily::setFamily( QString f )
{
    m_family = f;
    if( f == "Custom") setSupplyV( m_supplyV );
    else               setFamilyData( m_families.value( f ) );
}

void LogicFamily::setFamilyData( logicFamily_t lf )
{
    double supplyV = lf.supply;
    m_enableSupply = supplyV < 0;
    if( !m_enableSupply ) m_supplyV = supplyV;

    setSupplyV( m_supplyV );

    m_inHighVp = lf.inpLHp;
    m_inLowVp  = lf.inpHLp;
    m_inImp    = lf.inpImp;

    m_ouHighVp = lf.outHip;
    m_ouLowVp  = lf.outLop;
    m_ouImp    = lf.outImp;

    updateData();
}

void LogicFamily::updateData()
{
    m_blocked = true;

    setInpHighV( m_supplyV * m_inHighVp );
    setInpLowV(  m_supplyV * m_inLowVp );
    setInputImp( m_inImp );

    setOutHighV( m_supplyV * m_ouHighVp);
    setOutLowV(  m_supplyV * m_ouLowVp );
    setOutImp(   m_ouImp );
    m_blocked = false;
}

void LogicFamily::getFamilies() // Static
{
    m_families["Default"] = { -1, 0.5, 0.5, 1e9, 1, 0, 40 };
    m_families["Custom"]  = { -1, 0.5, 0.5, 1e9, 1, 0, 40 };

    QString modelsFile = MainWindow::self()->getDataFilePath("logic_families.model");
    if( !QFile::exists( modelsFile ) ) return;

    QString doc = fileToString( modelsFile, "LogicFamily::getFamilies()" );
    QVector<QStringRef> docLines = doc.splitRef("\n");
    for( QStringRef line : docLines )
    {
        QVector<propStr_t> properties = parseProps( line );
        if( properties.isEmpty() ) break;
        QString familyName = properties.takeFirst().name.toString();
        logicFamily_t family;

        for( propStr_t property : properties )
        {
            QStringRef propName = property.name;
            double  propValue = property.value.toDouble();
            if     ( propName == "supply") family.supply = propValue;
            else if( propName == "inpLHp") family.inpLHp = propValue;
            else if( propName == "inpHLp") family.inpHLp = propValue;
            else if( propName == "inpImp") family.inpImp = propValue;
            else if( propName == "outHip") family.outHip = propValue;
            else if( propName == "outLop") family.outLop = propValue;
            else if( propName == "outImp") family.outImp = propValue;
        }
        m_families[familyName] = family;
    }
}
