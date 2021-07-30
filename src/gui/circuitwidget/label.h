/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
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

#ifndef COMPLABEL_H
#define COMPLABEL_H

#include <QtWidgets>

class Component;

class Label : public QGraphicsTextItem
{
    friend class Component;

    Q_OBJECT
    public:
        Label();
        ~Label();

        void setComponent( Component* parent );
        void setLabelPos();

    public slots:
        void rotateCW();
        void rotateCCW();
        void rotate180();
        void H_flip( int hf );
        void V_flip( int vf );
        void updateGeometry(int, int, int) { document()->setTextWidth( -1 ); }

    protected:
        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );
        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        void focusOutEvent( QFocusEvent* event );

    private:
        Component* m_parentComp;

        int m_labelx;
        int m_labely;
        int m_labelrot;
};
#endif
