#ifndef FBL_MAIN_H
#define FBL_MAIN_H

/******************************** Global Types ***************************************/
//#define APP_VAILD_FLAG_ADDR     ((0x00080000UL-8U))
#define APP_VAILD_FLAG_ADDR     (0x20210000UL)
#define APP_VAILD_FLAG_VALUE    (0x5555AAAAUL)
#define APP_RESET_FLAG_ADDR     (0x20210000UL)
#define APP_RESET_FLAG_VALUE    (0x5555AAAAUL)

#define VECTOR_ADDRESS_BANK0    (6*1024)
#define APP_JUMP_ADDR           (VECTOR_ADDRESS_BANK0 + 4)
#define APP_RESET_COUNT_MAX     (0x5UL)

#define APP_VAILD_FLAG          (((*(uint32_t *)(APP_VAILD_FLAG_ADDR)) == (APP_VAILD_FLAG_VALUE)) ? true : false)
#define APP_RESET_FLAG          (((*(uint32_t *)(APP_RESET_FLAG_ADDR)) == (APP_RESET_FLAG_VALUE)) ? true : false)

#define JUMPTO(jmp_address)     {((void (*)())(*(uint32_t *)(jmp_address))) ();}
#define JUMP_TO_APP             JUMPTO(APP_JUMP_ADDR)

#define SET_APP_RESET_FLAG()    (*((uint32_t*)APP_RESET_FLAG_ADDR) = APP_RESET_FLAG_VALUE)
#define CLEAR_APP_RESET_FLAG()  (*((uint32_t*)APP_RESET_FLAG_ADDR) = 0x0)

#define BANK_N                  (0U)
#define BANK_A                  ('a')
#define BANK_B                  ('b')

extern void FblMain(void);

#endif
