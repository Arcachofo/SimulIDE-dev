#ifndef TEXTCOMPONENT_H
#define TEXTCOMPONENT_H

#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT TextComponent : public Component
{
    Q_OBJECT
    public:
        TextComponent( QObject* parent, QString type, QString id );
        ~TextComponent();
        
        QRectF boundingRect() const 
        { 
            return QRectF( m_area.x()-m_border/2-1, m_area.y()-m_border/2-1, 
                           m_area.width()+m_border+2, m_area.height()+m_border+2 ); 
        }

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        int  margin();
        void setMargin( int margin );
        
        int  border() { return m_border; }
        void setBorder( int border ) { m_border = border; }
        
        int  fontSize() { return m_fontSize; }
        void setFontSize( int size );
        
        bool fixedW() { return m_fixedW; }
        void setFixedW( bool fixedW );

        QString getText();
        void    setText( QString text );
        
        QString getFont() { return m_font; }
        void    setFont( QString font );

        qreal opac() { return m_opac; }
        void setOpac( qreal op ) { m_opac = op; update(); }

        virtual void setHidden( bool hid, bool hidLabel=false ) override;

        bool eventFilter( QObject* object, QEvent* event );

        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );


    public slots:
        void updateGeometry(int, int, int);

    protected:
        void mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event );

    private:
        QGraphicsTextItem* m_text;

        qreal m_opac;
        
        int  m_fontSize;
        int  m_margin;
        int  m_border;

        bool m_fixedW;
        
        QString m_font;
};

#endif // TEXTCOMPONENT_H
