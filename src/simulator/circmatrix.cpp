/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <iostream>
#include <QtMath>
//#include <iomanip> // setw()

#include "circmatrix.h"
#include "simulator.h"

CircMatrix* CircMatrix::m_pSelf = 0l;

CircMatrix::CircMatrix()
{
    m_pSelf = this;
    m_numEnodes = 0;
}
CircMatrix::~CircMatrix(){}

void CircMatrix::createMatrix( QList<eNode*> &eNodeList )
{
    m_eNodeList = &eNodeList;
    m_numEnodes = eNodeList.size();

    m_circMatrix.clear();
    m_coefVect.clear();

    m_circMatrix.resize( m_numEnodes , d_vector_t( m_numEnodes , 0 ) );
    m_coefVect.resize( m_numEnodes , 0 );

    //m_admitChanged = false;
    //m_currChanged  = false;

    qDebug() <<"\n  Initializing Matrix: "<< m_numEnodes << " eNodes";
    analyze();
}

void CircMatrix::addConnections( int enodNum, QList<int>* nodeGroup, QList<int>* allNodes )
{
    nodeGroup->append( enodNum );
    allNodes->removeOne( enodNum );

    eNode* enod = m_eNodeList->at( enodNum-1 );
    enod->setSingle( false );

    QList<int> cons = enod->getConnections();

    for( int nodeNum : cons )
    {
        if( nodeNum == 0 ) continue;
        if( !nodeGroup->contains( nodeNum ) ) addConnections( nodeNum, nodeGroup, allNodes );
    }
}

void CircMatrix::analyze()
{
    QList<int> allNodes;
    for( int i=0; i<m_numEnodes; i++ ) allNodes.append( i+1 );

    m_aList.clear();
    m_aFaList.clear();
    m_bList.clear();
    m_eNodeActList.clear();
    int group = 0;
    int singleNode = 0;

    while( !allNodes.isEmpty() ) // Get a list of groups of nodes interconnected
    {
        QList<int> nodeGroup;
        addConnections( allNodes.first(), &nodeGroup, &allNodes ); // Get a group of nodes interconnected

        int numEnodes = nodeGroup.size();
        if( numEnodes==1 )           // Sigle nodes do by themselves
        {
            eNode* enod = m_eNodeList->at( nodeGroup[0]-1 );
            enod->setSingle( true );
            singleNode++;
        }else{
            dp_matrix_t a;
            d_matrix_t ap;
            dp_vector_t b;
            QList<eNode*> eNodeActive;

            a.resize( numEnodes , dp_vector_t( numEnodes , 0 ) );
            ap.resize( numEnodes , d_vector_t( numEnodes , 0 ) );
            b.resize( numEnodes , 0 );

            int ny=0;
            for( int y=0; y<m_numEnodes; ++y )    // Copy data to reduced Matrix
            {
                if( !nodeGroup.contains( y+1 ) ) continue;
                int nx=0;
                for( int x=0; x<m_numEnodes; ++x )
                {
                    if( !nodeGroup.contains( x+1 ) ) continue;
                    a[nx][ny] = &(m_circMatrix[x][y]);
                    nx++;
                }
                b[ny] = &(m_coefVect[y]);
                eNode* node = m_eNodeList->at(y);
                node->setNodeGroup( group );
                eNodeActive.append( node );
                ny++;
            }
            m_aList.append( a );
            m_aFaList.append( ap );
            m_bList.append( b );
            m_eNodeActList.append( eNodeActive );
            group++;
        }
    }
    m_admitChanged.resize( group, true );
    m_currChanged.resize(  group, true );

    qDebug() <<"CircMatrix::solveMatrix"<<group<<"Circuits";
    qDebug() <<"CircMatrix::solveMatrix"<<singleNode<<"Single Nodes\n";
}

bool CircMatrix::solveMatrix()
{
    bool ok = true;
    for( int i=0; i<m_bList.size(); ++i )
    {
        if( !m_admitChanged[i] && !m_currChanged[i] ) continue;

        m_eNodeActive = &(m_eNodeActList[i]);
        int n = m_eNodeActive->size();

        if( n == 2 )
        {
            double a00 = *m_aList[i][0][0];
            double a01 = *m_aList[i][0][1];
            double a10 = *m_aList[i][1][0];
            double a11 = *m_aList[i][1][1];

            double det = ( a00 * a11 ) - ( a01 * a10 );
            if( det == 0 ) return false;

            double bi0 = *m_bList[i][0];
            double bi1 = *m_bList[i][1];

            double b0 = ( bi0 * a11 ) - ( a01 * bi1 );
            double b1 = ( a00 * bi1 ) - ( bi0 * a10 );

            m_eNodeActive->at(0)->setVolt( b0 / det );
            m_eNodeActive->at(1)->setVolt( b1 / det );
        }
        else{
            if( m_admitChanged[i] ) factorMatrix( n, i );
            if( !luSolve( n, i ) ) ok = false;
        }
        m_currChanged[i]  = false;
        m_admitChanged[i] = false;
    }
    return ok;
}

void CircMatrix::factorMatrix( int n, int group ) // Factor matrix into Lower/Upper triangular
{
    dp_matrix_t& ap = m_aList[group];
    d_matrix_t&   a = m_aFaList[group];

    /*std::cout << "\nAdmitance Matrix:\n"<< std::endl;
    for( int i=0; i<n; i++ )
    {
        for( int j=0; j<n; ++j ) { std::cout << std::setw(15); std::cout << *ap[i][j]; }
        std::cout << std::endl;
    }*/

    int row,col,k;

    for( col=0; col<n; ++col )              // Crout's method: loop through columns
    {
        for( row=0; row<col; ++row )        // Upper triangular elements
        {
            double q = *(ap[row][col]);
            for( k=0; k<row; ++k ) q -= a[row][k]*a[k][col];
            a[row][col] = q;
        }
        for( row=col; row<n; ++row )        // Lower triangular elements
        {
            double q = *(ap[row][col]);
            for( k=0; k<col; ++k ) q -= a[row][k]*a[k][col];
            a[row][col] = q;
        }
        if( col != n-1 )                    // Normalize column respect to diagonal
        {
            double div = a[col][col];
            if( div == 0 ) continue;
            for( row=col+1; row<n; ++row ) a[row][col] /= div;
        }
    }
    /*std::cout << "\nFactored Matrix:\n" << std::endl;
    for( int i=0; i<n; i++ )
    {
        for( int j=0; j<n; j++ ) { std::cout << std::setw(15); std::cout << a[i][j]; }
        std::cout << std::endl;
    }*/
}

bool CircMatrix::luSolve( int n, int group ) // Solves the system to get voltages for each node
{
    const d_matrix_t&  a  = m_aFaList[group];
    const dp_vector_t& bp = m_bList[group];

    /*std::cout << "\nCurrent vector:\n" << std::endl;
    for( int i=0; i<n; i++ )
    {
        std::cout << std::setw(15); std::cout << *bp[i];
        std::cout << std::endl;
    }*/

    d_vector_t b;
    b.resize( n , 0 );

    double tot;
    int i;
    for( i=0; i<n; ++i )
    {
        tot = *(bp[i]);
        b[i] = tot;
        if( tot != 0 ) break; // First nonzero b element
    }

    int bi = i++;
    for( ; i<n; ++i )
    {
        tot = *(bp[i]);
        for( int j=bi; j<i; ++j ) tot -= a[i][j]*b[j]; // Forward substitution from lower triangular matrix
        b[i] = tot;
    }
    bool isOk = true;

    for( i=n-1; i>=0; --i )
    {
        tot = b[i];
        for( int j=i+1; j<n; ++j ) tot -= a[i][j]*b[j]; // Back substitution from upper triangular matrix

        double div = a[i][i];
        double volt = 0;
        if( div != 0 ) volt = tot/div;
        else isOk = false;

        b[i] = volt;
        m_eNodeActive->at(i)->setVolt( volt );         // Set Node Voltages
    }
    return isOk;
}
