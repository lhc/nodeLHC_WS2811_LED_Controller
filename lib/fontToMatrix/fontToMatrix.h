#ifndef _FONT_TO_MATRIX_H_
#define _FONT_TO_MATRIX_H_

#include <stdint.h>

#define SPACE_BETWEEN_CHAR_IN_PIXELS 0

#define SIZE_FONT 8


int strToFrame(
            char *str,          //!< [in] Strint to be converted
            uint32_t *state,    //!< [in] State variable
            uint8_t *buf_out    //!< [out] Frame designed. Not sized check
              );


#endif // _FONT_TO_MATRIX_H_