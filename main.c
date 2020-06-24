/**
   * File name: main.c
   * File type: source code file (.c)
*/

#define F_CPU 16000000

#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>
#include "useful_functions.c"
#include "chess_game_rules_functions.c"

/* Declare and initialize global variables */
uint16_t counted_ten_milliseconds = 0;
uint8_t whoMoves = WHITE_MOVES; /* 1 is white and 0 is black */
uint8_t white_minutes = INITIAL_WHITE_MINUTES;
uint8_t white_seconds = INITIAL_WHITE_SECONDS;
uint8_t black_minutes = INITIAL_BLACK_MINUTES;
uint8_t black_seconds = INITIAL_BLACK_SECONDS;
uint8_t checkMate_flag = 0;

enum System_state{STATE_1 = 0, STATE_2 = 1};
typedef enum System_state System_state;

ISR(TIMER1_COMPA_vect)
{
	counted_ten_milliseconds++;
	if(counted_ten_milliseconds == 100)
	{
		if(whoMoves == WHITE_MOVES)
		{
			if(white_seconds != 0)
			{
				white_seconds--;
			}
			else
			{
				white_minutes--;
				white_seconds = 59;
			}
			if((white_minutes == 0) && (white_seconds == 0))
			{
				//Mark that black wins! (set a variable)
				//WS2812_MATRIX_setLedsBasedOnSensorValuesAndColor((uint64_t)sensorValues, TURQOISE);
				//display white_minutes and white_seconds (marking that black wins)
				checkMate_flag = 1;
				MAX7219_SPI_displayWhiteTime(white_minutes, white_seconds);
				ATMEGA2560_TIMER1_stop();
			}
			else
			{
				//display white_minutes and white_seconds (without marking anything)
				MAX7219_SPI_displayWhiteTime(white_minutes, white_seconds);
			}
		}
		else
		{
			if(black_seconds != 0)
			{
				black_seconds--;
			}
			else
			{
				black_minutes--;
				black_seconds = 59;
			}
			if((black_minutes == 0) && (black_seconds == 0))
			{
				//Mark that white wins! (set a variable)
				//WS2812_MATRIX_setLedsBasedOnSensorValuesAndColor((uint64_t)sensorValues, YELLOW);
				//display black_minutes and black_seconds (marking that white wins)
				checkMate_flag = 1;
				MAX7219_SPI_displayBlackTime(black_minutes, black_seconds);
				ATMEGA2560_TIMER1_stop();
			}
			else
			{
				//display black_minutes and black_seconds (without marking anything)
				MAX7219_SPI_displayBlackTime(black_minutes, black_seconds);
			}
		}
		
		counted_ten_milliseconds = 0;
	}
	else
	{
		//do nothing
	}
}

void main(void)
{
	uint64_t sensorValues = 0xFFFFFFFFFFFFFFFF;
	System_state current_system_state = STATE_1;
	uint8_t button1_StateMain_confirmWhiteMove = 1;
	uint8_t button2_StateMain_confirmBlackMove = 1;
	uint8_t button3_StateMain_startGame = 1;
	uint8_t button4_StateMain_promotePawnToKnight = 1;
	uint8_t button5_StateMain_promotePawnToBishop = 1;
	uint8_t button6_StateMain_promotePawnToRook = 1;
	uint8_t button7_StateMain_promotePawnToQueen = 1;
	SN74HC165_init();
	Buttons_init();
	MAX7219_SPI_configureDisplay_decoded();
	ATMEGA2560_TIMER1_init();
	sei();
	MAX7219_SPI_displayWhiteTime(white_minutes, white_seconds);
	MAX7219_SPI_displayBlackTime(black_minutes, black_seconds);
	/*---------------------------------------------1------------------------------------------------*/
	/*----------Bloc de initializare a variabilelor folosite in implementarea algoritmului----------*/
	uint8_t currentPieceRow = 0; /* Variabila care stocheaza linia pe care se gaseste piesa curenta ce trebuie mutata. */
	uint8_t currentPieceColumn = 0;  /* Variabila care stocheaza coloana pe care se gaseste piesa curenta ce trebuie mutata. */
	uint8_t currentPieceMove = 0;  /* Variabila care stocheaza tipul piesei curente ce trebuie mutata. */

	uint8_t nrOfPiecesHeld = 0; /* Variabila care stocheaza numarul de piese ridicate in cadrul unei mutari. */

	uint8_t firstPieceRow = 0;
	uint8_t firstPieceColumn = 0;
	uint8_t secondPieceRow = 0;
	uint8_t secondPieceColumn = 0;

	/* Variables used to detect a checkmate. */
	uint8_t white_king_currentRow_checkmateDetection = 0;
	uint8_t white_king_currentColumn_checkmateDetection = 0;
	uint8_t black_king_currentRow_checkmateDetection = 0;
	uint8_t black_king_currentColumn_checkmateDetection = 0;
	uint8_t white_king_numberOfPossibleMoves = 0;
	uint8_t black_king_numberOfPossibleMoves = 0;
	uint8_t pieceWhichTreatMate_currentRow = 0;
	uint8_t pieceWhichTreatMate_currentColumn = 0;
	uint8_t numberOfPiecesWhichTreatMate = 0;
	
	uint8_t diagonalTreatFields_rowVector[NEWSENSORVALUESMATRIX_NR_ROWS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t diagonalTreatFields_columnVector[NEWSENSORVALUESMATRIX_NR_COLUMNS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8_t diagonalTreatFields_vectorsIndexCounter = 0;

	/* Matrice ce stocheaza valorile necesare pentru aprinderea led-urilor in functie de piesa curenta ridicata si de mutarile posibile ale acesteia. */
	Matrix_uint8_t current_king_checkmateDetectionMatrix =
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
	Matrix_uint8_t pieceWhichTreatMateMatrix =
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

	/*--------------------------------------2-------------------------------------------*/
	/*----------Variabile ajutatoare pentru simularea software a tablei de sah----------*/
	/*DEBUG*/ /*uint8_t contor = 100;*/ uint8_t contor = 2;
	uint8_t chessClockPressEnable = 0;
	
	/* Variables used in long castle and short castle detection. */
	uint8_t first_white_king_move = 1;
	uint8_t first_black_king_move = 1;
	uint8_t first_a_column_white_rook_move = 1;
	uint8_t first_a_column_black_rook_move = 1;
	uint8_t first_h_column_white_rook_move = 1;
	uint8_t first_h_column_black_rook_move = 1;
	uint8_t whiteCastleInProgress = 0;
	uint8_t blackCastleInProgress = 0;
	
	uint8_t ledVectorOrderOfIndexes_main[64] = { 56, 57, 58, 59, 60, 61, 62, 63,   55, 54, 53, 52, 51, 50, 49, 48,   40, 41, 42, 43, 44, 45, 46, 47,
		39, 38, 37, 36, 35, 34, 33, 32,   24, 25, 26, 27, 28, 29, 30, 31,   23, 22, 21, 20, 19, 18, 17, 16,   8, 9, 10, 11, 12, 13, 14, 15,
	7, 6, 5, 4, 3, 2, 1, 0 };

	/*----------Variabile ajutatoare pentru simularea software a tablei de sah----------*/
	/*--------------------------------------2-------------------------------------------*/

	previousSensorValuesMatrix = newSensorValuesMatrix;
	currentSensorValuesMatrix = newSensorValuesMatrix;

	/*----------Bloc de initializare a variabilelor folosite in implementarea algoritmului----------*/
	/*---------------------------------------------1------------------------------------------------*/
	
	while (1)
	{
		/*----------------------------------------------STATES_HANDLER_instructions---------------------------------------------------------------*/
		button3_StateMain_startGame = Button_debounce(&READ_PIN_Buttons, PIN_Button3_startGame);
		if(!button3_StateMain_startGame)
		{
			switch(current_system_state)
			{
				case STATE_1:
					current_system_state = STATE_2;
					ATMEGA2560_TIMER1_start(); /* Start timer1. */
				break;
				case STATE_2:
					current_system_state = STATE_1;
					ATMEGA2560_TIMER1_stop(); /* Stop timer1. */
				break;
				default:
					current_system_state = STATE_1;
			}
			_delay_ms(10);
		}
		else
		{
			/* do nothing */
		}
		/*----------------------------------------------STATES_HANDLER_instructions---------------------------------------------------------------*/
		switch(current_system_state)
		{
			case STATE_1:
				/*----------------------------------------------STATE_1_instructions---------------------------------------------------------------*/
				/* --------------------------------------------Resetting all variables ------------------------------------------------------------*/
				whoMoves = WHITE_MOVES;
				checkMate_flag = 0;
				
				currentPieceRow = 0;
				currentPieceColumn = 0;
				currentPieceMove = 0;

				nrOfPiecesHeld = 0;

				firstPieceRow = 0;
				firstPieceColumn = 0;
				secondPieceRow = 0;
				secondPieceColumn = 0;


				white_king_currentRow_checkmateDetection = 0;
				white_king_currentColumn_checkmateDetection = 0;
				black_king_currentRow_checkmateDetection = 0;
				black_king_currentColumn_checkmateDetection = 0;
				white_king_numberOfPossibleMoves = 0;
				black_king_numberOfPossibleMoves = 0;
				pieceWhichTreatMate_currentRow = 0;
				pieceWhichTreatMate_currentColumn = 0;
				numberOfPiecesWhichTreatMate = 0;
					

				for(int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
				{
					diagonalTreatFields_rowVector[i] = 0;
				}
				for(int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_COLUMNS; i++)
				{
					diagonalTreatFields_columnVector[i] = 0;
				}
				diagonalTreatFields_vectorsIndexCounter = 0;


				current_king_checkmateDetectionMatrix = Matrix_uint8_t_reset_values(current_king_checkmateDetectionMatrix, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
				pieceWhichTreatMateMatrix = Matrix_uint8_t_reset_values(current_king_checkmateDetectionMatrix, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
				

				contor = 2;
				chessClockPressEnable = 0;
					

				first_white_king_move = 1;
				first_black_king_move = 1;
				first_a_column_white_rook_move = 1;
				first_a_column_black_rook_move = 1;
				first_h_column_white_rook_move = 1;
				first_h_column_black_rook_move = 1;
				whiteCastleInProgress = 0;
				blackCastleInProgress = 0;

				previousSensorValuesMatrix = newSensorValuesMatrix;
				currentSensorValuesMatrix = newSensorValuesMatrix;
				
				piecesPositionMatrix = initialPiecesPositionMatrix;
				currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
				newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
				/* --------------------------------------------Resetting all variables ------------------------------------------------------------*/
				button1_StateMain_confirmWhiteMove = Button_debounce(&READ_PIN_Buttons, PIN_Button1_confirm_white_move); /* In STATE_1, button1 increment white time with one second. */
				button2_StateMain_confirmBlackMove = Button_debounce(&READ_PIN_Buttons, PIN_Button2_confirm_black_move); /* In STATE_1, button2 increment black time with one second. */
				if(!button1_StateMain_confirmWhiteMove)
				{
					white_seconds++;
					if(white_seconds >= 60) /* Normally, white_seconds cannot exceed 60 , but it is a safety measure. */
					{
						white_seconds = 0;
						white_minutes++;
						if(white_minutes >= 100) /* Normally, white_minutes cannot exceed 100 , but it is a safety measure. */
						{
							white_minutes = 0;
							white_seconds = 0;
						}
					}
					//display white_minutes and white_seconds
					MAX7219_SPI_displayWhiteTime(white_minutes, white_seconds);
				}
				if(!button2_StateMain_confirmBlackMove)
				{
					black_seconds++;
					if(black_seconds >= 60) /* Normally, black_seconds cannot exceed 60 , but it is a safety measure. */
					{
						black_seconds = 0;
						black_minutes++;
						if(black_minutes >= 100) /* Normally, black_minutes cannot exceed 100 , but it is a safety measure. */
						{
							black_minutes = 0;
							black_seconds = 0;
						}
					}
					//display black_minutes and black_seconds
					MAX7219_SPI_displayBlackTime(black_minutes, black_seconds);
				}
				_delay_ms(10);
				checkMate_flag = 0;
				sensorValues = SN74HC165_SPI_masterReceive_eightBytes();
				sensorValues = ~sensorValues;
				//WS2812_MATRIX_setLedsBasedOnSensorValuesAndColor((uint64_t)sensorValues, ORANGE);
				newSensorValuesMatrix = sensorPositionDecoder(sensorValues);
				WS2812_ledPositionDecoder(newSensorValuesMatrix, ORANGE);
				/*----------------------------------------------STATE_1_instructions---------------------------------------------------------------*/
				break;
			case STATE_2:
				/*----------------------------------------------STATE_2_instructions---------------------------------------------------------------*/
				button1_StateMain_confirmWhiteMove = Button_debounce(&READ_PIN_Buttons, PIN_Button1_confirm_white_move); /* In STATE_2, button1 confirm white move. */
				button2_StateMain_confirmBlackMove = Button_debounce(&READ_PIN_Buttons, PIN_Button2_confirm_black_move); /* In STATE_2, button2 confirm black move. */
				if(!button1_StateMain_confirmWhiteMove)
				{
					whoMoves = BLACK_MOVES;
				}
				else
				{
					/* whoMoves doesn't change */
				}
				if(!button2_StateMain_confirmBlackMove)
				{
					whoMoves = WHITE_MOVES;
				}
				else
				{
					/* whoMoves doesn't change */
				}
				_delay_ms(10);

		sensorValues = SN74HC165_SPI_masterReceive_eightBytes();
				//MAX7219_SPI_sensorValues(sensorValues);
				
				//WS2812_MATRIX_setLedsBasedOnSensorValues((uint64_t)0xFFFFFFFFFFFFFFFF);
				
				/*DEBUG*/ /*contor--;*/

		/* sensorValues = SN74HC165_SPI_masterReceive_eightBytes(); (Citirea celor 64 de senzori) */
		/* Cateva exemple: */
		/* sensorValues = 0x3333_3333_3333_3333; (Valoarea din pozitia initiala)  */
		/* sensorValues = 0x7333_3333_3333_3333; (A fost ridicat pionul de pe A2) */
		/* sensorValues = 0x3733_3333_3333_3333; (A fost ridicat pionul de pe B2) */
		/* sensorValues = 0x3B33_3333_3333_3333; (A fost ridicat calul de pe B1)  */
		/*DEBUG*/ /* printf("Citirea celor 64 de senzori: sensorValues = "); */
		/*DEBUG*/ /*scanf_s("%I64X", &sensorValues);*/
		/**/ /*printf("\n");*/
		newSensorValuesMatrix = sensorPositionDecoder(sensorValues); /* Decodarea campurilor pe care sunt plasati senzorii si stocarea valorilor citite in newSensorMatrix. */

		/* Update la cele doua matrici (previousSensorValuesMatrix si newSensorValuesMatrix) ce trebuie comparate pentru a determina (in program) modificarile aparute pe tabla de sah. */
		previousSensorValuesMatrix = currentSensorValuesMatrix;
		currentSensorValuesMatrix = newSensorValuesMatrix;

		if(contor == 0)
		{
			/* Detectarea ridicarii piesei curente de pe campul de plecare. */
			for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
			{
				for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
				{
					if ((previousSensorValuesMatrix.values[i][j] == 0) && (currentSensorValuesMatrix.values[i][j] == 1))
					{
						nrOfPiecesHeld++;
						if (nrOfPiecesHeld == 1)
						{
							firstPieceRow = i;
							firstPieceColumn = j;
						}
						else
						{
							if (nrOfPiecesHeld == 2)
							{
								secondPieceRow = i;
								secondPieceColumn = j;
							}
						}
						// check which piece is piece[i][j], hold piece[i][j] in a variable (Ex: currentPieceMove = WHITE_ROOK)
						// switch(piece[i][j])....case(ROOK,KNIGHT,etc....)
						// set LEDs based on piece type (one function for each piece type, this function will handle problems according with piecesPositionMatrix)
						// Probleme posibile: punerea piesei pe acelasi camp, imposibilitatea de a muta piesa pe vreun camp. 
					}
				}
			}

			/*DEBUG*/ /*if (contor == 99)*/ /* La prima rulare, reseteaza nrOfPiecesHeld dupa primele doua for-uri*/
			if(contor != 0)
			{
				nrOfPiecesHeld = 0;
				//contor = 0;
			}

			/*DEBUG*/ /*printf("nrOfPiecesHeld = %d", nrOfPiecesHeld);*/

			/* Obtinerea campurilor posibile unde poate fi mutata piesa curenta ce a fost ridicata. */
			if (nrOfPiecesHeld == 1)
			{
				currentPieceRow = firstPieceRow;
				currentPieceColumn = firstPieceColumn;
				currentPieceMove = piecesPositionMatrix.values[currentPieceRow][currentPieceColumn];

				switch (currentPieceMove)
				{
				case WHITE_PAWN:
				{
					if (whoMoves == WHITE_MOVES)
					{
						setLedsForPossibleMovesPawn(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case WHITE_KNIGHT:
				{
					if (whoMoves == WHITE_MOVES)
					{
						setLedsForPossibleMovesKnight(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case WHITE_BISHOP:
				{
					if (whoMoves == WHITE_MOVES)
					{
						setLedsForPossibleMovesBishop(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case WHITE_ROOK:
				{
					if (whoMoves == WHITE_MOVES)
					{
						setLedsForPossibleMovesRook(currentPieceMove, currentPieceRow, currentPieceColumn, whiteCastleInProgress, blackCastleInProgress);
					}
					break;
				}
				case WHITE_QUEEN:
				{
					if (whoMoves == WHITE_MOVES)
					{
						setLedsForPossibleMovesQueen(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case WHITE_KING:
				{
					if (whoMoves == WHITE_MOVES)
					{
						newLedMatrix = eliminatePossibleChecksForCurrentKing(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case BLACK_PAWN:
				{
					if (whoMoves == BLACK_MOVES)
					{
						setLedsForPossibleMovesPawn(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case BLACK_KNIGHT:
				{
					if (whoMoves == BLACK_MOVES)
					{
						setLedsForPossibleMovesKnight(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case BLACK_BISHOP:
				{
					if (whoMoves == BLACK_MOVES)
					{
						setLedsForPossibleMovesBishop(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case BLACK_ROOK:
				{
					if (whoMoves == BLACK_MOVES)
					{
						setLedsForPossibleMovesRook(currentPieceMove, currentPieceRow, currentPieceColumn, whiteCastleInProgress, blackCastleInProgress);
					}
					break;
				}
				case BLACK_QUEEN:
				{
					if (whoMoves == BLACK_MOVES)
					{
						setLedsForPossibleMovesQueen(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				case BLACK_KING:
				{
					if (whoMoves == BLACK_MOVES)
					{
						newLedMatrix = eliminatePossibleChecksForCurrentKing(currentPieceMove, currentPieceRow, currentPieceColumn);
					}
					break;
				}
				}
			}
			else
			{
				if (nrOfPiecesHeld == 2)
				{
					nrOfPiecesHeld = 0;
					if (((piecesPositionMatrix.values[firstPieceRow][firstPieceColumn] >= WHITE_PAWN) && (piecesPositionMatrix.values[firstPieceRow][firstPieceColumn] <= WHITE_QUEEN)) &&
						((piecesPositionMatrix.values[secondPieceRow][secondPieceColumn] >= BLACK_PAWN) && (piecesPositionMatrix.values[secondPieceRow][secondPieceColumn] <= BLACK_QUEEN)))
					{
						if (whoMoves == WHITE_MOVES)
						{
							currentPieceRow = firstPieceRow;
							currentPieceColumn = firstPieceColumn;
						}
						else
						{
							if (whoMoves == BLACK_MOVES)
							{
								currentPieceRow = secondPieceRow;
								currentPieceColumn = secondPieceColumn;
							}
						}
					}
					else
					{
						if (((piecesPositionMatrix.values[firstPieceRow][firstPieceColumn] >= BLACK_PAWN) && (piecesPositionMatrix.values[firstPieceRow][firstPieceColumn] <= BLACK_QUEEN)) &&
							((piecesPositionMatrix.values[secondPieceRow][secondPieceColumn] >= WHITE_PAWN) && (piecesPositionMatrix.values[secondPieceRow][secondPieceColumn] <= WHITE_QUEEN)))
						{
							if (whoMoves == WHITE_MOVES)
							{
								currentPieceRow = secondPieceRow;
								currentPieceColumn = secondPieceColumn;
							}
							else
							{
								if (whoMoves == BLACK_MOVES)
								{
									currentPieceRow = firstPieceRow;
									currentPieceColumn = firstPieceColumn;
								}
							}
						}
						else
						{
							//EROARE
						}
					}
					currentPieceMove = piecesPositionMatrix.values[currentPieceRow][currentPieceColumn];
					switch (currentPieceMove)
					{
					case WHITE_PAWN:
					{
						if (whoMoves == WHITE_MOVES)
						{
							setLedsForPossibleMovesPawn(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case WHITE_KNIGHT:
					{
						if (whoMoves == WHITE_MOVES)
						{
							setLedsForPossibleMovesKnight(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case WHITE_BISHOP:
					{
						if (whoMoves == WHITE_MOVES)
						{
							setLedsForPossibleMovesBishop(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case WHITE_ROOK:
					{
						if (whoMoves == WHITE_MOVES)
						{
							setLedsForPossibleMovesRook(currentPieceMove, currentPieceRow, currentPieceColumn, whiteCastleInProgress, blackCastleInProgress);
						}
						break;
					}
					case WHITE_QUEEN:
					{
						if (whoMoves == WHITE_MOVES)
						{
							setLedsForPossibleMovesQueen(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case WHITE_KING:
					{
						if (whoMoves == WHITE_MOVES)
						{
							newLedMatrix = eliminatePossibleChecksForCurrentKing(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case BLACK_PAWN:
					{
						if (whoMoves == BLACK_MOVES)
						{
							setLedsForPossibleMovesPawn(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case BLACK_KNIGHT:
					{
						if (whoMoves == BLACK_MOVES)
						{
							setLedsForPossibleMovesKnight(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case BLACK_BISHOP:
					{
						if (whoMoves == BLACK_MOVES)
						{
							setLedsForPossibleMovesBishop(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case BLACK_ROOK:
					{
						if (whoMoves == BLACK_MOVES)
						{
							setLedsForPossibleMovesRook(currentPieceMove, currentPieceRow, currentPieceColumn, whiteCastleInProgress, blackCastleInProgress);
						}
						break;
					}
					case BLACK_QUEEN:
					{
						if (whoMoves == BLACK_MOVES)
						{
							setLedsForPossibleMovesQueen(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					case BLACK_KING:
					{
						if (whoMoves == BLACK_MOVES)
						{
							newLedMatrix = eliminatePossibleChecksForCurrentKing(currentPieceMove, currentPieceRow, currentPieceColumn);
						}
						break;
					}
					}
				}
				else
				{
					//EROARE
				}
			}

			/*------------------------------------*/
			/* Bloc pentru aprinderea LED-urilor. */
			
			WS2812_ledPositionDecoder(newLedMatrix, GREEN); /* Decodarea campurilor pe care sunt plasate LED-urilor si setarea culorilor acestora in conformitate cu campurile decodate. */
			
			/* Bloc pentru aprinderea LED-urilor. */
			/*------------------------------------*/
		
			for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
			{
				for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
				{
					if ((previousSensorValuesMatrix.values[i][j] == 1) && (currentSensorValuesMatrix.values[i][j] == 0))
					{
						if (newLedMatrix.values[i][j] != 0)
						{
							//LEGAL move
							//WS2812_MATRIX_setLedsBasedOnSensorValuesAndColor((uint64_t)sensorValues, BLUE);
							led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
							led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
							led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
							ws2812_setleds(led,MAXPIX);
							/*DEBUG*/ /*printf("it's legal; i = %d, j = %d, currentPieceRow = %d, currentPieceColumn = %d, newLedMatrix = %d\n", i, j, currentPieceRow, currentPieceColumn, newLedMatrix.values[i][j]);*/
							/*DEBUG*/ /*printf("contor = %d", contor);*/

							//put the piece on the new field
							piecesPositionMatrix.values[currentPieceRow][currentPieceColumn] = NO_PIECE;
							piecesPositionMatrix.values[i][j] = currentPieceMove;

							if (((currentPieceMove == WHITE_PAWN) && (i == 7)) || ((currentPieceMove == BLACK_PAWN) && (i == 0)))
							{
								do
								{
									button4_StateMain_promotePawnToKnight = Button_debounce(&READ_PIN_Buttons, PIN_Button4_pawn_promote_knight);
									button5_StateMain_promotePawnToBishop = Button_debounce(&READ_PIN_Buttons, PIN_Button5_pawn_promote_bishop);
									button6_StateMain_promotePawnToRook = Button_debounce(&READ_PIN_Buttons, PIN_Button6_pawn_promote_rook);
									button7_StateMain_promotePawnToQueen = Button_debounce(&READ_PIN_Buttons, PIN_Button7_pawn_promote_queen);
									/*
									printf("Give BUTTON_KNIGHT_PRESSED =  ");
									scanf_s("%d", &BUTTON_KNIGHT_PRESSED);
									printf("\n");
									printf("Give BUTTON_BISHOP_PRESSED =  ");
									scanf_s("%d", &BUTTON_BISHOP_PRESSED);
									printf("\n");
									printf("Give BUTTON_ROOK_PRESSED =  ");
									scanf_s("%d", &BUTTON_ROOK_PRESSED);
									printf("\n");
									printf("Give BUTTON_QUEEN_PRESSED =  ");
									scanf_s("%d", &BUTTON_QUEEN_PRESSED);
									printf("\n");
									*/
									if (!button4_StateMain_promotePawnToKnight) /* Read Knight promote button. */
									{
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
										ws2812_setleds(led,MAXPIX);
										_delay_ms(200);
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
										ws2812_setleds(led,MAXPIX);
										piecesPositionMatrix.values[currentPieceRow][currentPieceColumn] = NO_PIECE;
										if (currentPieceMove == WHITE_PAWN)
										{
											piecesPositionMatrix.values[i][j] = WHITE_KNIGHT; /* Promote white pawn to knight. */
										}
										if (currentPieceMove == BLACK_PAWN)
										{
											piecesPositionMatrix.values[i][j] = BLACK_KNIGHT; /* Promote black pawn to knight. */
										}
										break;
									}
									if (!button5_StateMain_promotePawnToBishop) /* Read Bishop promote button. */
									{
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
										ws2812_setleds(led,MAXPIX);
										_delay_ms(200);
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
										ws2812_setleds(led,MAXPIX);
										piecesPositionMatrix.values[currentPieceRow][currentPieceColumn] = NO_PIECE;
										if (currentPieceMove == WHITE_PAWN)
										{
											piecesPositionMatrix.values[i][j] = WHITE_BISHOP; /* Promote white pawn to bishop. */
										}
										if (currentPieceMove == BLACK_PAWN)
										{
											piecesPositionMatrix.values[i][j] = BLACK_BISHOP; /* Promote black pawn to bishop. */
										}
										break;
									}
									if (!button6_StateMain_promotePawnToRook) /* Read Rook promote button. */
									{
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
										ws2812_setleds(led,MAXPIX);
										_delay_ms(200);
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
										ws2812_setleds(led,MAXPIX);
										piecesPositionMatrix.values[currentPieceRow][currentPieceColumn] = NO_PIECE;
										if (currentPieceMove == WHITE_PAWN)
										{
											piecesPositionMatrix.values[i][j] = WHITE_ROOK; /* Promote white pawn to rook. */
										}
										if (currentPieceMove == BLACK_PAWN)
										{
											piecesPositionMatrix.values[i][j] = BLACK_ROOK; /* Promote black pawn to rook. */
										}
										break;
									}
									if (!button7_StateMain_promotePawnToQueen) /* Read Queen promote button. */
									{
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 255;
										ws2812_setleds(led,MAXPIX);
										_delay_ms(200);
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
										led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
										ws2812_setleds(led,MAXPIX);
										piecesPositionMatrix.values[currentPieceRow][currentPieceColumn] = NO_PIECE;
										if (currentPieceMove == WHITE_PAWN)
										{
											piecesPositionMatrix.values[i][j] = WHITE_QUEEN; /* Promote white pawn to queen. */
										}
										if (currentPieceMove == BLACK_PAWN)
										{
											piecesPositionMatrix.values[i][j] = BLACK_QUEEN; /* Promote black pawn to queen. */
										}
										break;
									}
								} while (1);
							}
						
						

						

							//set led for legal move

							if ((currentPieceMove == WHITE_KING) && (first_white_king_move == 1) && ((j - currentPieceColumn == 2) || (currentPieceColumn - j == 2)))
							{
								if (newLedMatrix.values[currentPieceRow][currentPieceColumn] == 1)
								{
									if ((newLedMatrix.values[currentPieceRow][currentPieceColumn - 1] == 1) && (newLedMatrix.values[currentPieceRow][currentPieceColumn - 2] == 1))
									{
										if (first_a_column_white_rook_move == 1)
										{
											whiteCastleInProgress = 1; /* Long castle need also a rook move from a1 to d1. */
										}
									}
									else
									{
										newLedMatrix.values[currentPieceRow][currentPieceColumn - 2] = 0; // Long castle is not available for the white king.
									}
									if ((newLedMatrix.values[currentPieceRow][currentPieceColumn + 1] == 1) && (newLedMatrix.values[currentPieceRow][currentPieceColumn + 2] == 1))
									{
										if (first_h_column_white_rook_move == 1)
										{
											whiteCastleInProgress = 1;  /* Short castle need also a rook move from h1 to f1. */
										}
									}
									else
									{
										newLedMatrix.values[currentPieceRow][currentPieceColumn + 2] = 0; // Short castle is not available for the white king.
									}
								}
							}
							else
							{
								if (currentPieceMove == WHITE_KING)
								{
									first_white_king_move = 0; /* White king made a move. He can't castle this game anymore.*/
								}
							}

							if ((currentPieceMove == BLACK_KING) && (first_black_king_move == 1) && ((j - currentPieceColumn == 2) || (currentPieceColumn - j == 2)))
							{
								if (newLedMatrix.values[currentPieceRow][currentPieceColumn] == 1)
								{
									if ((newLedMatrix.values[currentPieceRow][currentPieceColumn - 1] == 1) && (newLedMatrix.values[currentPieceRow][currentPieceColumn - 2] == 1))
									{
										if (first_a_column_black_rook_move == 1)
										{
											blackCastleInProgress = 1; /* Long castle need also a rook move from a8 to d8. */
										}
									}
									else
									{
										newLedMatrix.values[currentPieceRow][currentPieceColumn - 2] = 0; // Long castle is not available for the black king.
									}
									if ((newLedMatrix.values[currentPieceRow][currentPieceColumn + 1] == 1) && (newLedMatrix.values[currentPieceRow][currentPieceColumn + 2] == 1))
									{
										if (first_h_column_black_rook_move == 1)
										{
											blackCastleInProgress = 1;  /* Short castle need also a rook move from h8 to f8. */
										}
									}
									else
									{
										newLedMatrix.values[currentPieceRow][currentPieceColumn + 2] = 0; // Short castle is not available for the black king.
									}
								}
							}
							else
							{
								if (currentPieceMove == BLACK_KING)
								{
									first_black_king_move = 0; /* Black king made a move. He can't castle this game anymore.*/
								}
							}

							if ((whiteCastleInProgress == 1) && (currentPieceMove == WHITE_ROOK))
							{
								whiteCastleInProgress = 0; /* Long or short castle completed. */
								first_white_king_move = 0;
							}

							if ((blackCastleInProgress == 1) && (currentPieceMove == BLACK_ROOK))
							{
								blackCastleInProgress = 0; /* Long or short castle completed. */
								first_black_king_move = 0;
							}

							if (currentPieceMove == WHITE_ROOK)
							{
								if (currentPieceColumn == 0)
								{
									first_a_column_white_rook_move = 0; /* White a1 rook made a move. White king can't castle long this game anymore.*/
								}
								else
								{
									if (currentPieceColumn == 7)
									{
										first_h_column_white_rook_move = 0; /* White h1 rook made a move. White king can't castle short this game anymore.*/
									}
								}
							}

							if (currentPieceMove == BLACK_ROOK)
							{
								if (currentPieceColumn == 0)
								{
									first_a_column_black_rook_move = 0;  /* Black a8 rook made a move. Black king can't castle long this game anymore.*/
								}
								else
								{
									if (currentPieceColumn == 7)
									{
										first_h_column_black_rook_move = 0; /* Black h8 rook made a move. Black king can't castle short this game anymore.*/
									}
								}
							}

							if ((!((i == currentPieceRow) && (j == currentPieceColumn))) && (whiteCastleInProgress == 0) && (blackCastleInProgress == 0))
							{
								//enable chess clock press
								chessClockPressEnable = 1;
								nrOfPiecesHeld = 0;

								/*
									if ((clock_button == pressed) && (chessClockPressEnable == 1))
									{
										//switch whoMoves
										if (whoMoves == WHITE_MOVES)
										{
											whoMoves = BLACK_MOVES;
										}
										else
										{
											if (whoMoves == BLACK_MOVES)
											{
												whoMoves = WHITE_MOVES;
											}
										}
										Start_black_clock();
										chessClockPressEnable = 0;
									}
								*/


								
								if (whoMoves == WHITE_MOVES)
								{
									whoMoves = BLACK_MOVES;
								}
								else
								{
									if (whoMoves == BLACK_MOVES)
									{
										whoMoves = WHITE_MOVES;
									}
								}
							}
							else
							{
								/* Do nothing.*/
							}
							//reset matrix
							currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
							newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);

							/*---------------------CHECKMATE ALGORITHM------------------------------*/
							//CHECKMATE ALGORITHM:
							//Check possible checkmate!
							//(if whoMoves == WHITE_MOVES -> check white king safety)
							//(if whoMoves == BLACK_MOVES -> check black king safety)
							if (whoMoves == WHITE_MOVES)
							{
								/*DEBUG*/ /*printf("White king safety!");*/
								/* Check white king current position. */
								for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
								{
									for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
									{
										if (piecesPositionMatrix.values[i][j] == WHITE_KING)
										{
											white_king_currentRow_checkmateDetection = i;
											white_king_currentColumn_checkmateDetection = j;
										}
									}
								}
								/* Check white king number of possible moves (store them in newLedMatrix). */
								newLedMatrix = eliminatePossibleChecksForCurrentKing(WHITE_KING, white_king_currentRow_checkmateDetection, white_king_currentColumn_checkmateDetection);
								white_king_numberOfPossibleMoves = 0;
								for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
								{
									for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
									{
										if (piecesPositionMatrix.values[i][j] == WHITE_KING)
										{
											white_king_numberOfPossibleMoves += newLedMatrix.values[i][j];
											break;
										}
									}
								}

								//reset matrix
								currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
								newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
								if (white_king_numberOfPossibleMoves == 0)
								{
									/* Check the number of black pieces which attack the white king */
									numberOfPiecesWhichTreatMate = 0;
									for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
									{
										for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
										{
											switch (piecesPositionMatrix.values[i][j])
											{
											case BLACK_PAWN:
											{
												setLedsForPossibleMovesPawn(piecesPositionMatrix.values[i][j], i, j);
												if (newLedMatrix.values[white_king_currentRow_checkmateDetection][white_king_currentColumn_checkmateDetection] == 1)
												{
													pieceWhichTreatMate_currentRow = i;
													pieceWhichTreatMate_currentColumn = j;
													numberOfPiecesWhichTreatMate++;
												}
												break;
											}
											case BLACK_KNIGHT:
											{
												setLedsForPossibleMovesKnight(piecesPositionMatrix.values[i][j], i, j);
												if (newLedMatrix.values[white_king_currentRow_checkmateDetection][white_king_currentColumn_checkmateDetection] == 1)
												{
													pieceWhichTreatMate_currentRow = i;
													pieceWhichTreatMate_currentColumn = j;
													numberOfPiecesWhichTreatMate++;
												}
												break;
											}
											case BLACK_BISHOP:
											{
												setLedsForPossibleMovesBishop(piecesPositionMatrix.values[i][j], i, j);
												if (newLedMatrix.values[white_king_currentRow_checkmateDetection][white_king_currentColumn_checkmateDetection] == 1)
												{
													pieceWhichTreatMate_currentRow = i;
													pieceWhichTreatMate_currentColumn = j;
													numberOfPiecesWhichTreatMate++;
												}
												break;
											}
											case BLACK_ROOK:
											{
												setLedsForPossibleMovesRook(piecesPositionMatrix.values[i][j], i, j, whiteCastleInProgress, blackCastleInProgress);
												if (newLedMatrix.values[white_king_currentRow_checkmateDetection][white_king_currentColumn_checkmateDetection] == 1)
												{
													pieceWhichTreatMate_currentRow = i;
													pieceWhichTreatMate_currentColumn = j;
													numberOfPiecesWhichTreatMate++;
												}
												break;
											}
											case BLACK_QUEEN:
											{
												setLedsForPossibleMovesQueen(piecesPositionMatrix.values[i][j], i, j);
												if (newLedMatrix.values[white_king_currentRow_checkmateDetection][white_king_currentColumn_checkmateDetection] == 1)
												{
													pieceWhichTreatMate_currentRow = i;
													pieceWhichTreatMate_currentColumn = j;
													numberOfPiecesWhichTreatMate++;
												}
												break;
											}
											}
											//reset matrix
											currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
											newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
										}
									}
									if (numberOfPiecesWhichTreatMate > 1)
									{
										//It's checkmate! Black wins
										/*DEBUG*/ /*printf("Checkmate! Black wins!");*/
										checkMate_flag = 1;
									}
									else
									{
										if (numberOfPiecesWhichTreatMate == 1)
										{
											checkMate_flag = 1; /* Assume it's checkmate. */
											//Capture or block the piece which treat checkmate!
											/*DEBUG*/ /*printf("It's not probably checkmate yet!");*/
											/* The next if it's used in diagonal white king treat detection. */
											if ((piecesPositionMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == BLACK_BISHOP) ||
												(piecesPositionMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == BLACK_QUEEN))
											{
												setLedsForPossibleMovesBishop(WHITE_BISHOP, white_king_currentRow_checkmateDetection, white_king_currentColumn_checkmateDetection);
												current_king_checkmateDetectionMatrix = newLedMatrix;

												//reset matrix
												currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
												newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);

												setLedsForPossibleMovesBishop(piecesPositionMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn], pieceWhichTreatMate_currentRow, pieceWhichTreatMate_currentColumn);
												pieceWhichTreatMateMatrix = newLedMatrix;

												//reset matrix
												currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
												newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
											}

											/* Extract diagonal fields to cover. (for threatening the white king on the diagonal). */
											/*-------------------------------------------------------------------------------------*/
											// Reseteaza contorul vectorilor in care vor fi stocate campurile de pe diagonala ce pot fi acoperite pentru a evita matul.
											// <contor_vectori> = 0
											diagonalTreatFields_vectorsIndexCounter = 0;
											for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
											{
												for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
												{
													if (((current_king_checkmateDetectionMatrix.values[i][j] * pieceWhichTreatMateMatrix.values[i][j]) != 0) &&
														(i != white_king_currentRow_checkmateDetection) && (i != pieceWhichTreatMate_currentRow) &&
														(j != white_king_currentColumn_checkmateDetection) && (j != pieceWhichTreatMate_currentColumn))
													{
														//<vector_linii>[<contor_vectori>] = i;
														diagonalTreatFields_rowVector[diagonalTreatFields_vectorsIndexCounter] = i;
														//<vector_coloane>[<contor_vectori>] = j;
														diagonalTreatFields_columnVector[diagonalTreatFields_vectorsIndexCounter] = j;
														//<contor_vectori>++;
														diagonalTreatFields_vectorsIndexCounter++;
													}
												}
											}
											/*-------------------------------------------------------------------------------------*/
											for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
											{
												for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
												{
													switch (piecesPositionMatrix.values[i][j])
													{
													case WHITE_PAWN:
													{
														setLedsForPossibleMovesPawn(piecesPositionMatrix.values[i][j], i, j);
														if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
														{
															/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
															checkMate_flag = 0;
														}
														else
														{
															if ((white_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																(((j > white_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																((j >= pieceWhichTreatMate_currentColumn) && (j < white_king_currentColumn_checkmateDetection))))
															{
																if (newLedMatrix.values[white_king_currentRow_checkmateDetection][j] != 0)
																{
																	/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																	checkMate_flag = 0;
																}
															}
															else
															{
																if ((white_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																	(((i > white_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																	((i >= pieceWhichTreatMate_currentRow) && (i < white_king_currentRow_checkmateDetection))))
																{
																	if (newLedMatrix.values[i][white_king_currentColumn_checkmateDetection] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																	for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																	{
																		if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																			((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																			checkMate_flag = 0;
																		}
																		//diagonalTreatFields clear vectors
																		diagonalTreatFields_rowVector[k] = 0;
																		diagonalTreatFields_columnVector[k] = 0;
																	}
																}
															}
														}
														break;
													}
													case WHITE_KNIGHT:
													{
														setLedsForPossibleMovesKnight(piecesPositionMatrix.values[i][j], i, j);
														if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
														{
															/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
															checkMate_flag = 0;
														}
														else
														{
															if ((white_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																(((j > white_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																((j >= pieceWhichTreatMate_currentColumn) && (j < white_king_currentColumn_checkmateDetection))))
															{
																if (newLedMatrix.values[white_king_currentRow_checkmateDetection][j] != 0)
																{
																	/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																	checkMate_flag = 0;
																}
															}
															else
															{
																if ((white_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																	(((i > white_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																	((i >= pieceWhichTreatMate_currentRow) && (i < white_king_currentRow_checkmateDetection))))
																{
																	if (newLedMatrix.values[i][white_king_currentColumn_checkmateDetection] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																	for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																	{
																		if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																			((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																			checkMate_flag = 0;
																		}
																		//diagonalTreatFields clear vectors
																		diagonalTreatFields_rowVector[k] = 0;
																		diagonalTreatFields_columnVector[k] = 0;
																	}
																}
															}
														}
														break;
													}
													case WHITE_BISHOP:
													{
														setLedsForPossibleMovesBishop(piecesPositionMatrix.values[i][j], i, j);
														if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
														{
															/*DEBUG*/ /* printf("It's not checkmate( Case 1 )!"); */
															checkMate_flag = 0;
														}
														else
														{
															if ((white_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																(((j > white_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																((j >= pieceWhichTreatMate_currentColumn) && (j < white_king_currentColumn_checkmateDetection))))
															{
																if (newLedMatrix.values[white_king_currentRow_checkmateDetection][j] != 0)
																{
																	/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!"); */
																	checkMate_flag = 0;
																}
															}
															else
															{
																if ((white_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																	(((i > white_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																	((i >= pieceWhichTreatMate_currentRow) && (i < white_king_currentRow_checkmateDetection))))
																{
																	if (newLedMatrix.values[i][white_king_currentColumn_checkmateDetection] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																	for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																	{
																		if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																			((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																			checkMate_flag = 0;
																		}
																		//diagonalTreatFields clear vectors
																		diagonalTreatFields_rowVector[k] = 0;
																		diagonalTreatFields_columnVector[k] = 0;
																	}
																}
															}
														}
														break;
													}
													case WHITE_ROOK:
													{
														setLedsForPossibleMovesRook(piecesPositionMatrix.values[i][j], i, j, whiteCastleInProgress, blackCastleInProgress);
														if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
														{
															/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
															checkMate_flag = 0;
														}
														else
														{
															if ((white_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																(((j > white_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																((j >= pieceWhichTreatMate_currentColumn) && (j < white_king_currentColumn_checkmateDetection))))
															{
																if (newLedMatrix.values[white_king_currentRow_checkmateDetection][j] != 0)
																{
																	/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																	checkMate_flag = 0;
																}
															}
															else
															{
																if ((white_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																	(((i > white_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																	((i >= pieceWhichTreatMate_currentRow) && (i < white_king_currentRow_checkmateDetection))))
																{
																	if (newLedMatrix.values[i][white_king_currentColumn_checkmateDetection] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																	for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																	{
																		if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																			((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																			checkMate_flag = 0;
																		}
																		//diagonalTreatFields clear vectors
																		diagonalTreatFields_rowVector[k] = 0;
																		diagonalTreatFields_columnVector[k] = 0;
																	}
																}
															}
														}
														break;
													}
													case WHITE_QUEEN:
													{
														setLedsForPossibleMovesQueen(piecesPositionMatrix.values[i][j], i, j);
														if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
														{
															/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
															checkMate_flag = 0;
														}
														else
														{
															if ((white_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																(((j > white_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																((j >= pieceWhichTreatMate_currentColumn) && (j < white_king_currentColumn_checkmateDetection))))
															{
																if (newLedMatrix.values[white_king_currentRow_checkmateDetection][j] != 0)
																{
																	/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																	checkMate_flag = 0;
																}
															}
															else
															{
																if ((white_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																	(((i > white_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																	((i >= pieceWhichTreatMate_currentRow) && (i < white_king_currentRow_checkmateDetection))))
																{
																	if (newLedMatrix.values[i][white_king_currentColumn_checkmateDetection] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																	for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																	{
																		if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																			((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																			checkMate_flag = 0;
																		}
																		//diagonalTreatFields clear vectors
																		diagonalTreatFields_rowVector[k] = 0;
																		diagonalTreatFields_columnVector[k] = 0;
																	}
																}
															}
														}
														break;
													}
													}
													//reset matrix
													currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
													newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
												}
											}
										}
									}
								}
								else
								{
									checkMate_flag = 0;
								}
							}
							else
							{
								if (whoMoves == BLACK_MOVES)
								{
									/*DEBUG*/ /*printf("Black king safety!");*/
									/*------Black_king-------*/
									/* Check black king current position. */
									for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
									{
										for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
										{
											if (piecesPositionMatrix.values[i][j] == BLACK_KING)
											{
												black_king_currentRow_checkmateDetection = i;
												black_king_currentColumn_checkmateDetection = j;
											}
										}
									}
									/* Check white king number of possible moves (store them in newLedMatrix). */
									newLedMatrix = eliminatePossibleChecksForCurrentKing(BLACK_KING, black_king_currentRow_checkmateDetection, black_king_currentColumn_checkmateDetection);
									black_king_numberOfPossibleMoves = 0;
									for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
									{
										for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
										{
											if (piecesPositionMatrix.values[i][j] == BLACK_KING)
											{
												black_king_numberOfPossibleMoves += newLedMatrix.values[i][j];
												break;
											}
										}
									}

									//reset matrix
									currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
									newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
									if (black_king_numberOfPossibleMoves == 0)
									{
										/* Check the number of black pieces which attack the white king */
										numberOfPiecesWhichTreatMate = 0;
										for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
										{
											for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
											{
												switch (piecesPositionMatrix.values[i][j])
												{
												case WHITE_PAWN:
												{
													setLedsForPossibleMovesPawn(piecesPositionMatrix.values[i][j], i, j);
													if (newLedMatrix.values[black_king_currentRow_checkmateDetection][black_king_currentColumn_checkmateDetection] == 1)
													{
														pieceWhichTreatMate_currentRow = i;
														pieceWhichTreatMate_currentColumn = j;
														numberOfPiecesWhichTreatMate++;
													}
													break;
												}
												case WHITE_KNIGHT:
												{
													setLedsForPossibleMovesKnight(piecesPositionMatrix.values[i][j], i, j);
													if (newLedMatrix.values[black_king_currentRow_checkmateDetection][black_king_currentColumn_checkmateDetection] == 1)
													{
														pieceWhichTreatMate_currentRow = i;
														pieceWhichTreatMate_currentColumn = j;
														numberOfPiecesWhichTreatMate++;
													}
													break;
												}
												case WHITE_BISHOP:
												{
													setLedsForPossibleMovesBishop(piecesPositionMatrix.values[i][j], i, j);
													if (newLedMatrix.values[black_king_currentRow_checkmateDetection][black_king_currentColumn_checkmateDetection] == 1)
													{
														pieceWhichTreatMate_currentRow = i;
														pieceWhichTreatMate_currentColumn = j;
														numberOfPiecesWhichTreatMate++;
													}
													break;
												}
												case WHITE_ROOK:
												{
													setLedsForPossibleMovesRook(piecesPositionMatrix.values[i][j], i, j, whiteCastleInProgress, blackCastleInProgress);
													if (newLedMatrix.values[black_king_currentRow_checkmateDetection][black_king_currentColumn_checkmateDetection] == 1)
													{
														pieceWhichTreatMate_currentRow = i;
														pieceWhichTreatMate_currentColumn = j;
														numberOfPiecesWhichTreatMate++;
													}
													break;
												}
												case WHITE_QUEEN:
												{
													setLedsForPossibleMovesQueen(piecesPositionMatrix.values[i][j], i, j);
													if (newLedMatrix.values[black_king_currentRow_checkmateDetection][black_king_currentColumn_checkmateDetection] == 1)
													{
														pieceWhichTreatMate_currentRow = i;
														pieceWhichTreatMate_currentColumn = j;
														numberOfPiecesWhichTreatMate++;
													}
													break;
												}
												}
												//reset matrix
												currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
												newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
											}
										}
										if (numberOfPiecesWhichTreatMate > 1)
										{
											//It's checkmate! White wins
											/*DEBUG*/ /*printf("Checkmate! White wins!");*/
											checkMate_flag = 1;
										}
										else
										{
											if (numberOfPiecesWhichTreatMate == 1)
											{
												checkMate_flag = 1; /* Assume it's checkmate. */
												//Capture or block the piece which treat checkmate!
												/*DEBUG*/ /*printf("It's not probably checkmate yet!");*/
												/* The next if it's used in diagonal black king treat detection. */
												if ((piecesPositionMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == WHITE_BISHOP) ||
													(piecesPositionMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == WHITE_QUEEN))
												{
													setLedsForPossibleMovesBishop(BLACK_BISHOP, black_king_currentRow_checkmateDetection, black_king_currentColumn_checkmateDetection);
													current_king_checkmateDetectionMatrix = newLedMatrix;

													//reset matrix
													currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
													newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);

													setLedsForPossibleMovesBishop(piecesPositionMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn], pieceWhichTreatMate_currentRow, pieceWhichTreatMate_currentColumn);
													pieceWhichTreatMateMatrix = newLedMatrix;

													//reset matrix
													currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
													newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
												}

												/* Extract diagonal fields to cover. (for threatening the black king on the diagonal). */
												/*-------------------------------------------------------------------------------------*/
												// Reseteaza contorul vectorilor in care vor fi stocate campurile de pe diagonala ce pot fi acoperite pentru a evita matul.
												// <contor_vectori> = 0
												diagonalTreatFields_vectorsIndexCounter = 0;
												for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
												{
													for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
													{
														if (((current_king_checkmateDetectionMatrix.values[i][j] * pieceWhichTreatMateMatrix.values[i][j]) != 0) &&
															(i != black_king_currentRow_checkmateDetection) && (i != pieceWhichTreatMate_currentRow) &&
															(j != black_king_currentColumn_checkmateDetection) && (j != pieceWhichTreatMate_currentColumn))
														{
															//<vector_linii>[<contor_vectori>] = i;
															diagonalTreatFields_rowVector[diagonalTreatFields_vectorsIndexCounter] = i;
															//<vector_coloane>[<contor_vectori>] = j;
															diagonalTreatFields_columnVector[diagonalTreatFields_vectorsIndexCounter] = j;
															//<contor_vectori>++;
															diagonalTreatFields_vectorsIndexCounter++;
														}
													}
												}
												/*-------------------------------------------------------------------------------------*/
												for (int8_t i = 0; i < NEWSENSORVALUESMATRIX_NR_ROWS; i++)
												{
													for (int8_t j = 0; j < NEWSENSORVALUESMATRIX_NR_COLUMNS; j++)
													{
														switch (piecesPositionMatrix.values[i][j])
														{
														case BLACK_PAWN:
														{
															setLedsForPossibleMovesPawn(piecesPositionMatrix.values[i][j], i, j);
															if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
															{
																/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
																checkMate_flag = 0;
															}
															else
															{
																if ((black_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																	(((j > black_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																	((j >= pieceWhichTreatMate_currentColumn) && (j < black_king_currentColumn_checkmateDetection))))
																{
																	if (newLedMatrix.values[black_king_currentRow_checkmateDetection][j] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	if ((black_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																		(((i > black_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																		((i >= pieceWhichTreatMate_currentRow) && (i < black_king_currentRow_checkmateDetection))))
																	{
																		if (newLedMatrix.values[i][black_king_currentColumn_checkmateDetection] != 0)
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																			checkMate_flag = 0;
																		}
																	}
																	else
																	{
																		/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																		for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																		{
																			if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																				((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																			{
																				/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																				checkMate_flag = 0;
																			}
																			//diagonalTreatFields clear vectors
																			diagonalTreatFields_rowVector[k] = 0;
																			diagonalTreatFields_columnVector[k] = 0;
																		}
																	}
																}
															}
															break;
														}
														case BLACK_KNIGHT:
														{
															setLedsForPossibleMovesKnight(piecesPositionMatrix.values[i][j], i, j);
															if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
															{
																/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
																checkMate_flag = 0;
															}
															else
															{
																if ((black_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																	(((j > black_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																	((j >= pieceWhichTreatMate_currentColumn) && (j < black_king_currentColumn_checkmateDetection))))
																{
																	if (newLedMatrix.values[black_king_currentRow_checkmateDetection][j] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	if ((black_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																		(((i > black_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																		((i >= pieceWhichTreatMate_currentRow) && (i < black_king_currentRow_checkmateDetection))))
																	{
																		if (newLedMatrix.values[i][black_king_currentColumn_checkmateDetection] != 0)
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																			checkMate_flag = 0;
																		}
																	}
																	else
																	{
																		/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																		for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																		{
																			if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																				((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																			{
																				/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!"); */
																				checkMate_flag = 0;
																			}
																			//diagonalTreatFields clear vectors
																			diagonalTreatFields_rowVector[k] = 0;
																			diagonalTreatFields_columnVector[k] = 0;
																		}
																	}
																}
															}
															break;
														}
														case BLACK_BISHOP:
														{
															setLedsForPossibleMovesBishop(piecesPositionMatrix.values[i][j], i, j);
															if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
															{
																/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
																checkMate_flag = 0;
															}
															else
															{
																if ((black_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																	(((j > black_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																	((j >= pieceWhichTreatMate_currentColumn) && (j < black_king_currentColumn_checkmateDetection))))
																{
																	if (newLedMatrix.values[black_king_currentRow_checkmateDetection][j] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	if ((black_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																		(((i > black_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																		((i >= pieceWhichTreatMate_currentRow) && (i < black_king_currentRow_checkmateDetection))))
																	{
																		if (newLedMatrix.values[i][black_king_currentColumn_checkmateDetection] != 0)
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																			checkMate_flag = 0;
																		}
																	}
																	else
																	{
																		/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																		for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																		{
																			if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																				((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																			{
																				/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																				checkMate_flag = 0;
																			}
																			//diagonalTreatFields clear vectors
																			diagonalTreatFields_rowVector[k] = 0;
																			diagonalTreatFields_columnVector[k] = 0;
																		}
																	}
																}
															}
															break;
														}
														case BLACK_ROOK:
														{
															setLedsForPossibleMovesRook(piecesPositionMatrix.values[i][j], i, j, whiteCastleInProgress, blackCastleInProgress);
															if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
															{
																/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
																checkMate_flag = 0;
															}
															else
															{
																if ((black_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																	(((j > black_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																	((j >= pieceWhichTreatMate_currentColumn) && (j < black_king_currentColumn_checkmateDetection))))
																{
																	if (newLedMatrix.values[black_king_currentRow_checkmateDetection][j] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	if ((black_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																		(((i > black_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																		((i >= pieceWhichTreatMate_currentRow) && (i < black_king_currentRow_checkmateDetection))))
																	{
																		if (newLedMatrix.values[i][black_king_currentColumn_checkmateDetection] != 0)
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																			checkMate_flag = 0;
																		}
																	}
																	else
																	{
																		/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																		for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																		{
																			if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																				((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																			{
																				/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																				checkMate_flag = 0;
																			}
																			//diagonalTreatFields clear vectors
																			diagonalTreatFields_rowVector[k] = 0;
																			diagonalTreatFields_columnVector[k] = 0;
																		}
																	}
																}
															}
															break;
														}
														case BLACK_QUEEN:
														{
															setLedsForPossibleMovesQueen(piecesPositionMatrix.values[i][j], i, j);
															if (newLedMatrix.values[pieceWhichTreatMate_currentRow][pieceWhichTreatMate_currentColumn] == 1)
															{
																/*DEBUG*/ /*printf("It's not checkmate( Case 1 )!");*/
																checkMate_flag = 0;
															}
															else
															{
																if ((black_king_currentRow_checkmateDetection == pieceWhichTreatMate_currentRow) &&
																	(((j > black_king_currentColumn_checkmateDetection) && (j <= pieceWhichTreatMate_currentColumn)) ||
																	((j >= pieceWhichTreatMate_currentColumn) && (j < black_king_currentColumn_checkmateDetection))))
																{
																	if (newLedMatrix.values[black_king_currentRow_checkmateDetection][j] != 0)
																	{
																		/*DEBUG*/ /*printf("It's not checkmate( Case 2 )!");*/
																		checkMate_flag = 0;
																	}
																}
																else
																{
																	if ((black_king_currentColumn_checkmateDetection == pieceWhichTreatMate_currentColumn) &&
																		(((i > black_king_currentRow_checkmateDetection) && (i <= pieceWhichTreatMate_currentRow)) ||
																		((i >= pieceWhichTreatMate_currentRow) && (i < black_king_currentRow_checkmateDetection))))
																	{
																		if (newLedMatrix.values[i][black_king_currentColumn_checkmateDetection] != 0)
																		{
																			/*DEBUG*/ /*printf("It's not checkmate( Case 3 )!");*/
																			checkMate_flag = 0;
																		}
																	}
																	else
																	{
																		/*DEBUG*/ /*printf("It's diagonal attack or it's mate!");*/
																		for (int8_t k = 0; k < DIAGONALTREATFIELDS_LENGTH; k++)
																		{
																			if ((newLedMatrix.values[diagonalTreatFields_rowVector[k]][diagonalTreatFields_columnVector[k]] != 0) &&
																				((diagonalTreatFields_rowVector[k] != 0) || (diagonalTreatFields_columnVector[k] != 0)))
																			{
																				/*DEBUG*/ /*printf("It's not checkmate( Case 4 )!");*/
																				checkMate_flag = 0;
																			}
																			//diagonalTreatFields clear vectors
																			diagonalTreatFields_rowVector[k] = 0;
																			diagonalTreatFields_columnVector[k] = 0;
																		}
																	}
																}
															}
															break;
														}
														}
														//reset matrix
														currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
														newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
													}
												}
											}
										}
									}
									else
									{
										checkMate_flag = 0;
									}
									/*------Black king-------*/
								}
							}

							if (checkMate_flag == 1)
							{
								do
								{
									if (whoMoves == WHITE_MOVES)
									{
										/*DEBUG*/ /*printf("Congratulation! Black won!");*/
										//WS2812_MATRIX_setLedsBasedOnSensorValuesAndColor((uint64_t)sensorValues, TURQOISE);
										led[ledVectorOrderOfIndexes_main[white_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + white_king_currentColumn_checkmateDetection]].r = 0;
										led[ledVectorOrderOfIndexes_main[white_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + white_king_currentColumn_checkmateDetection]].g = 255;
										led[ledVectorOrderOfIndexes_main[white_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + white_king_currentColumn_checkmateDetection]].b = 255;
										ws2812_setleds(led,MAXPIX);
										_delay_ms(200);
										led[ledVectorOrderOfIndexes_main[white_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + white_king_currentColumn_checkmateDetection]].r = 0;
										led[ledVectorOrderOfIndexes_main[white_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + white_king_currentColumn_checkmateDetection]].g = 0;
										led[ledVectorOrderOfIndexes_main[white_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + white_king_currentColumn_checkmateDetection]].b = 0;
										ws2812_setleds(led,MAXPIX);
										_delay_ms(200);
									}
									else
									{
										if (whoMoves == BLACK_MOVES)
										{
											/*DEBUG*/ /*printf("Congratulation! White won!");*/
											//WS2812_MATRIX_setLedsBasedOnSensorValuesAndColor((uint64_t)sensorValues, YELLOW);
											led[ledVectorOrderOfIndexes_main[black_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + black_king_currentColumn_checkmateDetection]].r = 255;
											led[ledVectorOrderOfIndexes_main[black_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + black_king_currentColumn_checkmateDetection]].g = 255;
											led[ledVectorOrderOfIndexes_main[black_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + black_king_currentColumn_checkmateDetection]].b = 0;
											ws2812_setleds(led,MAXPIX);
											_delay_ms(200);
											led[ledVectorOrderOfIndexes_main[black_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + black_king_currentColumn_checkmateDetection]].r = 0;
											led[ledVectorOrderOfIndexes_main[black_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + black_king_currentColumn_checkmateDetection]].g = 0;
											led[ledVectorOrderOfIndexes_main[black_king_currentRow_checkmateDetection * NEWLEDMATRIX_NR_COLUMNS + black_king_currentColumn_checkmateDetection]].b = 0;
											ws2812_setleds(led,MAXPIX);
											_delay_ms(200);
										}
									}
									//PRESS START BUTTON TO SET A NEW GAME
									button3_StateMain_startGame = Button_debounce(&READ_PIN_Buttons, PIN_Button3_startGame);
									if(!button3_StateMain_startGame)
									{
										break;
									}
								} while (1);
							}

							//reset matrix
							current_king_checkmateDetectionMatrix = Matrix_uint8_t_reset_values(current_king_checkmateDetectionMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
							pieceWhichTreatMateMatrix = Matrix_uint8_t_reset_values(pieceWhichTreatMateMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
							/*---------------------CHECKMATE ALGORITHM------------------------------*/

							//reset matrix
							currentPiecePossibleMove = Matrix_uint8_t_reset_values(currentPiecePossibleMove, NEWSENSORVALUESMATRIX_NR_ROWS, NEWSENSORVALUESMATRIX_NR_COLUMNS);
							newLedMatrix = Matrix_uint8_t_reset_values(newLedMatrix, NEWLEDMATRIX_NR_ROWS, NEWLEDMATRIX_NR_COLUMNS);
						}
						else
						{
							//ILEGAL move
							//WS2812_MATRIX_setLedsBasedOnSensorValuesAndColor((uint64_t)sensorValues, RED);
							led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].r = 255;
							led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].g = 0;
							led[ledVectorOrderOfIndexes_main[i * NEWLEDMATRIX_NR_COLUMNS + j]].b = 0;
							ws2812_setleds(led,MAXPIX);
							/*DEBUG*/ /*printf("it's not legal; i = %d, j = %d, currentPieceRow = %d, currentPieceColumn = %d, newLedMatrix = %d\n", i, j, currentPieceRow, currentPieceColumn, newLedMatrix.values[i][j]);*/
							/*DEBUG*/ /*printf("contor = %d", contor);*/
							//don't put the piece on the new field
							//set led for ilegal move
							//disable chess clock press
							chessClockPressEnable = 0;
							//don't switch whoMoves
							//don't reset matrix7													
						}
					}
				}
			}


			// activate promote pawn logic if it's necessary and set different colors for LEDs based on piece promotion type

			/*DEBUG lines*/
			/*
			printf("sensorValues = %I64X\n", sensorValues);
			printf("newLedMatrix values:\n");
			for (int8_t i = 0; i < NEWLEDMATRIX_NR_ROWS; i++)
			{
				for (int8_t j = 0; j < NEWLEDMATRIX_NR_COLUMNS; j++)
				{
					printf("%d\t", newLedMatrix.values[i][j]);
				}
				printf("\n");
			}

			printf("\n");
			printf("piecesPositionMatrix values: \n");
			for (int8_t i = 0; i < NEWLEDMATRIX_NR_ROWS; i++)
			{
				for (int8_t j = 0; j < NEWLEDMATRIX_NR_COLUMNS; j++)
				{
					printf("%d\t", piecesPositionMatrix.values[i][j]);
				}
				printf("\n");
			}

			printf("\n");
			printf("whoMoves = %d", whoMoves);
			printf("\n");
			printf("chessClockPressEnable = %d", chessClockPressEnable);
			printf("\n");
			printf("whiteCastleInProgress = %d", whiteCastleInProgress);
			printf("\n");
			printf("blackCastleInProgress = %d", blackCastleInProgress);
			printf("\n");
			printf("first_white_king_move = %d", first_white_king_move);
			printf("\n");
			printf("first_a_column_white_rook_move = %d", first_a_column_white_rook_move);
			printf("\n");
			printf("first_h_column_white_rook_move = %d", first_h_column_white_rook_move);
			printf("\n");
			printf("first_black_king_move = %d", first_black_king_move);
			printf("\n");
			printf("first_a_column_black_rook_move = %d", first_a_column_black_rook_move);
			printf("\n");
			printf("first_h_column_black_rook_move = %d", first_h_column_black_rook_move);
			printf("\n\n");
			*/
		}
		
		if(contor > 0)
		{
			contor--;
		}
		else
		{
			contor = 0;
		}


		
				/*----------------------------------------------STATE_2_instructions---------------------------------------------------------------*/
				break;
			default:
				_delay_us(1); /* Just add an instruction in default state. */
				/* do nothing */
		}
	}
}
