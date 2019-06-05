#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

#include "timer.h"
//#include "io.c"
#include "MAX7219.c"

#include "consts.h"
#include "utils.c" // Init_LED_Matrices, Set_Brightness, Load_word, ...
#include "blocks.c" // Block drawing funcs

#define INFO_INDEX 0

#define ROTATE_BTN      (~PINA & 0x04)
#define SHIFT_RIGHT_BTN (~PINA & 0x02)
#define SHIFT_LEFT_BTN  (~PINA & 0x08)
typedef unsigned  char uc;
typedef   signed  char sc;
typedef unsigned short us;



void MakeRandom()
{
	uc matrix = 0; uc col = 0;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b01111111;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b00000011;	
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111101;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111101;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11011111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11110111;
	POS_ARRAY[matrix][col++] = 0b11111111;
}

void MakeWall()
{
	uc matrix = 0; uc col = 0;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	POS_ARRAY[matrix][col++] = 0b00000000;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	POS_ARRAY[matrix][col++] = 0b00000011;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	matrix++; col = 0;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
	POS_ARRAY[matrix][col++] = 0b11111111;
}
/*
uc Lateral_Collision(uc curr_matrix, uc col, sc from_bottom)
{
	
	if (from_bottom <= 0 && curr_matrix < 3)
	{
		return ((POS_ARRAY[curr_matrix + 1][col] & (0xFF)) > 0);
	}

	return ((POS_ARRAY[curr_matrix][col] & (0xFF)) > 0);
}
*/

void Rotation_Check(Block *block)
{
	if (ROTATE_BTN) // Orientation check
	{
		block->orientation = (block->orientation + 1) % 4;
		asm("nop");
		
		if ((block->orientation == Horizontal_Down || block->orientation == Horizontal_Up))// && (*left - *right) == 1)
		{
			// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Fix by using block.rotation_change
			// Maintains left side in bound
			if (block->left > 7)
				block->right = block->left - (block->width + block->rotation_change);
			else
				block->left = block->right + block->width  + block->rotation_change;

		}
		else
			block->left = block->right + block->width;
	}
}

void Shift_Left_Check(Block *block)
{
	if (SHIFT_RIGHT_BTN && block->right > 1) // Shift check
	{
		// Test for horizontal collision
		// When horizontal, there should be an extra space left for the left/right most light
		//			if (!Lateral_Collision(cur_matrix, *right - 1, from_bottom))
		//			{
		--block->left;
		--block->right;
		//			}
	}
}

void Shift_Right_Check(Block *block)
{
	if (SHIFT_LEFT_BTN && block->left < 8) // Shift check
	{
		// Test for horizontal collision
		// When horizontal, there should be an extra space left for the left/right most light
		//			if (!Lateral_Collision(cur_matrix, *left, from_bottom))
		//			{
		++block->left;
		++block->right;
		//			}
	}
}

void Drop_A_Block(
//	uc (*fct) (uc, uc, uc, sc, uc, uc),
	uc *cur_matrix,
	uc *is_done,
//	uc *left,
//	uc *right,
//	uc *orientation,
	Block *block,
	sc *from_bottom)
{
	uc contact = 0;
	
	if (!(*is_done))
	{
		Shift_Right_Check(block);
		Shift_Left_Check(block);
		Rotation_Check(block);
		Clear_All();
		
		// from_bottom < 0 if part of the block is on the lower matrix
		if (*from_bottom < 0 && *cur_matrix < BOTTOM_MATRIX)
		{
			// Not placing this fct call first erases the upper matrix's part of block
//			contact = (*fct)(*cur_matrix + 1, *left, *right, 8 + *from_bottom, *orientation, 0);
			contact = block->fct(*cur_matrix + 1, block->left, block->right, 8 + *from_bottom, block->orientation, 0);
		}
		contact = block->fct(*cur_matrix, block->left, block->right, *from_bottom, block->orientation, contact);
		--(*from_bottom);
		
		if(*from_bottom < -2 && *cur_matrix < 3 && !contact)
		{
			*from_bottom = 5;
			++(*cur_matrix);
		}
		else if ((*from_bottom < 0 && *cur_matrix >= 3) || contact)
		{
			*is_done = 0x01;
//			block->left = block->init_left;
//			block->right = block->init_right;
		}
	}
}


void Draw_Block_Matrix()
{
	for (uc col = 0; col < 8; col++)
	{
		for (uc matrix = 0; matrix < 4; matrix++)
		{
			MAX7219_SendByte(col+1);
			MAX7219_SendByte(POS_ARRAY[matrix][col]);
		}
		Load_Word();
	}
}

// This function should collapse the specified row
void Solid_Row_Collapser(uc curr_matrix, uc row)
{
	for (uc col = 0; col < 8; col++)
	{
		if (row == 0)
		{
			POS_ARRAY[curr_matrix][col] >>= 1;
		}
		else
		{
			uc above_row  = POS_ARRAY[curr_matrix][col] & (0xFF << (row + 1)); // Saves everything above solid row
			POS_ARRAY[curr_matrix][col] &= (0xFF >> (7 - (row - 1))); // Will mask off everything above solid row
			POS_ARRAY[curr_matrix][col] |= (above_row >> 1); // Shifts everything below solid row by one
		}
	}
}

Collapsed_Row_Merger(uc re[4][9])
{
	// matrix < 3 ensures that (matrix  + 1) will never be greater than 3
	for (uc matrix = 3; matrix > 0; --matrix)
	{
		for (uc col = 0; col < 8; ++col)
		{
			for (uc tmp_mtrx = matrix - 1; tmp_mtrx < 3; ++tmp_mtrx)
			{	
				uc tmp_col = POS_ARRAY[tmp_mtrx][col];
				
				POS_ARRAY[tmp_mtrx][col] = tmp_col >> (re[tmp_mtrx + 1][INFO_INDEX]);
			
				POS_ARRAY[tmp_mtrx + 1][col] |= (tmp_col << (7 - (re[tmp_mtrx + 1][INFO_INDEX] - 1)));
			}
		}
	}
}

void Solid_Row_Eliminator()
{
		/*
		 * re[0:4][ - ] = number of matrix starting from the top
		 * re[0:4][ 0 ] = number of solid rows per matrix
		 * re[0:4][1:8] = solid row numbers
		 */  
		uc re[4][9];
		uc elimination_sum = 0;
		for (uc curr_matrix = 0; curr_matrix < 4; curr_matrix++)
		{
			Solid_Row_Detector(curr_matrix, re[curr_matrix]);
					
			for (uc row = re[curr_matrix][INFO_INDEX]; row >= 1; --row)
				Solid_Row_Collapser(curr_matrix, re[curr_matrix][row]);
		}
		Collapsed_Row_Merger(re);
}
//***************************************** MAIN
int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;

	TimerSet(450);
	TimerOn();

	Init_LED_Matrices(4);
	Set_Brightness(0, 4);
	Clear_All();

	uc cur_matrix = 0;
	sc from_bottom = 7;
	uc is_done = 0;
	
	uc cur_matrix2 = 0;
	sc from_bottom2 = 7;
	uc is_done2 = 0;

	Block blocks[num_of_blocks];
	Initialize_Blocks(blocks);
	
//	uc (*fct[]) (uc, uc, uc, sc, uc, uc) = {Draw_L_Block, Draw_O_Block, Draw_I_Block};
//***************
//	MakeRandom();
Drop_A_Block(&cur_matrix, &is_done, &blocks[5], from_bottom);
//***************

	uc block_type = (rand() % 4) + 3;
	while (1) 
    {
		while(!is_done && !Game_Over())
		{
			Clear_All();
			Draw_Block_Matrix();
			Drop_A_Block(
				&cur_matrix, 
				&is_done, 
				&blocks[block_type],
				&from_bottom
			);
			
			while(!TimerFlag);
			TimerFlag = 0;
		}
		
		if (is_done && !Game_Over())
		{
			block_type = 4 + (rand() % 3);
			is_done = 0;
			cur_matrix = 0;
			
			blocks[block_type].left  = blocks[block_type].init_left;
			blocks[block_type].right = blocks[block_type].init_right;
			blocks[block_type].orientation = Vertical_Up;
			from_bottom = from_bottom2;
			
			Solid_Row_Eliminator();
			Draw_Block_Matrix();
		}
		else
		{
			Clear_All();
		}
    }

}