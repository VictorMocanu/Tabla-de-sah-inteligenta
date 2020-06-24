/**
   * File name: chess_game_rules_functions.c
   * File type: source code file (.c)
*/

#include "chess_game_rules_functions.h"



/*---------------------------------------------1-------------------------------------------------*/
/*----------Defining the global variables used to implement the rules of the chess game----------*/
/*----------Definirea variabilelor globale folosite pentru implementarea regulilor jocului de sah----------*/

Matrix_uint8_t newSensorValuesMatrix; /* Matrice care stocheaza valorile citite de la senzor la fiecare modificare a acestora. */
Matrix_uint8_t previousSensorValuesMatrix; /* Matrice ce stocheaza valorile citite de la senzor de la mutarea anterioara. */
Matrix_uint8_t currentSensorValuesMatrix; /* Matrice ce stocheaza valorile citite de la senzor de la mutarea curenta. */

/* Matrice folosita pentru stocarea pozitiilor initiale ale pieselor. Folosita la inceputul jocului de sah si dupa resetarea tablei de sah. */

/*      A            B             C             D           E            F             G            H       */
Matrix_uint8_t initialPiecesPositionMatrix =
{
	WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK,      /* 1 */
	WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,  NO_PIECE, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,      /* 2 */
	NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,    NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,        /* 3 */
	NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,    WHITE_PAWN,   NO_PIECE,     NO_PIECE,     NO_PIECE,        /* 4 */
	NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,    NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,        /* 5 */
	NO_PIECE,   NO_PIECE,     BLACK_PAWN,     NO_PIECE,    NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,        /* 6 */
	BLACK_PAWN, BLACK_PAWN,   NO_PIECE,   BLACK_PAWN,  BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,      /* 7 */
	BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK       /* 8 */
};                                                                                                                 
/* Matrice folosita pentru stocarea pozitiilor curente ale pieselor. Aceasta matrice permite diferentierea tipului de piesa. */

/*      A            B             C             D           E            F             G            H       */
Matrix_uint8_t piecesPositionMatrix =
{
	WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK,      /* 1 */
	WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,  NO_PIECE, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,      /* 2 */
	NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,    NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,        /* 3 */
	NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,    WHITE_PAWN,   NO_PIECE,     NO_PIECE,     NO_PIECE,        /* 4 */
	NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,    NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,        /* 5 */
	NO_PIECE,   NO_PIECE,     BLACK_PAWN,     NO_PIECE,    NO_PIECE,   NO_PIECE,     NO_PIECE,     NO_PIECE,        /* 6 */
	BLACK_PAWN, BLACK_PAWN,   NO_PIECE,   BLACK_PAWN,  BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,      /* 7 */
	BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK       /* 8 */
};

/* Matrice folosita pentru determinarea mutarilor posibile la ridicarea piesei curente de pe pozitia de inceput de mutare (nu neaparat cea de la mutarea 1). */
Matrix_uint8_t currentPiecePossibleMove =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/* Matrice ce stocheaza valorile necesare pentru aprinderea led-urilor in functie de piesa curenta ridicata si de mutarile posibile ale acesteia. */
Matrix_uint8_t newLedMatrix =
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/* Structura folosita pentru decodarea valorilor necesare pentru aprinderea led-urilor */
struct cRGB newLedMatrixRGB[NEWLEDMATRIX_NR_ROWS][NEWLEDMATRIX_NR_COLUMNS];

/*----------Definirea variabilelor globale folosite pentru implementarea regulilor jocului de sah----------*/
/*----------Defining the global variables used to implement the rules of the chess game----------*/
/*---------------------------------------------1-------------------------------------------------*/


/*-------------------------------------------2--------------------------------------------*/
/*----------Defining the functions used to implement the rules of the chess game----------*/
/*----------Definirea functiilor folosite pentru implementarea regulilor jocului de sah----------*/

/* Functie de decodare a campurilor pe care sunt plasati senzorii. (conversie variabila uint64_t in matrice 8 x 8 de tip Matrix_uint8_t). */
Matrix_uint8_t sensorPositionDecoder(uint64_t oldSensorValues)
{
	uint64_t oldSensorValuesCurrentBit[64];
	uint8_t oldSensorValuesCurrentBitIndex[64] = { 27, 31, 19, 23, 11, 15, 3, 7,   26, 30, 18, 22, 10, 14, 2, 6,
	25, 29, 17, 21, 9, 13, 1, 5,   24, 28, 16, 20, 8, 12, 0, 4,   32, 36, 40, 44, 48, 52, 56, 60,   33, 37, 41, 45, 49, 53, 57, 61,
	34, 38, 42, 46, 50, 54, 58, 62,   35, 39, 43, 47, 51, 55, 59, 63 };
	uint64_t extractBitsMask = 0x0000000000000001;
	Matrix_uint8_t newSensorValuesMatrix;
	//uint64_t newSensorValues = 0x0000000000000000;
	for (uint8_t i = 0; i < 64; i++)
	{
		oldSensorValuesCurrentBit[i] = ((oldSensorValues & (extractBitsMask << i)) >> i);
	}

	for (uint8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
	{
		for (uint8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
		{
			//newSensorValuesMatrix.values[i][j] = (uint8_t)(oldSensorValuesCurrentBit[oldSensorValuesCurrentBitIndex[64 - 1 - (i * NEWSENSORVALUESMATRIX_NR_COLUMNS + j)]]);
			newSensorValuesMatrix.values[i][j] = (uint8_t)(oldSensorValuesCurrentBit[oldSensorValuesCurrentBitIndex[(i * NEWSENSORVALUESMATRIX_NR_COLUMNS + j)]]); 
		}
	}

	/* Vector of bits read from sensors. / Single 64-bit value with bits read from sensors. */
	/*
	for (uint8_t i = 0; i < 64; i++)
	{
		newSensorValues = newSensorValues | (oldSensorValuesCurrentBit[oldSensorValuesCurrentBitIndex[i]] << 64 - 1 - i);
		//newSensorValuesBitsVector[64 - 1 - i] = oldSensorValuesCurrentBit[oldSensorValuesCurrentBitIndex[i]];
	}
	*/
	return newSensorValuesMatrix;
	//return newSensorValuesBitsVector; /* Vector of bits read from sensors. */
}

/* Functie de decodare a campurilor pe care sunt plasate LED-urile. (conversie matrice 8 x 8 de tip Matrix_uint8_t in vector cu 64 de elemente)*/
void WS2812_ledPositionDecoder(Matrix_uint8_t local_newLedMatrix, uint8_t color)
{
	uint8_t ledVectorOrderOfIndexes[64] = { 56, 57, 58, 59, 60, 61, 62, 63,   55, 54, 53, 52, 51, 50, 49, 48,   40, 41, 42, 43, 44, 45, 46, 47,
	39, 38, 37, 36, 35, 34, 33, 32,   24, 25, 26, 27, 28, 29, 30, 31,   23, 22, 21, 20, 19, 18, 17, 16,   8, 9, 10, 11, 12, 13, 14, 15,
	7, 6, 5, 4, 3, 2, 1, 0 };

	/* Storing the LED settings in a 64-element vector. */
	/*
	for (uint8_t i = 0; i < sizeof(led) / sizeof(struct cRGB); i++)
	{
		led[ledVectorOrderOfIndexes[i]] = newLedValues[i];
	}
	*/

	for (uint8_t i = 0; i < NEWLEDMATRIX_NR_ROWS; i++)
	{
		for (uint8_t j = 0; j < NEWLEDMATRIX_NR_COLUMNS; j++)
		{
			if (local_newLedMatrix.values[i][j] != 0)
			{
				switch(color)
				{
					case RED:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
						break;
					}
					case GREEN:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
						break;
					}
					case BLUE:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
						break;
					}
					case WHITE:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
						break;
					}
					case YELLOW:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
						break;
					}
					case PURPLE:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
						break;
					}
					case TURQOISE:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
						break;
					}
					case ORANGE:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 150;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 50;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
						break;
					}
					case PINK_COLOR:
					{
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
						led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 128;
						break;
					}
					default:
					{
                        //Do nothing!
					}
				}
			}
			else 
			{
				led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
				led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
				led[ledVectorOrderOfIndexes[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
			}
		}
	}
	ws2812_setleds(led, MAXPIX); /* enable current led settings */
}

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un pion. */
void setLedsForPossibleMovesPawn(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn)
{
	if (local_currentPieceMove == WHITE_PAWN)
	{
		if ((local_currentPieceRow >= 1) && (local_currentPieceRow < 7))
		{
			/* Put the white pawn back on its current position.*/
			currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn] = 103;//set newLedMatrix[local_currentPieceRow][local_currentPieceColumn];
			newLedMatrix.values[local_currentPieceRow][local_currentPieceColumn] = 103;

			/* Move the white pawn forward with one field.*/
			if ((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS)
			{
				/* Check that no other pieces occupies the front field. */
				if (piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] == NO_PIECE)
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn] = 103;//set newLedMatrix[local_currentPieceRow + 1][local_currentPieceColumn];
					newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] = 103;
				}
			}

			/* Move the white pawn forward with two fields.*/
			if ((local_currentPieceRow + 2) < NEWSENSORVALUESMATRIX_NR_ROWS)
			{
				/* Check that the pawn is in the starting position. */
				/* Check that no other pieces occupies the front field. */
				if ((local_currentPieceRow == 1) && (piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn] == NO_PIECE))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn] = 103;//set newLedMatrix[local_currentPieceRow + 2][local_currentPieceColumn];
					newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn] = 103;
				}
			}

			/* Captures a black piece on the diagonal on the left side. */
			if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn - 1) >= 0))
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] <= BLACK_KING)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;//set newLedMatrix[local_currentPieceRow + 1][local_currentPieceColumn - 1]
					newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] <= WHITE_QUEEN)))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;//set newLedMatrix[local_currentPieceRow + 1][local_currentPieceColumn - 1]
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] == WHITE_KING)
						{
							/* Do nothing. */
						}
					}
				}
			}

			/* Captures a black piece on the diagonal on the right side. */
			if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
			{

				if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] <= BLACK_KING)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;//set newLedMatrix[local_currentPieceRow + 1][local_currentPieceColumn + 1]
					newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] <= WHITE_QUEEN)))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;//set newLedMatrix[local_currentPieceRow + 1][local_currentPieceColumn - 1]
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] == WHITE_KING)
						{
							/* Do nothing. */
						}
					}
				}
			}
		}
	}
	else
	{
		/* Case: local_currentPieceMove == BLACK_PAWN. */

		if ((local_currentPieceRow >= 1) && (local_currentPieceRow < 7))
		{
			/* Put the black pawn back on its current position.*/
			currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn] = 103;//set newLedMatrix[local_currentPieceRow][local_currentPieceColumn];
			newLedMatrix.values[local_currentPieceRow][local_currentPieceColumn] = 103;

			/* Move the black pawn forward with one field.*/
			if ((local_currentPieceRow - 1) >= 0)
			{
				/* Check that no other pieces occupies the front field. */
				if (piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] == NO_PIECE)
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn] = 103;//set newLedMatrix[local_currentPieceRow - 1][local_currentPieceColumn];
					newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] = 103;
				}
			}

			/* Move the black pawn forward with two fields.*/
			if ((local_currentPieceRow - 2) >= 0)
			{
				/* Check that the pawn is in the starting position. */
				/* Check that no other pieces occupies the front field. */
				if ((local_currentPieceRow == 6) && (piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn] == NO_PIECE))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn] = 103;//set newLedMatrix[local_currentPieceRow - 2][local_currentPieceColumn];
					newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn] = 103;
				}
			}

			/* Captures a white piece on the diagonal on the left side. */
			if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn - 1) >= 0))
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] <= WHITE_KING)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;//set newLedMatrix[local_currentPieceRow - 1][local_currentPieceColumn - 1]
					newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] <= BLACK_QUEEN)))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;//set newLedMatrix[local_currentPieceRow + 1][local_currentPieceColumn - 1]
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] == BLACK_KING)
						{
							/* Do nothing. */
						}
					}
				}
			}

			/* Captures a white piece on the diagonal on the right side. */
			if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] <= WHITE_KING)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;//set newLedMatrix[local_currentPieceRow - 1][local_currentPieceColumn + 1]
					newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] <= BLACK_QUEEN)))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;//set newLedMatrix[local_currentPieceRow + 1][local_currentPieceColumn - 1]
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] == BLACK_KING)
						{
							/* Do nothing. */
						}
					}
				}
			}
		}
	}
}

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un cal. */
void setLedsForPossibleMovesKnight(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn)
{
	if (local_currentPieceMove == WHITE_KNIGHT)
	{
		/* Put the white knight back on its current position.                                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][e][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*      Row ->   0  1  2  3  4  5  6  7                                                */
		currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn] = 103;//set newLedMatrix[local_currentPieceRow][local_currentPieceColumn];
		newLedMatrix.values[local_currentPieceRow][local_currentPieceColumn] = 103;

		/* Move knight in this way ( [current_row - 2][current_column - 1] ):                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][x][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][x][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][f][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*      Row ->   0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow - 2) >= 0) && ((local_currentPieceColumn - 1) >= 0))
		{
			if ((piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] <= BLACK_KING))
			{
				currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
				newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] <= WHITE_QUEEN))
				{
					/* Protects an allied piece. */
					currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if (piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] == WHITE_KING)
					{
						/* Do nothing. */
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
						newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
					}
				}
			}
		}

		/* Move knight in this way ( [current_row - 1][current_column - 2] ):                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][x][x][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][f][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*      Row ->   0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn - 2) >= 0))
		{
			if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] <= BLACK_KING))
			{
				currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
				newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] <= WHITE_QUEEN))
				{
					/* Protects an allied piece. */
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
				}
				else
				{
					if (piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] == WHITE_KING)
					{
						/* Do nothing. */
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
						newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
					}
				}
			}
		}

		/* Move knight in this way ( [current_row + 2][current_column - 1] ):                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][f][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][x][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][x][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*      Row ->   0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow + 2) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn - 1) >= 0))
		{
			if ((piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] <= BLACK_KING))
			{
				currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
				newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] <= WHITE_QUEEN))
				{
					/* Protects an allied piece. */
					currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if (piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] == WHITE_KING)
					{
						/* Do nothing. */
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
						newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
					}
				}
			}
		}

		/* Move knight in this way ( [current_row + 1][current_column - 2] ):                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][f][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][x][x][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*      Row ->   0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn - 2) >= 0))
		{
			if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] <= BLACK_KING))
			{
				currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
				newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] <= WHITE_QUEEN))
				{
					/* Protects an allied piece. */
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
				}
				else
				{
					if (piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] == WHITE_KING)
					{
						/* Do nothing. */
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
						newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
					}
				}
			}
		}

		/* Move knight in this way ( [current_row - 2][current_column + 1] ):                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][x][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][x][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][f][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow - 2) >= 0) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
		{
			if ((piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] <= BLACK_KING))
			{
				currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
				newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] <= WHITE_QUEEN))
				{
					/* Protects an allied piece. */
					currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if (piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] == WHITE_KING)
					{
						/* Do nothing. */
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
						newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
					}
				}
			}
		}

		/* Move knight in this way ( [current_row - 1][current_column + 2] ):                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][x][x][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][f][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn + 2) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
		{
			if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] <= BLACK_KING))
			{
				currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
				newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] <= WHITE_QUEEN))
				{
					/* Protects an allied piece. */
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
				}
				else
				{
					if (piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] == WHITE_KING)
					{
						/* Do nothing. */
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
						newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
					}
				}
			}
		}

		/* Move knight in this way ( [current_row + 2][current_column + 1] ):                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][f][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][x][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][x][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow + 2) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
		{
			if ((piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] <= BLACK_KING))
			{
				currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
				newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] <= WHITE_QUEEN))
				{
					/* Protects an allied piece. */
					currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if (piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] == WHITE_KING)
					{
						/* Do nothing. */
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
						newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
					}
				}
			}
		}

		/* Move knight in this way ( [current_row + 1][current_column + 2] ):                  */
		/* i = initial knight position;                                                        */
		/* x = intermediate fields that complete the knight move;                              */
		/* f = final knight position;                                                          */
		/* e = case: (e = initial knight position = final knight position);                    */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][f][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][x][x][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn + 2) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
		{
			if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] <= BLACK_KING))
			{
				currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
				newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] <= WHITE_QUEEN))
				{
					/* Protects an allied piece. */
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
				}
				else
				{
					if (piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] == WHITE_KING)
					{
						/* Do nothing. */
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
						newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
					}
				}
			}
		}
	}
	else
	{
		/* Case: local_currentPieceMove == BLACK_KNIGHT. */
		if (local_currentPieceMove == BLACK_KNIGHT)
		{
			/* Put the white knight back on its current position.                                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][e][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn] = 103;//set newLedMatrix[local_currentPieceRow][local_currentPieceColumn];
			newLedMatrix.values[local_currentPieceRow][local_currentPieceColumn] = 103;

			/* Move knight in this way ( [current_row - 2][current_column - 1] ):                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][x][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][x][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][f][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow - 2) >= 0) && ((local_currentPieceColumn - 1) >= 0))
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] <= WHITE_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
					newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] <= BLACK_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] == BLACK_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
							newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn - 1] = 1;
						}
					}
				}
			}

			/* Move knight in this way ( [current_row - 1][current_column - 2] ):                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][x][x][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][f][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn - 2) >= 0))
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] <= WHITE_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
					newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] <= BLACK_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] == BLACK_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
							newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 2] = 1;
						}
					}
				}
			}

			/* Move knight in this way ( [current_row + 2][current_column - 1] ):                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][f][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][x][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][x][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow + 2) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn - 1) >= 0))
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] <= WHITE_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
					newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] <= BLACK_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] == BLACK_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
							newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn - 1] = 1;
						}
					}
				}
			}

			/* Move knight in this way ( [current_row + 1][current_column - 2] ):                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][f][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][x][x][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn - 2) >= 0))
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] <= WHITE_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
					newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] <= BLACK_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] == BLACK_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
							newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 2] = 1;
						}
					}
				}
			}

			/* Move knight in this way ( [current_row - 2][current_column + 1] ):                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][x][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][x][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][f][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow - 2) >= 0) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] <= WHITE_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
					newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] <= BLACK_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] == BLACK_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
							newLedMatrix.values[local_currentPieceRow - 2][local_currentPieceColumn + 1] = 1;
						}
					}
				}
			}

			/* Move knight in this way ( [current_row - 1][current_column + 2] ):                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][x][x][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][f][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn + 2) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] <= WHITE_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
					newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] <= BLACK_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] == BLACK_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
							newLedMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 2] = 1;
						}
					}
				}
			}

			/* Move knight in this way ( [current_row + 2][current_column + 1] ):                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][f][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][x][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][x][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow + 2) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] <= WHITE_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
					newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] <= BLACK_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] == BLACK_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
							newLedMatrix.values[local_currentPieceRow + 2][local_currentPieceColumn + 1] = 1;
						}
					}
				}
			}

			/* Move knight in this way ( [current_row + 1][current_column + 2] ):                  */
			/* i = initial knight position;                                                        */
			/* x = intermediate fields that complete the knight move;                              */
			/* f = final knight position;                                                          */
			/* e = case: (e = initial knight position = final knight position);                    */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][f][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][x][x][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn + 2) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] <= WHITE_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
					newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] <= BLACK_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] == BLACK_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
							newLedMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 2] = 1;
						}
					}
				}
			}
		}
	}
}

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un nebun. */
void setLedsForPossibleMovesBishop(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn)
{
	if ((local_currentPieceMove == WHITE_BISHOP) || (local_currentPieceMove == WHITE_QUEEN))
	{
		for (int8_t i = 0; ((i <= local_currentPieceRow) && (i <= local_currentPieceColumn)); i++)
		{
			if (i == 0)
			{
				currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 103;
				newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 103;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] >= BLACK_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] <= BLACK_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;//set newLedMatrix[local_currentPieceRow - i][local_currentPieceColumn - i]
					newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;
					if (piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] != BLACK_KING)//Check
					{
						break;
					}
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] <= WHITE_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;
						break;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] == WHITE_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;
							newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;
						}
					}
				}
			}
		}

		for (int8_t i = 0; ((i <= local_currentPieceRow) && (i < NEWSENSORVALUESMATRIX_NR_COLUMNS - local_currentPieceColumn)); i++)
		{
			if (i == 0)
			{
				currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 103;
				newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 103;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] >= BLACK_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] <= BLACK_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;//set newLedMatrix[local_currentPieceRow - i][local_currentPieceColumn + i]
					newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;
					if (piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] != BLACK_KING)//Check
					{
						break;
					}
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] <= WHITE_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;
						break;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] == WHITE_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;
							newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;
						}
					}
				}
			}
		}

		for (int8_t i = 0; ((i < NEWSENSORVALUESMATRIX_NR_ROWS - local_currentPieceRow) && (i <= local_currentPieceColumn)); i++)
		{
			if (i == 0)
			{
				currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 103;
				newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 103;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] >= BLACK_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] <= BLACK_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;//set newLedMatrix[local_currentPieceRow + i][local_currentPieceColumn - i]
					newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;
					if (piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] != BLACK_KING)//Check
					{
						break;
					}
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] <= WHITE_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;
						break;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] == WHITE_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;
							newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;
						}
					}
				}
			}
		}

		for (int8_t i = 0; ((i < NEWSENSORVALUESMATRIX_NR_ROWS - local_currentPieceRow) && (i < NEWSENSORVALUESMATRIX_NR_COLUMNS - local_currentPieceColumn)); i++)
		{
			if (i == 0)
			{
				currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 103;
				newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 103;
			}
			else
			{
				if ((piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] >= BLACK_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] <= BLACK_KING))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;//set newLedMatrix[local_currentPieceRow + i][local_currentPieceColumn + i]
					newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;
					if (piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] != BLACK_KING)//Check
					{
						break;
					}
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] <= WHITE_QUEEN))
					{
						/* Protects an allied piece. */
						currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;
						break;
					}
					else
					{
						if (piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] == WHITE_KING)
						{
							/* Do nothing. */
						}
						else
						{
							currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;
							newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;
						}
					}
				}
			}
		}
	}
	else
	{
		/* Case: local_currentPieceMove == BLACK_BISHOP or local_currentPieceMove == BLACK_QUEEN. */
		if ((local_currentPieceMove == BLACK_BISHOP) || (local_currentPieceMove == BLACK_QUEEN))
		{
			for (int8_t i = 0; ((i <= local_currentPieceRow) && (i <= local_currentPieceColumn)); i++)
			{
				if (i == 0)
				{
					currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 103;
					newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] <= WHITE_KING))
					{
						currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;//set newLedMatrix[local_currentPieceRow - i][local_currentPieceColumn - i]
						newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;
						if (piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] != WHITE_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] >= BLACK_PAWN) &&
							(piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] <= BLACK_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] == BLACK_KING)
							{
								/* Do nothing. */
							}
							else
							{
								currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;
								newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn - i] = 1;
							}
						}
					}
				}
			}

			for (int8_t i = 0; ((i <= local_currentPieceRow) && (i < NEWSENSORVALUESMATRIX_NR_COLUMNS - local_currentPieceColumn)); i++)
			{
				if (i == 0)
				{
					currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 103;
					newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] <= WHITE_KING))
					{
						currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;//set newLedMatrix[local_currentPieceRow - i][local_currentPieceColumn + i]
						newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;
						if (piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] != WHITE_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] >= BLACK_PAWN) &&
							(piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] <= BLACK_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] == BLACK_KING)
							{
								/* Do nothing. */
							}
							else
							{
								currentPiecePossibleMove.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;
								newLedMatrix.values[local_currentPieceRow - i][local_currentPieceColumn + i] = 1;
							}
						}
					}
				}
			}

			for (int8_t i = 0; ((i < NEWSENSORVALUESMATRIX_NR_ROWS - local_currentPieceRow) && (i <= local_currentPieceColumn)); i++)
			{
				if (i == 0)
				{
					currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 103;
					newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] <= WHITE_KING))
					{
						currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;//set newLedMatrix[local_currentPieceRow + i][local_currentPieceColumn - i]
						newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;
						if (piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] != WHITE_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] >= BLACK_PAWN) &&
							(piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] <= BLACK_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] == BLACK_KING)
							{
								/* Do nothing. */
							}
							else
							{
								currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;
								newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn - i] = 1;
							}
						}
					}
				}
			}

			for (int8_t i = 0; ((i < NEWSENSORVALUESMATRIX_NR_ROWS - local_currentPieceRow) && (i < NEWSENSORVALUESMATRIX_NR_COLUMNS - local_currentPieceColumn)); i++)
			{
				if (i == 0)
				{
					currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 103;
					newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] <= WHITE_KING))
					{
						currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;//set newLedMatrix[local_currentPieceRow + i][local_currentPieceColumn + i]
						newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;
						if (piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] != WHITE_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] >= BLACK_PAWN) &&
							(piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] <= BLACK_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] == BLACK_KING)
							{
								/* Do nothing. */
							}
							else
							{
								currentPiecePossibleMove.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;
								newLedMatrix.values[local_currentPieceRow + i][local_currentPieceColumn + i] = 1;
							}
						}
					}
				}
			}
		}
	}
}

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un turn. */
void setLedsForPossibleMovesRook(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn, 
	uint8_t local_whiteCastleInProgress, uint8_t local_blackCastleInProgress)
{
	/* Complete the white king castle by moving the white rook. */
	if ((local_currentPieceMove == WHITE_ROOK) && (local_whiteCastleInProgress == 1) && (local_blackCastleInProgress == 0))
	{
		if (piecesPositionMatrix.values[0][2] == WHITE_KING)
		{
			currentPiecePossibleMove.values[0][3] = 1;
			newLedMatrix.values[0][3] = 1;
		}
		else
		{
			if (piecesPositionMatrix.values[0][6] == WHITE_KING)
			{
				currentPiecePossibleMove.values[0][5] = 1;
				newLedMatrix.values[0][5] = 1;
			}
		}
	}
	else
	{
		if ((local_currentPieceMove == WHITE_ROOK) || (local_currentPieceMove == WHITE_QUEEN))
		{
			for (int8_t i = local_currentPieceRow; i >= 0; i--)
			{
				if (i == local_currentPieceRow)
				{
					currentPiecePossibleMove.values[i][local_currentPieceColumn] = 103;
					newLedMatrix.values[i][local_currentPieceColumn] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[i][local_currentPieceColumn] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[i][local_currentPieceColumn] <= BLACK_KING))
					{
						currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
						newLedMatrix.values[i][local_currentPieceColumn] = 1;
						if (piecesPositionMatrix.values[i][local_currentPieceColumn] != BLACK_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[i][local_currentPieceColumn] >= WHITE_PAWN) &&
							(piecesPositionMatrix.values[i][local_currentPieceColumn] <= WHITE_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[i][local_currentPieceColumn] == WHITE_KING)
							{
								/* Do nothing. */
								break;
							}
							else
							{
								currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
								newLedMatrix.values[i][local_currentPieceColumn] = 1;
							}
						}
					}
				}
			}

			for (int8_t i = local_currentPieceRow; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
			{
				if (i == local_currentPieceRow)
				{
					currentPiecePossibleMove.values[i][local_currentPieceColumn] = 103;
					newLedMatrix.values[i][local_currentPieceColumn] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[i][local_currentPieceColumn] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[i][local_currentPieceColumn] <= BLACK_KING))
					{
						currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
						newLedMatrix.values[i][local_currentPieceColumn] = 1;
						if (piecesPositionMatrix.values[i][local_currentPieceColumn] != BLACK_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[i][local_currentPieceColumn] >= WHITE_PAWN) &&
							(piecesPositionMatrix.values[i][local_currentPieceColumn] <= WHITE_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[i][local_currentPieceColumn] == WHITE_KING)
							{
								/* Do nothing. */
								break;
							}
							else
							{
								currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
								newLedMatrix.values[i][local_currentPieceColumn] = 1;
							}
						}
					}
				}
			}

			for (int8_t j = local_currentPieceColumn; j >= 0; j--)
			{
				if (j == local_currentPieceColumn)
				{
					currentPiecePossibleMove.values[local_currentPieceRow][j] = 103;
					newLedMatrix.values[local_currentPieceRow][j] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow][j] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow][j] <= BLACK_KING))
					{
						currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
						newLedMatrix.values[local_currentPieceRow][j] = 1;
						if (piecesPositionMatrix.values[local_currentPieceRow][j] != BLACK_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[local_currentPieceRow][j] >= WHITE_PAWN) &&
							(piecesPositionMatrix.values[local_currentPieceRow][j] <= WHITE_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[local_currentPieceRow][j] == WHITE_KING)
							{
								/* Do nothing. */
								break;
							}
							else
							{
								currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
								newLedMatrix.values[local_currentPieceRow][j] = 1;
							}
						}
					}
				}
			}

			for (int8_t j = local_currentPieceColumn; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
			{
				if (j == local_currentPieceColumn)
				{
					currentPiecePossibleMove.values[local_currentPieceRow][j] = 103;
					newLedMatrix.values[local_currentPieceRow][j] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow][j] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow][j] <= BLACK_KING))
					{
						currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
						newLedMatrix.values[local_currentPieceRow][j] = 1;
						if (piecesPositionMatrix.values[local_currentPieceRow][j] != BLACK_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[local_currentPieceRow][j] >= WHITE_PAWN) &&
							(piecesPositionMatrix.values[local_currentPieceRow][j] <= WHITE_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[local_currentPieceRow][j] == WHITE_KING)
							{
								/* Do nothing. */
								break;
							}
							else
							{
								currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
								newLedMatrix.values[local_currentPieceRow][j] = 1;
							}
						}
					}
				}
			}
			// 4 for-uri:
			// 1) linie = [local_currentPieceRow, 0] si coloana = local_currentPieceColumn;
			// set newLedMatrix[linie][local_currentPieceColumn]
			// 2) linie = [local_currentPieceRow, NEWSENSORVALUESMATRIX_NR_ROWS]  si coloana = local_currentPieceColumn;
			// set newLedMatrix[linie][local_currentPieceColumn]
			// 3) linie = local_currentPieceRow si coloana = [local_currentPieceColumn, 0];
			// set newLedMatrix[local_currentPieceRow][coloana]
			// 4) linie = local_currentPieceRow si coloana = [local_currentPieceColumn, NEWSENSORVALUESMATRIX_NR_COLUMNS].
			// set newLedMatrix[local_currentPieceRow][coloana]

			// In fiecare for sunt 3 cazuri posibile:
			// 1) Campul curent este liber:
			// --- se aprinde led-ul cu campul curent.
			// 2) Campul curent este ocupat cu o piesa adversa (exceptie rege):
			// --- se aprinde led-ul cu campul curent si se face break.
			// 3) Campul curent este ocupat cu o piesa proprie sau rege advers:
			// --- se face break.
		}
	}

	/* Complete the white king castle by moving the white rook. */
	if ((local_currentPieceMove == BLACK_ROOK) && (local_whiteCastleInProgress == 0) && (local_blackCastleInProgress == 1))
	{
		if (piecesPositionMatrix.values[7][2] == BLACK_KING)
		{
			currentPiecePossibleMove.values[7][3] = 1;
			newLedMatrix.values[7][3] = 1;
		}
		else
		{
			if (piecesPositionMatrix.values[7][6] == BLACK_KING)
			{
				currentPiecePossibleMove.values[7][5] = 1;
				newLedMatrix.values[7][5] = 1;
			}
		}
	}
	else
	{
		/* Case: local_currentPieceMove == BLACK_ROOK or local_currentPieceMove == BLACK_QUEEN. */
		if ((local_currentPieceMove == BLACK_ROOK) || (local_currentPieceMove == BLACK_QUEEN))
		{
			for (int8_t i = local_currentPieceRow; i >= 0; i--)
			{
				if (i == local_currentPieceRow)
				{
					currentPiecePossibleMove.values[i][local_currentPieceColumn] = 103;
					newLedMatrix.values[i][local_currentPieceColumn] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[i][local_currentPieceColumn] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[i][local_currentPieceColumn] <= WHITE_KING))
					{
						currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
						newLedMatrix.values[i][local_currentPieceColumn] = 1;
						if (piecesPositionMatrix.values[i][local_currentPieceColumn] != WHITE_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[i][local_currentPieceColumn] >= BLACK_PAWN) &&
							(piecesPositionMatrix.values[i][local_currentPieceColumn] <= BLACK_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[i][local_currentPieceColumn] == BLACK_KING)
							{
								/* Do nothing. */
								break;
							}
							else
							{
								currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
								newLedMatrix.values[i][local_currentPieceColumn] = 1;
							}
						}
					}
				}
			}

			for (int8_t i = local_currentPieceRow; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
			{
				if (i == local_currentPieceRow)
				{
					currentPiecePossibleMove.values[i][local_currentPieceColumn] = 103;
					newLedMatrix.values[i][local_currentPieceColumn] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[i][local_currentPieceColumn] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[i][local_currentPieceColumn] <= WHITE_KING))
					{
						currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
						newLedMatrix.values[i][local_currentPieceColumn] = 1;
						if (piecesPositionMatrix.values[i][local_currentPieceColumn] != WHITE_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[i][local_currentPieceColumn] >= BLACK_PAWN) &&
							(piecesPositionMatrix.values[i][local_currentPieceColumn] <= BLACK_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[i][local_currentPieceColumn] == BLACK_KING)
							{
								/* Do nothing. */
								break;
							}
							else
							{
								currentPiecePossibleMove.values[i][local_currentPieceColumn] = 1;
								newLedMatrix.values[i][local_currentPieceColumn] = 1;
							}
						}
					}
				}
			}

			for (int8_t j = local_currentPieceColumn; j >= 0; j--)
			{
				if (j == local_currentPieceColumn)
				{
					currentPiecePossibleMove.values[local_currentPieceRow][j] = 103;
					newLedMatrix.values[local_currentPieceRow][j] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow][j] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow][j] <= WHITE_KING))
					{
						currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
						newLedMatrix.values[local_currentPieceRow][j] = 1;
						if (piecesPositionMatrix.values[local_currentPieceRow][j] != WHITE_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[local_currentPieceRow][j] >= BLACK_PAWN) &&
							(piecesPositionMatrix.values[local_currentPieceRow][j] <= BLACK_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[local_currentPieceRow][j] == BLACK_KING)
							{
								/* Do nothing. */
								break;
							}
							else
							{
								currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
								newLedMatrix.values[local_currentPieceRow][j] = 1;
							}
						}
					}
				}
			}

			for (int8_t j = local_currentPieceColumn; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
			{
				if (j == local_currentPieceColumn)
				{
					currentPiecePossibleMove.values[local_currentPieceRow][j] = 103;
					newLedMatrix.values[local_currentPieceRow][j] = 103;
				}
				else
				{
					if ((piecesPositionMatrix.values[local_currentPieceRow][j] >= WHITE_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow][j] <= WHITE_KING))
					{
						currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
						newLedMatrix.values[local_currentPieceRow][j] = 1;
						if (piecesPositionMatrix.values[local_currentPieceRow][j] != WHITE_KING)//Check
						{
							break;
						}
					}
					else
					{
						if ((piecesPositionMatrix.values[local_currentPieceRow][j] >= BLACK_PAWN) &&
							(piecesPositionMatrix.values[local_currentPieceRow][j] <= BLACK_QUEEN))
						{
							/* Protects an allied piece. */
							currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
							break;
						}
						else
						{
							if (piecesPositionMatrix.values[local_currentPieceRow][j] == BLACK_KING)
							{
								/* Do nothing. */
								break;
							}
							else
							{
								currentPiecePossibleMove.values[local_currentPieceRow][j] = 1;
								newLedMatrix.values[local_currentPieceRow][j] = 1;
							}
						}
					}
				}
			}
		}
	}



}

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutata o dama. */
void setLedsForPossibleMovesQueen(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn)
{
	setLedsForPossibleMovesRook(local_currentPieceMove, local_currentPieceRow, local_currentPieceColumn, 0, 0);
	setLedsForPossibleMovesBishop(local_currentPieceMove, local_currentPieceRow, local_currentPieceColumn);
}

/* Functie de detectare (valori stocate in currentPiecePossibleMove si newLedMatrix) a campurilor posibile pe care poate fi mutat un rege. */
void setLedsForPossibleMovesKing(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn, 
	uint8_t local_isCurrentKing)
{
	if (local_currentPieceMove == WHITE_KING)
	{
		/* Put the white king back on its current position.*/
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][e][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn] = 1;//set newLedMatrix[local_currentPieceRow][local_currentPieceColumn];

		/* Move king in this way ( [current_row - 1][current_column - 1] ):                    */
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][f][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn - 1) >= 0))
		{
			if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;
				}
			}
		}

		/* Move king in this way ( [current_row - 1][current_column] ):                        */
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][f][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if ((local_currentPieceRow - 1) >= 0)
		{
			if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn] = 1;
				}
			}
		}

		/* Move king in this way ( [current_row - 1][current_column + 1] ):                    */
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][f][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
		{
			if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;
				}
			}
		}

		/* Move king in this way ( [current_row][current_column - 1] ):                        */
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][f][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if ((local_currentPieceColumn - 1) >= 0)
		{
			if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 1] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 1] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 1] = 1;
				}
			}
		}

		/* Move king in this way ( [current_row][current_column + 1] ):                        */
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][f][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS)
		{
			if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 1] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 1] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 1] = 1;
				}
			}
		}

		/* Move king in this way ( [current_row + 1][current_column - 1] ):                    */
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][f][ ][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn - 1) >= 0))
		{
			if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;
				}
			}
		}

		/* Move king in this way ( [current_row + 1][current_column] ):                        */
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][f][ ][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if ((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS)
		{
			if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn] = 1;
				}
			}
		}

		/* Move king in this way ( [current_row + 1][current_column + 1] ):                    */
		/* i = initial king position;                                                          */
		/* x = intermediate fields that complete the king move;                                */
		/* f = final king position;                                                            */
		/* e = case: (e = initial king position = final king position);                        */
		/*       Column                                                                        */
		/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          4   [ ][ ][ ][ ][f][ ][ ][ ]                                               */
		/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
		/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
		/*                                                                                     */
		/*     Row ->    0  1  2  3  4  5  6  7                                                */
		if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
		{
			if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;
				}
			}
		}

		/* Short and long castle white king */
		if ((local_currentPieceRow == 0) && (local_currentPieceColumn == 4))
		{
			/* For long castle */
			if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 2] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 2] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 2] = 1;
				}
			}

			/* For short castle */
			if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] >= BLACK_PAWN) &&
				(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] <= BLACK_QUEEN)))
			{
				currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 2] = 1;
			}
			else
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] <= WHITE_KING)) ||
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] == BLACK_KING))
				{
					/* Do nothing */
					if ((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] != BLACK_KING) && (local_isCurrentKing == 0))//Protect 
					{
						currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 2] = 1;
					}
				}
				else
				{
					currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 2] = 1;
				}
			}
		}
	}
	else
	{
		/* Case: local_currentPieceMove == BLACK_KING. */

		if (local_currentPieceMove == BLACK_KING)
		{
			/* Put the black king back on its current position.*/
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][e][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn] = 1;//set newLedMatrix[local_currentPieceRow][local_currentPieceColumn];

			/* Move king in this way ( [current_row - 1][current_column - 1] ):                    */
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][f][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn - 1) >= 0))
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn - 1] = 1;
					}
				}
			}

			/* Move king in this way ( [current_row - 1][current_column] ):                        */
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][f][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if ((local_currentPieceRow - 1) >= 0)
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn] = 1;
					}
				}
			}

			/* Move king in this way ( [current_row - 1][current_column + 1] ):                    */
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][f][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow - 1) >= 0) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow - 1][local_currentPieceColumn + 1] = 1;
					}
				}
			}

			/* Move king in this way ( [current_row][current_column - 1] ):                        */
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][f][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if ((local_currentPieceColumn - 1) >= 0)
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 1] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 1] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 1] = 1;
					}
				}
			}

			/* Move king in this way ( [current_row][current_column + 1] ):                        */
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][f][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS)
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 1] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 1] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 1] = 1;
					}
				}
			}

			/* Move king in this way ( [current_row + 1][current_column - 1] ):                    */
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][f][ ][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn - 1) >= 0))
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn - 1] = 1;
					}
				}
			}

			/* Move king in this way ( [current_row + 1][current_column] ):                        */
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][f][ ][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if ((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS)
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn] = 1;
					}
				}
			}

			/* Move king in this way ( [current_row + 1][current_column + 1] ):                    */
			/* i = initial king position;                                                          */
			/* x = intermediate fields that complete the king move;                                */
			/* f = final king position;                                                            */
			/* e = case: (e = initial king position = final king position);                        */
			/*       Column                                                                        */
			/*          7   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          6   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          5   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          4   [ ][ ][ ][ ][f][ ][ ][ ]                                               */
			/*          3   [ ][ ][ ][i][ ][ ][ ][ ]					   					       */
			/*          2   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          1   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*          0   [ ][ ][ ][ ][ ][ ][ ][ ]                                               */
			/*                                                                                     */
			/*     Row ->    0  1  2  3  4  5  6  7                                                */
			if (((local_currentPieceRow + 1) < NEWSENSORVALUESMATRIX_NR_ROWS) && ((local_currentPieceColumn + 1) < NEWSENSORVALUESMATRIX_NR_COLUMNS))
			{
				if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow + 1][local_currentPieceColumn + 1] = 1;
					}
				}
			}

			/* Short and long castle black king */
			if ((local_currentPieceRow == 7) && (local_currentPieceColumn == 4))
			{
				/* For long castle */
				if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 2] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn - 2] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 2] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn - 2] = 1;
					}
				}

				/* For short castle */
				if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] >= WHITE_PAWN) &&
					(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] <= WHITE_QUEEN)))
				{
					currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 2] = 1;
				}
				else
				{
					if (((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] >= BLACK_PAWN) &&
						(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] <= BLACK_KING)) ||
						(piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] == WHITE_KING))
					{
						/* Do nothing */
						if ((piecesPositionMatrix.values[local_currentPieceRow][local_currentPieceColumn + 2] != WHITE_KING) && (local_isCurrentKing == 0))//Protect 
						{
							currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 2] = 1;
						}
					}
					else
					{
						currentPiecePossibleMove.values[local_currentPieceRow][local_currentPieceColumn + 2] = 1;
					}
				}
			}

		}
	}
}

/* Functie ce implementeaza urmatoarele operatii matriceale: rezultat = a - (b * c); urmate de impunerea unor conditii necesare pentru logica functiei */
/* "eliminatePossibleChecksForCurrentKing()".																										   */
Matrix_uint8_t Matrix_uint8_t_operation_substract(Matrix_uint8_t a, Matrix_uint8_t b, Matrix_uint8_t c, uint8_t abc_nr_rows, uint8_t abc_nr_columns)
{
	Matrix_uint8_t return_matrix =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};
	for (int8_t i = 0; i < abc_nr_rows; i++)
	{
		for (int8_t j = 0; j < abc_nr_columns; j++)
		{
			return_matrix.values[i][j] = a.values[i][j] - (b.values[i][j] * c.values[i][j]);
			if (return_matrix.values[i][j] == 154) /* Because 1 - 103 = 154 for uint8_t values.*/
			{
				return_matrix.values[i][j] = 1;
			}
			else
			{
				if (return_matrix.values[i][j] != 1)
				{
					return_matrix.values[i][j] = 0;
				}
			}
		}
	}
	return return_matrix;
}

/* Functie de resetare a valorilor unei matrici. (elementele sunt setate la valoarea 0) */
Matrix_uint8_t Matrix_uint8_t_reset_values(Matrix_uint8_t a, uint8_t a_nr_rows, uint8_t a_nr_columns)
{
	for (int8_t i = 0; i < a_nr_rows; i++)
	{
		for (int8_t j = 0; j < a_nr_columns; j++)
		{
			a.values[i][j] = 0;
		}
	}
	return a;
}

/* Functie de eliminare a campurilor ocupate/atacate de catre toate piesele adverse din numarul total de mutari posibile ale regelui ce trebuie mutat. */
Matrix_uint8_t eliminatePossibleChecksForCurrentKing(uint8_t local_currentPieceMove, uint8_t local_currentPieceRow, uint8_t local_currentPieceColumn)
{
	Matrix_uint8_t currentKingPossibleMoves_withoutChecks =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};
	Matrix_uint8_t currentKingPossibleMoves_withChecks =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};
	Matrix_uint8_t oppositePiecePossibleMoves =
	{
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};

	currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
	setLedsForPossibleMovesKing(local_currentPieceMove, local_currentPieceRow, local_currentPieceColumn, 1);
	currentKingPossibleMoves_withoutChecks = currentPiecePossibleMove;
	currentKingPossibleMoves_withChecks = currentPiecePossibleMove;

	for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
	{
		for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
		{
			if (local_currentPieceMove == WHITE_KING)
			{
				switch (piecesPositionMatrix.values[i][j])
				{
				case BLACK_PAWN:
				{
					currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					setLedsForPossibleMovesPawn(piecesPositionMatrix.values[i][j], i, j);
					oppositePiecePossibleMoves = currentPiecePossibleMove;
					currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					//functie_diferenta_matrice
					//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
					//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
					//{
						//currentKingPossibleMoves = 0;
					//}
					break;
				}
				case BLACK_KNIGHT:
				{
					currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					setLedsForPossibleMovesKnight(piecesPositionMatrix.values[i][j], i, j);
					oppositePiecePossibleMoves = currentPiecePossibleMove;
					currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					//functie_diferenta_matrice
					//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
					//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
					//{
						//currentKingPossibleMoves = 0;
					//}
					break;
				}
				case BLACK_BISHOP:
				{
					currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					setLedsForPossibleMovesBishop(piecesPositionMatrix.values[i][j], i, j);
					oppositePiecePossibleMoves = currentPiecePossibleMove;
					currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					//functie_diferenta_matrice
					//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
					//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
					//{
						//currentKingPossibleMoves = 0;
					//}
					break;
				}
				case BLACK_ROOK:
				{
					currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					setLedsForPossibleMovesRook(piecesPositionMatrix.values[i][j], i, j, 0, 0);
					oppositePiecePossibleMoves = currentPiecePossibleMove;
					currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					//functie_diferenta_matrice
					//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
					//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
					//{
						//currentKingPossibleMoves = 0;
					//}
					break;
				}
				case BLACK_QUEEN:
				{
					currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					setLedsForPossibleMovesQueen(piecesPositionMatrix.values[i][j], i, j);
					oppositePiecePossibleMoves = currentPiecePossibleMove;
					currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					//functie_diferenta_matrice
					//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
					//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
					//{
						//currentKingPossibleMoves = 0;
					//}
					break;
				}
				case BLACK_KING:
				{
					currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					setLedsForPossibleMovesKing(piecesPositionMatrix.values[i][j], i, j, 0);
					oppositePiecePossibleMoves = currentPiecePossibleMove;
					currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
					//functie_diferenta_matrice
					//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
					//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
					//{
						//currentKingPossibleMoves = 0;
					//}
					break;
				}
				}
			}
			else
			{
				if (local_currentPieceMove == BLACK_KING)
				{
					switch (piecesPositionMatrix.values[i][j])
					{
					case WHITE_PAWN:
					{
						currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						setLedsForPossibleMovesPawn(piecesPositionMatrix.values[i][j], i, j);
						oppositePiecePossibleMoves = currentPiecePossibleMove;
						currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						//functie_diferenta_matrice
						//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
						//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
						//{
						//currentKingPossibleMoves = 0;
						//}
						break;
					}
					case WHITE_KNIGHT:
					{
						currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						setLedsForPossibleMovesKnight(piecesPositionMatrix.values[i][j], i, j);
						oppositePiecePossibleMoves = currentPiecePossibleMove;
						currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						//functie_diferenta_matrice
						//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
						//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
						//{
							//currentKingPossibleMoves = 0;
						//}
						break;
					}
					case WHITE_BISHOP:
					{
						currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						setLedsForPossibleMovesBishop(piecesPositionMatrix.values[i][j], i, j);
						oppositePiecePossibleMoves = currentPiecePossibleMove;
						currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						//functie_diferenta_matrice
						//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
						//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
						//{
							//currentKingPossibleMoves = 0;
						//}
						break;
					}
					case WHITE_ROOK:
					{
						currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						setLedsForPossibleMovesRook(piecesPositionMatrix.values[i][j], i, j, 0, 0);
						oppositePiecePossibleMoves = currentPiecePossibleMove;
						currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						//functie_diferenta_matrice
						//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
						//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
						//{
							//currentKingPossibleMoves = 0;
						//}
						break;
					}
					case WHITE_QUEEN:
					{
						currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						setLedsForPossibleMovesQueen(piecesPositionMatrix.values[i][j], i, j);
						oppositePiecePossibleMoves = currentPiecePossibleMove;
						currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						//functie_diferenta_matrice
						//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
						//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
						//{
							//currentKingPossibleMoves = 0;
						//}
						break;
					}
					case WHITE_KING:
					{
						currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						oppositePiecePossibleMoves = Matrix_uint8_t_reset_values(oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						setLedsForPossibleMovesKing(piecesPositionMatrix.values[i][j], i, j, 0);
						oppositePiecePossibleMoves = currentPiecePossibleMove;
						currentKingPossibleMoves_withChecks = Matrix_uint8_t_operation_substract(currentKingPossibleMoves_withChecks, currentKingPossibleMoves_withoutChecks, oppositePiecePossibleMoves, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
						//functie_diferenta_matrice
						//currentKingPossibleMoves = currentKingPossibleMoves - oppositePiecePossibleMoves;
						//if ((currentKingPossibleMoves != 0) || (currentKingPossibleMoves != 1)) 
						//{
							//currentKingPossibleMoves = 0;
						//}
						break;
					}
					}
				}
			}
		}
	}

	return currentKingPossibleMoves_withChecks;
}

/*----------Definirea functiilor folosite pentru implementarea regulilor jocului de sah----------*/
/*----------Defining the functions used to implement the rules of the chess game----------*/
/*-------------------------------------------2--------------------------------------------*/