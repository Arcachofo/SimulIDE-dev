#ifndef TEXTCOMPONENT_H
#define TEXTCOMPONENT_H

#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT TextComponent : public Component
{
    public:
        TextComponent( QObject* parent, QString type, QString id );
        ~TextComponent();
        
        QRectF boundingRect() const override
        { 
            return QRectF( m_area.x()-m_border/2-1, m_area.y()-m_border/2-1, 
                           m_area.width()+m_border+2, m_area.height()+m_border+2 ); 
        }

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        virtual void updateStep() override;

        int  margin();
        void setMargin( int margin );
        
        int  border() { return m_border; }
        void setBorder( int border ) { m_border = border; update(); }
        
        int  fontSize() { return m_fontSize; }
        void setFontSize( int size );
        
        bool fixedW() { return m_fixedW; }
        void setFixedW( bool fixedW );

        QString getText();
        void    setText( QString text );
        
        QString getFont() { return m_font; }
        void    setFont( QString font );

        qreal opac() { return m_opac; }
        void setOpac( qreal op );

        QString getLinks();
        void setLinks( QString links );

        virtual void createLinks(QList<Component*>*compList ) override;

        virtual void compSelected( Component* comp ) override;

        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget ) override;


    public slots:
        void updateGeometry(int, int, int);
        void linkComp();

    protected:
        virtual void contextMenuEvent( QGraphicsSceneContextMenuEvent* event ) override;

    private:
        QGraphicsTextItem* m_text;
        QString m_textString;

        QList<Component*> m_linkedComp;

        qreal m_opac;
        
        int  m_fontSize;
        int  m_docMargin;
        int  m_margin;
        int  m_border;

        bool m_fixedW;
        bool m_context;
        
        QString m_font;
};

#endif // TEXTCOMPONENT_H
