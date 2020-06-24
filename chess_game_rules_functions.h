/**
   * File name: chess_game_rules_functions.h
   * File type: header file (.h)
*/

#ifndef CHESS_GAME_RULES_FUNCTIONS_H
#define CHESS_GAME_RULES_FUNCTIONS_H

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

/*--------------------------------------1----------------------------------------*/
/*----------The dimensions corresponding to the matrix and vectors used----------*/
/*----------Dimensiuni corespunzatoare matricilor si vectorilor folositi----------*/
#define NEWSENSORVALUESMATRIX_NR_ROWS 8
#define NEWSENSORVALUESMATRIX_NR_COLUMNS 8
#define NEWLEDMATRIX_NR_ROWS 8
#define NEWLEDMATRIX_NR_COLUMNS 8
#define DIAGONALTREATFIELDS_LENGTH 8
/*----------Dimensiuni corespunzatoare matricilor si vectorilor folositi----------*/
/*----------The dimensions corresponding to the matrix and vectors used----------*/
/*--------------------------------------1----------------------------------------*/



/*------------------------------2--------------------------------*/
/*----------The values corresponding to the moving part----------*/
/*----------Valorile corespunzatoare partii care este la mutare----------*/
#define WHITE_MOVES 1
#define BLACK_MOVES 0
/*----------Valorile corespunzatoare partii care este la mutare----------*/
/*----------The values corresponding to the moving part----------*/
/*------------------------------2--------------------------------*/



/*----------------------3--------------------*/
/*----------Chess pieces definition----------*/
/*----------Definirea pieselor de sah----------*/

#define NO_PIECE 0

#define WHITE_PAWN 1
#define WHITE_KNIGHT 2
#define WHITE_BISHOP 3
#define WHITE_ROOK 4
#define WHITE_QUEEN 5
#define WHITE_KING 6

#define BLACK_PAWN 11
#define BLACK_KNIGHT 12
#define BLACK_BISHOP 13
#define BLACK_ROOK 14
#define BLACK_QUEEN 15
#define BLACK_KING 16

/*----------Definirea pieselor de sah----------*/
/*----------Chess pieces definition----------*/
/*----------------------3--------------------*/



/*-------------------------------------4---------------------------------------*/
/*----------Defining the data type used for all matrix in the program----------*/
/*----------Definirea tipului de date folosit pentru toate matricile din program----------*/
struct Matrix_uint8_t
{
	uint8_t values[8][8];
};
typedef struct Matrix_uint8_t Matrix_uint8_t;
/*----------Definirea tipului de date folosit pentru toate matricile din program----------*/
/*----------Defining the data type used for all matrix in the program----------*/
/*-------------------------------------4---------------------------------------*/

/*---------------------------------------------6--------------------------------------------------*/
/*----------Declaring the global variables used to implement the rules of the chess game----------*/
/*----------Declararea variabilelor globale folosite pentru implementarea regulilor jocului de sah----------*/


extern Matrix_uint8_t newSensorValuesMatrix; /* Matrice care stocheaza valorile citite de la senzor la fiecare modificare a acestora. */
extern Matrix_uint8_t previousSensorValuesMatrix; /* Matrice ce stocheaza valorile citite de la senzor de la mutarea anterioara. */
extern Matrix_uint8_t currentSensorValuesMatrix; /* Matrice ce stocheaza valorile citite de la senzor de la mutarea curenta. */

/* Matrice folosita pentru stocarea pozitiilor initiale ale pieselor. Folosita la inceputul jocului de sah si dupa resetarea tablei de sah. */
extern Matrix_uint8_t initialPiecesPositionMatrix;

/* Matrice folosita pentru stocarea pozitiilor curente ale pieselor. Aceasta matrice permite diferentierea tipului de piesa. */
extern Matrix_uint8_t piecesPositionMatrix;

/* Matrice folosita pentru determinarea mutarilor posibile la ridicarea piesei curente de pe pozitia de inceput de mutare (nu neaparat cea de la mutarea 1). */
extern Matrix_uint8_t currentPiecePossibleMove;

/* Matrice ce stocheaza valorile necesare pentru aprinderea led-urilor in functie de piesa curenta ridicata si de mutarile posibile ale acesteia. */
extern Matrix_uint8_t newLedMatrix;

/* Structura folosita pentru decodarea valorilor necesare pentru aprinderea led-urilor */
extern struct cRGB newLedMatrixRGB[NEWLEDMATRIX_NR_ROWS][NEWLEDMATRIX_NR_COLUMNS];

/*----------Declararea variabilelor globale folosite pentru implementarea regulilor jocului de sah----------*/
/*----------Declaring the global variables used to implement the rules of the chess game----------*/
/*---------------------------------------------6--------------------------------------------------*/



/*-------------------------------------------7---------------------------------------------*/
/*----------Declaring the functions used to implement the rules of the chess game----------*/
/*----------Declararea functiilor folosite pentru implementarea regulilor jocului de sah----------*/

/* Functie de decodare a campurilor pe care sunt plasati senzorii. (conversie variabila uint64_t in matrice 8 x 8 de tip Matrix_uint8_t). */
Matrix_uint8_t sensorPositionDecoder(uint64_t oldSensorValues);

/* Functie de decodare a campurilor pe care sunt plasate LED-urile. (conversie matrice 8 x 8 de tip Matrix_uint8_t in vector cu 64 de elemente)*/
void WS2812_ledPositionDecoder(Matrix_uint8_t local_newLedMatrix, uint8_t color);

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un pion. */
void setLedsForPossibleMovesPawn(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn);

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un cal. */
void setLedsForPossibleMovesKnight(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn);

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un nebun. */
extern void setLedsForPossibleMovesBishop(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn);

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un turn. */
void setLedsForPossibleMovesRook(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn, uint8_t local_whiteCastleInProgress, uint8_t local_blackCastleInProgress);

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutata o dama. */
void setLedsForPossibleMovesQueen(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn);

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un rege. */
void setLedsForPossibleMovesKing(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn, uint8_t local_isCurrentKing);

/* Functie ce implementeaza urmatoarele operatii matriceale: rezultat = a - (b * c); urmate de impunerea unor conditii necesare pentru logica functiei */
/* "eliminatePossibleChecksForCurrentKing()".																										   */
Matrix_uint8_t Matrix_uint8_t_operation_substract(Matrix_uint8_t a, Matrix_uint8_t b, Matrix_uint8_t c, uint8_t abc_nr_rows, uint8_t abc_nr_columns);

/* Functie de resetare a valorilor unei matrici. (elementele sunt setate la valoarea 0) */
Matrix_uint8_t Matrix_uint8_t_reset_values(Matrix_uint8_t a, uint8_t a_nr_rows, uint8_t a_nr_columns);

/* Functie de eliminare a campurilor ocupate/atacate de catre toate piesele adverse din numarul total de mutari posibile ale regelui ce trebuie mutat. */
Matrix_uint8_t eliminatePossibleChecksForCurrentKing(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn);

/*----------Declararea functiilor folosite pentru implementarea regulilor jocului de sah----------*/
/*----------Declaring the functions used to implement the rules of the chess game----------*/
/*-------------------------------------------7---------------------------------------------*/



#endif
