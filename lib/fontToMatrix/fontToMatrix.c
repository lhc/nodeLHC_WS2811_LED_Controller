#include <stdlib.h>
#include "fontToMatrix.h"
#include "font8x8.h"
#include <string.h>



int strToFrame(
            char *str,          //!< [in] Strint to be converted
            uint32_t *state,    //!< [in] State variable
            uint8_t *buf_out    //!< [out] Frame designed. Not sized check
              ){

    //__ Define the size of the square buffer
    #define SIDE_LEN 8
    #define COLOR_SIZE 3



    //__ Always set the buff with nothing 
    memset(buf_out, 0x00, SIDE_LEN*SIDE_LEN*COLOR_SIZE);

    //__ each font caracter has 8x8 pixels
    // so, each font caracter has at least 8 state, untill
    // another caracters appears. 
    if (*state < SIDE_LEN + SPACE_BETWEEN_CHAR_IN_PIXELS){
        *state += 1;
        return 0;
    }

    uint32_t pos_str = (*state / (8 + SPACE_BETWEEN_CHAR_IN_PIXELS)) - 1;
    char prev;
    if (pos_str == 0)
        prev = ' ';
    else
        prev = str[pos_str - 1];

    char actual;
    if (pos_str < strlen(str))
        actual = str[pos_str];
    else if (pos_str == strlen(str))
        actual = ' ';
    else {
        *state = 0;
        return 0;
    }

    //__ Actual position (column) of the str caracter index
    uint32_t pos_actual_char = *state % (8 + SPACE_BETWEEN_CHAR_IN_PIXELS);
        
    for (uint8_t line = 0; line < 8; line++){

        uint8_t left_of_prev_char = left_of_prev_char = SIDE_LEN + SPACE_BETWEEN_CHAR_IN_PIXELS - (pos_actual_char + 1);
        
        //__ Total of pixel of the font (8 + space)
        // Then when the pos_actual_char is 0, the firts column is apearing. So (pos_actual_char + 1)
        for (uint8_t col = 0; col < 8; col++){
            if ( (col < left_of_prev_char)    &&
                 (font8x8[prev][line] >> ((SIZE_FONT + SPACE_BETWEEN_CHAR_IN_PIXELS) - left_of_prev_char + col)) & 1 == 1 ){
                buf_out[line*COLOR_SIZE*SIDE_LEN + 3*col    ] = 0xff;
                buf_out[line*COLOR_SIZE*SIDE_LEN + 3*col + 1] = 0xff;
                buf_out[line*COLOR_SIZE*SIDE_LEN + 3*col + 2] = 0xff;
            }

            if ( (col >= left_of_prev_char)    &&
                 (font8x8[actual][line] >> (col - left_of_prev_char)) & 1 == 1 ){
                buf_out[line*COLOR_SIZE*SIDE_LEN + 3*col    ] = 0xff;
                buf_out[line*COLOR_SIZE*SIDE_LEN + 3*col + 1] = 0xff;
                buf_out[line*COLOR_SIZE*SIDE_LEN + 3*col + 2] = 0xff;
            }
        }

        /* DEBUG TEST
        //@@@@@
        char test_str[100];
        sprintf(test_str, "f:0x%02X l:%02d  ", font8x8[prev][line], left_of_prev_char); printf(test_str);
        for(uint8_t t = 0; t < 8*3; t += 3){
            if (buf_out[line*COLOR_SIZE*SIDE_LEN + t] == 0xff){
                sprintf(test_str, "X"); printf(test_str);
            } else {
                sprintf(test_str, "."); printf(test_str);
            }
        }
        sprintf(test_str, "\n\r"); printf(test_str);*/
    }

    *state += 1;

    return 0;

}
