/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class QTextDocument;

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

    public:
        Highlighter( QTextDocument* parent = 0);
        ~Highlighter();

        QStringList readSyntaxFile( QString fileName );

        void addObjects( QStringList patterns );
        void addMembers( QStringList patterns );
        void setMembers( QStringList patterns );
        void setExtraTypes( QStringList types );

    protected:
        void highlightBlock( const QString &text );

    private:
        struct HighlightRule
        {
            QRegExp pattern;
            QTextCharFormat format;
        };
        void addRule( QTextCharFormat, QString );
        void processRule( HighlightRule rule, QString lcText );

        bool m_multiline;
        
        QVector<HighlightRule> m_rules;
        QVector<HighlightRule> m_objectRules;
        QVector<HighlightRule> m_memberRules;
        QVector<HighlightRule> m_extraRules;

        QRegExp m_multiStart;
        QRegExp m_multiEnd;
        QTextCharFormat m_multiFormat;
};

#endif
