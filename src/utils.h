/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef UTILS_H
#define UTILS_H

#include <QtMath>

class QDomDocument;
class QByteArray;
class QStringList;
class QString;
class QPointF;
class QPoint;
class Pin;

#define unitToVal( val, mult ) \
    if     ( mult == " n" ) val *= 1e3; \
    else if( mult == " u")  val *= 1e6; \
    else if( mult == " m" ) val *= 1e9; \
    else if( mult == " ")   val *= 1e12;

#define valToUnit( val, mult, decimals ) \
    mult = " p";\
    if( qFabs( val ) > 999 ) { \
        val /= 1e3; mult = " n"; \
        if( qFabs( val ) > 999 ) { \
            val /= 1e3; mult = " u"; \
            if( qFabs( val ) > 999 ) { \
                val /= 1e3; mult = " m"; \
                if( qFabs( val ) > 999 ) { \
                    val /= 1e3; mult = " "; \
                    if( qFabs( val ) > 999 ) { \
                        val /= 1e3; mult = " k"; \
                        if( qFabs( val ) > 999 ) { \
                            val /= 1e3; mult = " M"; \
                            if( qFabs( val ) > 999 ) { \
                                val /= 1e3; mult = " G"; \
    }}}}}}} \
    if     ( qFabs( val ) < 10)   decimals = 3; \
    else if( qFabs( val ) < 100)  decimals = 2; \
    else if( qFabs( val ) < 1000) decimals = 1;

double getMultiplier( QString mult );
QString multToValStr( double value, QString mult );

QString val2hex( int d );
QString toDigit( int d );
QString decToBase( int value, int base, int digits );

//---------------------------------------------------

void MessageBoxNB( const QString &title, const QString &message );

//---------------------------------------------------

QString addQuotes( const QString &string );
QString remQuotes( QString string );
QString getBareName( const QString &filepath );
QString getFileName( const QString &filepath );
QString getFileDir( const QString &filepath );
QString getFileExt( const QString &filepath );
QString changeExt( const QString &filepath, const QString &ext );

//---------------------------------------------------

QDomDocument fileToDomDoc( const QString &fileName, const QString &caller );
QString      fileToString( const QString &fileName, const QString &caller );
QStringList  fileToStringList( const QString &fileName, const QString &caller );
QByteArray   fileToByteArray( const QString &fileName, const QString &caller );

//---------------------------------------------------

int roundDown( int x, int roundness );
int snapToGrid( int x );
int snapToCompGrid( int x );
QPointF toGrid( QPointF point );
QPoint  toGrid( QPoint point );
QPointF toCompGrid( QPointF point );

bool lessPinX( Pin* pinA, Pin* pinB );
bool lessPinY( Pin* pinA, Pin* pinB );

QPointF getPointF( QString p );
QString getStrPointF( QPointF p );

#endif

