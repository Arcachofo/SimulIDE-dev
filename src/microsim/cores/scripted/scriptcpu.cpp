/***************************************************************************
 *   Copyright (C) 2022 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QDebug>

#include "scriptcpu.h"
#include "angelscript.h"
#include "simulator.h"
#include "mcupin.h"
#include "mcuport.h"
#include "ioport.h"
#include "scriptusart.h"
#include "watcher.h"
#include "console.h"
#include "mcu.h"
#include "utils.h"

#include "scriptprop.h"

using namespace std;

ScriptCpu::ScriptCpu( eMcu* mcu )
         : ScriptBase( mcu->getId()+"-"+"ScriptCpu" )
         , McuCpu( mcu )
{
    m_watcher  = nullptr;

    m_progWordMask = 0;
    for( uint i=0; i<mcu->wordSize(); ++i )
    {
        m_progWordMask <<= 8;
        m_progWordMask |= 0xFF;
    }

    // Script functions
    m_reset       = NULL;
    m_voltChanged = NULL;
    m_runEvent    = NULL;
    m_extClock    = NULL;
    m_extClockF   = NULL;
    m_INTERRUPT   = NULL;
    m_getCpuReg   = NULL;
    m_getStrReg   = NULL;
    m_command     = NULL;
    m_setLinkedVal= NULL;
    m_setLinkedStr= NULL;
    m_vChangedCtx = NULL;
    m_runEventCtx = NULL;
    m_extClockCtx = NULL;
    m_runStepCtx  = NULL;

    m_mcuComp = m_mcu->component();

    //m_aEngine->RegisterObjectType("pod", sizeof(pod), asOBJ_VALUE | asOBJ_POD);

    m_aEngine->RegisterObjectType("ScriptCpu", 0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterGlobalProperty("ScriptCpu component", this );

    m_aEngine->RegisterObjectType("eElement", 0, asOBJ_REF | asOBJ_NOCOUNT );
    m_aEngine->RegisterGlobalProperty("eElement element", this );

    m_typeWords.insert("IoPort" , IoPort::registerScript( m_aEngine ) );
    m_typeWords.insert("IoPin"  , IoPin::registerScript( m_aEngine ) );
    m_typeWords.insert("McuPort", McuPort::registerScript( m_aEngine ) );
    m_typeWords.insert("McuPin" , McuPin::registerScript( m_aEngine ) );

    m_types = m_typeWords.keys();

    QStringList memberList;

    memberList << "addCpuReg( string name, string type )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void addCpuReg(string n, string t)"
                                   , asMETHODPR( ScriptCpu, addCpuReg, (string, string), void)
                                   , asCALL_THISCALL );

    memberList << "addCpuVar( string name, string type )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void addCpuVar( string n, string t )"
                                   , asMETHODPR( ScriptCpu, addCpuVar, (string, string), void)
                                   , asCALL_THISCALL );

    memberList << "toConsole( string message )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void toConsole( string r )"
                                   , asMETHODPR( ScriptCpu, toConsole, (string), void)
                                   , asCALL_THISCALL );

    memberList << "showValue( string property )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void showValue( string r )"
                                   , asMETHODPR( ScriptCpu, showValue, (string), void)
                                   , asCALL_THISCALL );

    memberList << "addEvent( uint time )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void addEvent(uint t)"
                                   , asMETHODPR( ScriptCpu, addEvent, (uint), void)
                                   , asCALL_THISCALL );

    memberList << "cancelEvents()";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void cancelEvents()"
                                   , asMETHOD( ScriptCpu, cancelEvents )
                                   , asCALL_THISCALL );

    memberList << "circTime()";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "uint64 circTime()"
                                   , asMETHODPR( ScriptCpu, circTime, (), uint64_t)
                                   , asCALL_THISCALL );

    memberList << "readPGM( uint address )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "int readPGM(uint n)"
                                   , asMETHODPR( ScriptCpu, readPGM, (uint), int)
                                   , asCALL_THISCALL );

    memberList << "writePGM( uint address, int value )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void writePGM(uint a, int v)"
                                   , asMETHODPR( ScriptCpu, writePGM, (uint, int), void)
                                   , asCALL_THISCALL );

    memberList << "readRAM( uint address )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "int readRAM(uint n)"
                                   , asMETHODPR( ScriptCpu, readRAM, (uint), int)
                                   , asCALL_THISCALL );

    memberList << "writeRAM( uint address, int value )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void writeRAM(uint a, int v)"
                                   , asMETHODPR( ScriptCpu, writeRAM, (uint, int), void)
                                   , asCALL_THISCALL );

    memberList << "readROM( uint address )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "int readROM(uint n)"
                                   , asMETHODPR( ScriptCpu, readROM, (uint), int)
                                   , asCALL_THISCALL );

    memberList << "writeROM( uint address, int value )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void writeROM(uint a, int v)"
                                   , asMETHODPR( ScriptCpu, writeROM, (uint, int), void)
                                   , asCALL_THISCALL );

    memberList << "getPort( const string port )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "IoPort@ getPort(const string port)"
                                   , asMETHODPR( ScriptCpu, getPort, (const string), IoPort*)
                                   , asCALL_THISCALL );

    memberList << "getPin( const string pin )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "IoPin@ getPin(const string pin)"
                                   , asMETHODPR( ScriptCpu, getPin, (const string), IoPin*)
                                   , asCALL_THISCALL );

    memberList << "getMcuPort( const string port )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "McuPort@ getMcuPort(const string port)"
                                   , asMETHODPR( ScriptCpu, getMcuPort, (const string), McuPort*)
                                   , asCALL_THISCALL );

    memberList << "getMcuPin(const string pin)";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "McuPin@ getMcuPin(const string pin)"
                                   , asMETHODPR( ScriptCpu, getMcuPin, (const string), McuPin*)
                                   , asCALL_THISCALL );

    memberList << "INTERRUPT( uint vector )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void INTERRUPT( uint vector )"
                                   , asMETHODPR( ScriptCpu, INTERRUPT, (uint32_t), void)
                                   , asCALL_THISCALL );

    memberList << "getPropStr( int index, const string name )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "string getPropStr( int index, const string p )"
                                   , asMETHODPR( ScriptCpu, getPropStr, (int,const string), string)
                                   , asCALL_THISCALL );

    memberList << "setPropStr( int index, const string name, const string value )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void setPropStr( int index, const string p,const string v )"
                                   , asMETHODPR( ScriptCpu, setPropStr, (int,const string,const string), void)
                                   , asCALL_THISCALL );

    memberList << "setLinkedValue( int index, double value, int i )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void setLinkedValue( int index, double v, int i )"
                                   , asMETHODPR( ScriptCpu, setLinkedValue, (int,double,int), void)
                                   , asCALL_THISCALL );

    memberList << "setLinkedString( int index, string valuie, int i )";
    m_aEngine->RegisterObjectMethod("ScriptCpu", "void setLinkedString( int index, string v, int i )"
                                   , asMETHODPR( ScriptCpu, setLinkedString, (int,const string,int), void)
                                   , asCALL_THISCALL );

    m_typeWords.insert("ScriptCpu", memberList );
}
ScriptCpu::~ScriptCpu()
{
    if( m_vChangedCtx ) m_vChangedCtx->Release();
    if( m_runEventCtx ) m_runEventCtx->Release();
    if( m_extClockCtx ) m_extClockCtx->Release();
    if( m_runStepCtx  ) m_runStepCtx->Release();
}

void ScriptCpu::setPeriferals( std::vector<ScriptPerif*> p )
{
    m_periferals = p;
    for( ScriptPerif* perif : m_periferals )
        m_typeWords.insert( perif->type(), perif->registerScript( this ) );
}

void ScriptCpu::setScriptFile( QString scriptFile, bool compile )
{
    ScriptBase::setScriptFile( scriptFile, compile );
    if( compile ) compileScript();
}

#include "as_module.h"

int ScriptCpu::compileScript()
{
    if( !m_aEngine ) return -1;

    int r = ScriptBase::compileScript();
    if( r < 0 ) return r;

    asCModule* module = (asCModule*)m_aEngine->GetModule( 0 );

    m_memberWords.clear();
    m_memberWords.insert( "component", m_typeWords.value("ScriptCpu") );
    m_memberWords.insert( "element", QStringList() );

    int n = module->GetGlobalVarCount();

    for( int i=0; i<n; ++i )
    {
        const char* name;
        const char* type;
        if( module->getGlobalVarData( i, &name, &type ) != asERROR )
        {
            QStringList list = m_typeWords.value( QString( type ) ); //
            m_memberWords.insert( QString( name ), list );
            //qDebug() << name << type;
        }
    }

    m_reset       = module->GetFunctionByDecl("void reset()");
    m_voltChanged = module->GetFunctionByDecl("void voltChanged()");
    m_updateStep  = module->GetFunctionByDecl("void updateStep()");
    m_runEvent    = module->GetFunctionByDecl("void runEvent()");
    m_INTERRUPT   = module->GetFunctionByDecl("void INTERRUPT( uint vector )");
    m_runStep     = module->GetFunctionByDecl("void runStep()");
    m_extClock    = module->GetFunctionByDecl("void extClock( bool clkState )");
    m_extClockF   = module->GetFunctionByDecl("void extClock()");
    m_getCpuReg   = module->GetFunctionByDecl("int getCpuReg( string reg )");
    m_getStrReg   = module->GetFunctionByDecl("string getStrReg( string reg )");
    m_command     = module->GetFunctionByDecl("void command( string c )");
    m_setLinkedVal= module->GetFunctionByDecl("void setLinkedValue( double v, int i )");
    m_setLinkedStr= module->GetFunctionByDecl("void setLinkedString( string str, int i )");

    m_vChangedCtx = m_voltChanged ? m_aEngine->CreateContext() : NULL;
    m_runEventCtx = m_runEvent    ? m_aEngine->CreateContext() : NULL;
    m_extClockCtx = m_extClockF   ? m_aEngine->CreateContext() : NULL;
    m_runStepCtx  = m_runStep     ? m_aEngine->CreateContext() : NULL;

    for( ComProperty* p : m_scriptProps ) // Get properties getters and setters from script
    {
        QString propName = p->name();
        QString type = p->type();

        QString getter = type+" get"+propName+"()";
        asIScriptFunction* asFunc = m_aEngine->GetModule(0)->GetFunctionByDecl( getter.toLocal8Bit().constData() );
        m_propGetters[propName] = asFunc;

        QString setter = "void set"+propName+"("+type+")";
        asFunc = m_aEngine->GetModule(0)->GetFunctionByDecl( setter.toLocal8Bit().constData() );
        m_propSetters[propName] = asFunc;
    }

    if( m_getCpuReg || m_getStrReg )
    {
        m_mcu->createWatcher( this );
    }

    for( ScriptPerif* perif : m_periferals ) perif->startScript();

    asIScriptFunction* func = m_aEngine->GetModule(0)->GetFunctionByDecl("void setup()");
    if( func ) callFunction( func );

    return 0;
}

void ScriptCpu::updateStep()
{
    if( m_updateStep ) callFunction( m_updateStep );

    m_aEngine->GarbageCollect( asGC_FULL_CYCLE );  // Automatic garbage collection is disabled, doing it manually

    if( !m_changed ) return;
    m_changed = false;
    m_mcuComp->setValLabelText( m_value );
}

void ScriptCpu::reset()
{
    m_watcher = m_mcu->getWatcher();

    m_mcuComp->setShowVal( true );
    m_value = "";

    if( m_reset ) callFunction( m_reset );
}
void ScriptCpu::voltChanged()
{
    if( !m_voltChanged ) return;

#ifndef SIMULIDE_W32 // Defined in .pro file for win32
    m_status = m_vChangedCtx->executeJit0( m_voltChanged );
#else
    m_status = callFunction0( m_voltChanged, m_vChangedCtx );
#endif
    if( m_status != asEXECUTION_FINISHED ) printError( m_vChangedCtx );
}

void ScriptCpu::runEvent()
{
    if( !m_runEvent ) return;
#ifndef SIMULIDE_W32 // Defined in .pro file for win32
    m_status = m_runEventCtx->executeJit0( m_runEvent );
#else
    m_status = callFunction0( m_runEvent, m_runEventCtx );
#endif
    if( m_status != asEXECUTION_FINISHED ) printError( m_runEventCtx );
}

void ScriptCpu::INTERRUPT( uint vector )
{
    prepare( m_INTERRUPT );
    m_context->SetArgDWord( 0, vector );
    //m_context->SetArgAddress( 0, &vector ); // Not working
    execute();
}

void ScriptCpu::runStep()
{
    if( !m_runStep ) return;
    m_mcu->cyclesDone = 1;
#ifndef SIMULIDE_W32 // Defined in .pro file for win32
    m_status = m_runStepCtx->executeJit0( m_runStep );
#else
    m_status = callFunction0( m_runStep, m_runStepCtx );
#endif
    if( m_status != asEXECUTION_FINISHED ) printError( m_runStepCtx );
}

void ScriptCpu::extClock( bool clkState )
{
    if( m_extClockF )
    {
#ifndef SIMULIDE_W32 // Defined in .pro file for win32
        m_status = m_extClockCtx->executeJit0( m_extClockF );
#else
        m_status = callFunction0( m_extClockF , m_extClockCtx );
#endif
        if( m_status != asEXECUTION_FINISHED ) printError( m_extClockCtx );
    }
    if( !m_extClock ) return;

    prepare( m_extClock );
    m_context->SetArgByte( 0, clkState );
    execute();
}

void ScriptCpu::command( QString c )
{
    if( c.isEmpty() ) return;
    if( !m_command ) return;

    prepare( m_command );
    std::string str = c.toStdString();
    m_context->SetArgObject( 0, &str );
    execute();

    //if( m_status != asEXECUTION_FINISHED ) return;
    //str = *(string*)m_context->GetReturnObject();
    //return QString::fromStdString( str );
}

void ScriptCpu::toConsole( string r )
{
    if( !m_watcher ) return;
    Console* c = m_watcher->console();
    if( c ) c->appendText( QString::fromStdString(r) );
}

void ScriptCpu::showValue( string r )
{
    m_value = QString::fromStdString(r);
    m_changed = true;
}

void ScriptCpu::addCpuReg( string name, string type )
{
    if( !m_watcher ) return;
    m_watcher->addRegister( QString::fromStdString( name ), QString::fromStdString( type ) );
}

void ScriptCpu::addCpuVar( string name, string type )
{
    if( !m_watcher ) return;
    m_watcher->addVariable( QString::fromStdString( name ), QString::fromStdString( type ) );
}

int ScriptCpu::getCpuReg( QString reg )
{
    if( !m_getCpuReg ) return 0;

    prepare( m_getCpuReg );
    std::string str = reg.toStdString();
    m_context->SetArgObject( 0, &str );
    execute();

    if( m_status != asEXECUTION_FINISHED ) return 0;
    asQWORD ret = m_context->GetReturnQWord();
    return ret;
}

QString ScriptCpu::getStrReg( QString val )
{
    if( !m_getStrReg ) return "";

    prepare( m_getStrReg );
    std::string str = val.toStdString();
    m_context->SetArgObject( 0, &str );
    execute();

    if( m_status != asEXECUTION_FINISHED ) return "";
    str = *(string*)m_context->GetReturnObject();
    return QString::fromStdString( str );
}

void ScriptCpu::addProperty( QString group, QString name, QString type, QString unit )
{
    ComProperty* p = new ScriptProp<ScriptCpu>( name, name, unit, this
                                              , &ScriptCpu::getProp, &ScriptCpu::setProp, type );
    m_scriptProps.push_back( p );

    m_mcu->component()->addProperty( group, p );
}

QString ScriptCpu::getProp( ComProperty* p )
{
    QString name = p->name();
    QString type = p->type();

    asIScriptFunction* asFunc = m_propGetters.value( name );

    if( !asFunc ) return "";

    prepare( asFunc );
    execute();

    if( m_status != asEXECUTION_FINISHED ) return "";

    if( type == "string" )
    {
        std::string str = *(string*)m_context->GetReturnObject();
        return QString::fromStdString( str );
    }
    else if( type == "int" )
    {
        int64_t ret = m_context->GetReturnQWord();
        return QString::number( ret );
    }
    else if( type == "uint" )
    {
        uint64_t ret = m_context->GetReturnQWord();
        return QString::number( ret );
    }
    else if( type == "double" )
    {
        double ret = m_context->GetReturnDouble();
        return QString::number( ret );
    }
    else if( type == "bool")
    {
        asBYTE ret = m_context->GetReturnByte();
        QString retStr = ret ? "true" : "false";
        return retStr;
    }
    return "";
}

void ScriptCpu::setProp( ComProperty* p, QString val )
{
    QString name = p->name();
    QString type = p->type();

    asIScriptFunction* asFunc = m_propSetters.value( name );

    if( !asFunc ) return; // Repeated in command(), create a function for this
    prepare( asFunc );
    if( type == "string" )
    {
        std::string str = val.toStdString();
        m_context->SetArgObject( 0, &str );
    }
    else if( type == "bool"   ) m_context->SetArgByte(   0, val == "true" );
    else{
        double multiplier = 1;
        QStringList l = val.split(" ");
        val = l.first();
        if( l.size() > 1 ) multiplier = getMultiplier( l.at(1) );
        if     ( type == "int"    ) m_context->SetArgDWord(  0, val.toInt()*multiplier );
        else if( type == "uint"   ) m_context->SetArgDWord(  0, val.toUInt()*multiplier );
        else if( type == "double" ) m_context->SetArgDouble( 0, val.toDouble()*multiplier );
    }
    execute();
}

void ScriptCpu::addEvent( uint time ) { Simulator::self()->addEvent( time, this ); }
void ScriptCpu::cancelEvents()        { Simulator::self()->cancelEvents( this ); }
uint64_t ScriptCpu::circTime()        { return Simulator::self()->circTime(); }

int  ScriptCpu::readPGM( uint addr )         { if( addr < m_progSize       ) return m_progMem[addr] & m_progWordMask ; return -1; }
void ScriptCpu::writePGM( uint addr, int v ) { if( addr < m_progSize       ) m_progMem[addr] = v & m_progWordMask; }
int  ScriptCpu::readRAM( uint addr )         { if( addr <= m_dataMemEnd    ) return m_dataMem[addr]; return -1; }
void ScriptCpu::writeRAM( uint addr, int v ) { SET_RAM( addr, v ); }
int  ScriptCpu::readROM( uint addr )         { if( addr < m_mcu->romSize() ) return m_mcu->getRomValue( addr ); return -1; }
void ScriptCpu::writeROM( uint addr, int v ) { if( addr < m_mcu->romSize() ) m_mcu->setRomValue( addr, v ); }

IoPort* ScriptCpu::getPort( const string portName )
{
    QString name = QString::fromStdString( portName );
    IoPort* port = m_mcu->getIoPort( name );
    if( !port ) qDebug() << "Error: ScriptCpu::getPort Port"<< name << "Doesn't exist";
    return port;
}

IoPin* ScriptCpu::getPin( const string pinName )
{
    QString name = QString::fromStdString( pinName );
    IoPin* pin = m_mcu->getIoPin( name );
    if( !pin ) qDebug() << "Error: ScriptCpu::getPin Pin"<< name << "Doesn't exist";
    return pin;
}

McuPort* ScriptCpu::getMcuPort( const string portName )
{
    QString name = QString::fromStdString( portName );
    McuPort* port = m_mcu->getMcuPort( name );
    if( !port ) qDebug() << "Error: ScriptCpu::getMcuPort Port"<< name << "Doesn't exist";
    return port;
}

McuPin* ScriptCpu::getMcuPin( const string pinName )
{
    QString name = QString::fromStdString( pinName );
    McuPin* pin = m_mcu->getMcuPin( name );
    if( !pin ) qDebug() << "Error: ScriptCpu::getMcuPin Pin"<< name << "Doesn't exist";
    return pin;
}

//---- Linked --------------------------------------------

string ScriptCpu::getPropStr( int index, const string p  )
{
    Component* comp = m_mcuComp->getLinkedComp( index );
    if( !comp ) return "";

    QString propValue = comp->getPropStr( QString::fromStdString(p) );
    return propValue.toStdString();
}

void ScriptCpu::setPropStr( int index, const string p, const string v ) // Script should call this in updateStep()
{
    Component* comp = m_mcuComp->getLinkedComp( index );
    if( !comp ) return;

    comp->setPropStr( QString::fromStdString(p), QString::fromStdString(v) );
}

void ScriptCpu::setLinkedValue( int index, double v, int i )
{
    Component* comp = m_mcuComp->getLinkedComp( index );
    if( !comp ) return;

    comp->setLinkedValue( v, i );
}

void ScriptCpu::setLinkedString( int index, string str, int i )
{
    Component* comp = m_mcuComp->getLinkedComp( index );
    if( !comp ) return;

    comp->setLinkedString( QString::fromStdString( str ), i );
}

void ScriptCpu::setLinkedVal( double v, int i )
{
    if( !m_setLinkedVal ) return;
    if( !Simulator::self()->isRunning() ) return;

    prepare( m_setLinkedVal );
    m_context->SetArgDouble( 0, v );
    m_context->SetArgDWord( 1, i );
    execute();
}

void ScriptCpu::setLinkedStr( QString s, int i )
{
    if( !m_setLinkedStr ) return;
    if( !Simulator::self()->isRunning() ) return;

    prepare( m_setLinkedStr );
    std::string str = s.toStdString();
    m_context->SetArgObject( 0, &str );
    m_context->SetArgDWord( 1, i );
    execute();
}
