/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef CIRCMATRIX_H
#define CIRCMATRIX_H

#include <vector>
#include <QList>

#include "e-node.h"

class MAINMODULE_EXPORT CircMatrix
{
    typedef std::vector<int>                  i_vector_t;
    typedef std::vector<double>               d_vector_t;
    typedef std::vector<double*>              dp_vector_t;
    typedef std::vector<std::vector<double>>  d_matrix_t;
    typedef std::vector<std::vector<double*>> dp_matrix_t;
    
    public:
        CircMatrix();
        ~CircMatrix();

 static CircMatrix* self() { return m_pSelf; }

        void createMatrix( QList<eNode*> &eNodeList );
        bool solveMatrix();

        inline void stampMatrix( int row, int col, double value ){
            m_admitChanged = true;
            m_circMatrix[row-1][col-1] = value;      // eNode numbers start at 1
        }
        inline void stampCoef( int row, double value ){
            m_currChanged = true;
            m_coefVect[row-1] = value;
        }

    private:
 static CircMatrix* m_pSelf;
        
        void analyze();
        void addConnections( int enodNum, QList<int>* nodeGroup, QList<int>* allNodes );

        inline void factorMatrix( int n, int group );
        inline bool luSolve( int n, int group );

        int m_numEnodes;
        QList<eNode*>* m_eNodeList;
        
        QList<dp_matrix_t> m_aList;
        QList<d_matrix_t>  m_aFaList;
        QList<dp_vector_t> m_bList;
        
        QList<eNode*>*       m_eNodeActive;
        QList<QList<eNode*>> m_eNodeActList;

        d_matrix_t m_circMatrix;
        d_vector_t m_coefVect;
        
        bool m_admitChanged;
        bool m_currChanged;
};
 #endif


