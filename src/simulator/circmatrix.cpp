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

    m_admitChanged = false;
    m_currChanged  = false;

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
                eNodeActive.append( m_eNodeList->at(y) );
                ny++;
            }
            m_aList.append( a );
            m_aFaList.append( ap );
            m_bList.append( b );
            m_eNodeActList.append( eNodeActive );
            group++;
        }
    }
    qDebug() <<"CircMatrix::solveMatrix"<<group<<"Circuits";
    qDebug() <<"CircMatrix::solveMatrix"<<singleNode<<"Single Nodes\n";
}

bool CircMatrix::solveMatrix()
{
    if( !m_admitChanged && !m_currChanged ) return true;

    for( int i=0; i<m_bList.size(); ++i )
    {
        m_eNodeActive = &(m_eNodeActList[i]);
        int n = m_eNodeActive->size();

        if( m_admitChanged ) factorMatrix( n, i );
        if( !luSolve( n, i ) ) return false;
    }
    m_currChanged  = false;
    m_admitChanged = false;
    return true;
}

void CircMatrix::factorMatrix( int n, int group ) // factors a matrix into upper and lower triangular matrices by gaussian elimination.
{
    dp_matrix_t&  ap  = m_aList[group];
    d_matrix_t& a = m_aFaList[group];

    /*std::cout << "\nAdmitance Matrix:\n"<< std::endl;
    for( int i=0; i<n; i++ )
    {
        for( int j=0; j<n; ++j ) { std::cout << std::setw(15); std::cout << a[i][j]; }
        std::cout << std::endl;
    }*/

    int i,j,k;

    for( j=0; j<n; ++j ) // use Crout's method; loop through the columns
    {
        for( i=0; i<j; ++i ) // calculate upper triangular elements for this column
        {
            double q = *(ap[i][j]);
            for( k=0; k<i; ++k ) q -= a[i][k]*a[k][j];
            a[i][j] = q;
        }
        for( i=j; i<n; ++i ) // calculate lower triangular elements for this column
        {
            double q = *(ap[i][j]);
            for( k=0; k<j; ++k ) q -= a[i][k]*a[k][j];
            a[i][j] = q;
        }
        if( j != n-1 )               // Normalize this column respect to a[j][j]
        {
            double div = a[j][j];
            if( div != 0.0 )         //a[j][j] = div = 1e-18; // avoid zeros in a[j][j]
                for( i=j+1; i<n; ++i ) a[i][j] /= div;
        }
    }
    /*std::cout << "\nFactored Matrix: << std::endl;
    for( int i=0; i<n; i++ )
    {
        for( int j=0; j<n; j++ ) { std::cout << std::setw(15); std::cout << a[i][j]; }
        std::cout << std::endl;
    }*/
}

bool CircMatrix::luSolve( int n, int group ) // Solves the set of n linear equations using a LU factorization previously performed by solveMatrix.
{                                            // On input, b[0..n-1] is the right hand side of the equations, and on output, contains the solution.
    const d_matrix_t&  a  = m_aFaList[group];
    const dp_vector_t& bp = m_bList[group];

    d_vector_t b;
    b.resize( n , 0 );

    double tot;
    int i;
    for( i=0; i<n; ++i )
    {
        tot = *(bp[i]);
        b[i] = tot;
        if( tot != 0 ) break; // find first nonzero b element
    }

    int bi = i++;
    for( ; i<n; ++i )
    {
        tot = *(bp[i]);
        for( int j=bi; j<i; ++j ) tot -= a[i][j]*b[j]; // forward substitution using the lower triangular matrix
        b[i] = tot;
    }
    bool isOk = true;
    
    for( i=n-1; i>=0; --i )
    {
        tot = b[i];
        for( int j=i+1; j<n; ++j ) tot -= a[i][j]*b[j]; // back-substitution using the upper triangular matrix

        double div = a[i][i];
        double volt = 0;
        if( div != 0 ) volt = tot/div;
        else isOk = false;

        b[i] = volt;
        m_eNodeActive->at(i)->setVolt( volt );      // Set Node Voltages
    }
    return isOk;
}
