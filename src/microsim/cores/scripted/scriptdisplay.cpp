/***************************************************************************
 *   Copyright (C) 2023 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>

#include "scriptdisplay.h"
#include "scriptcpu.h"
#include "simulator.h"

ScriptDisplay::ScriptDisplay( int w, int h, QString name, QWidget* parent )
             : QWidget( parent )
             , ScriptPerif()
             , Updatable()
             , eElement( name )
             , m_image( w, h, QImage::Format_RGB888 )
{
    m_width  = w;
    m_height = h;
    m_name   = name;
    m_scale  = 0;

    m_background= 0;
    m_changed = false;

    m_clear = NULL;

    m_image.fill( 0 );

    this->setFixedSize( m_width, m_height );
}
ScriptDisplay::~ScriptDisplay(){}

void ScriptDisplay::registerScript( ScriptCpu* cpu )
{
    m_scriptCpu = cpu;

    asIScriptEngine* engine = cpu->engine();

    asITypeInfo* info = engine->GetTypeInfoByName("Display"); // Check if "Display is already registered
    if( !info ) registerScriptMetods( engine );

    string display = "Display "+m_name.toStdString(); // Type name
    engine->RegisterGlobalProperty( display.c_str(), this );
}

void ScriptDisplay::registerScriptMetods( asIScriptEngine* engine ) // Static: register Object type and methods only once
{
    engine->RegisterObjectType("Display", 0, asOBJ_REF | asOBJ_NOCOUNT );

    engine->RegisterObjectMethod("Display", "void setWidth(int w)"
                                   , asMETHODPR( ScriptDisplay, setWidth, (int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("Display", "void setHeight(int h)"
                                   , asMETHODPR( ScriptDisplay, setHeight, (int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("Display", "void setBackground(int b)"
                                   , asMETHODPR( ScriptDisplay, setBackground, (int), void)
                                   , asCALL_THISCALL );

    engine->RegisterObjectMethod("Display", "void setPixel(int x, int y, int color)"
                                   , asMETHODPR( ScriptDisplay, setPixel, (int,int,int), void)
                                   , asCALL_THISCALL );
}

void ScriptDisplay::startScript()
{
    asIScriptEngine* aEngine = m_scriptCpu->engine();

    m_clear = aEngine->GetModule(0)->GetFunctionByDecl("void clear()");
    //m_frameSent    = aEngine->GetModule(0)->GetFunctionByDecl("void frameSent( uint data )");
}

void ScriptDisplay::initialize()
{
    if( m_clear ) m_scriptCpu->callFunction( m_clear );
}

void ScriptDisplay::updateStep()
{
    if( m_changed )
    {
        m_changed = false;
        updtImageSize();
    }
    update();
}

void ScriptDisplay::setWidth( int w )
{
    if( m_width == w || w < 1 ) return;
    m_width = w;
    m_changed = true;
}

void ScriptDisplay::setHeight( int h )
{
    if( m_height == h || h < 1 ) return;
    m_height = h;
    m_changed = true;
}

void ScriptDisplay::setSize( int w, int h )
{
    if( w < 1 || h < 1 ) return;
    m_width  = w;
    m_height = h;
    m_changed = true;
}

void ScriptDisplay::setMonitorScale( double scale )
{
    if( scale <= 0 ) return;
    m_scale = scale;

    this->setFixedSize( m_width*scale, m_height*scale );

    Simulator::self()->addToUpdateList( this );

    show();
}

void ScriptDisplay::setBackground( int b )
{
    if( m_background == b ) return;
    m_background = b;
    m_image.fill( m_background );
}

void ScriptDisplay::setPixel( int x, int y, int color )
{
    if(x > m_width || y > m_height ) return;
    m_image.setPixel( x, y, QColor(color).rgb() );
}

void ScriptDisplay::updtImageSize()
{
    m_image = m_image.scaled( m_width, m_height );
    m_image.fill( m_background );
    this->setFixedSize( m_width, m_height );
}

void ScriptDisplay::paintEvent( QPaintEvent* )
{
    if( !m_scale ) return;

    QPainter p(this);

    p.drawImage( 0, 0, m_image.scaled( width(), height() ) );
}

