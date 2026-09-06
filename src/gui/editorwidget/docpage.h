/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#pragma once

#include <QWidget>

class QFrame;
class QLabel;
class QPushButton;
class CodeEditor;

class DocPage : public QWidget
{
    Q_OBJECT

    public:
        DocPage( CodeEditor* editor, QWidget* parent = nullptr );

        CodeEditor* editor() { return m_editor; }

    public slots:
        void showModifiedBar( QString path, bool docModified );
        void showDeletedBar( QString path );
        void hideChangeBar();

    private slots:
        void reloadBar();
        void keepBar();
        void closeTabBar();
        void keepOpenBar();

    private:
        CodeEditor*  m_editor;
        QFrame*      m_changedBar;
        QLabel*      m_changedLabel;
        QPushButton* m_reloadBtn;
        QPushButton* m_keepBtn;
        QPushButton* m_closeBtn;
        QPushButton* m_keepOpenBtn;
};