/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#ifndef MATRIXSOLVER_H
#define MATRIXSOLVER_H

#include <vector>

    // info about each row/column of the matrix for simplification purposes
class RowInfo 
{
    public:
        RowInfo() { type = ROW_NORMAL; }
        
        static const int ROW_NORMAL = 0;  // ordinary value
        static const int ROW_CONST  = 1;  // value is constant
        static const int ROW_EQUAL  = 2;  // value is equal to another value
        
        int nodeEq, type, mapCol, mapRow;
        double value;
        
        bool rsChanges; // row's right side changes
        bool lsChanges; // row's left side changes
        bool dropRow;   // row is not needed in matrix
        
};

class MatrixSolver
{
    public:
        MatrixSolver();
        ~MatrixSolver();
        
        void simplifyMatrix( std::vector<std::vector<double> > circuitMatrix, std::vector<double>  circuitRightSide );
        
};


 #endif
