
/* bonanza������Q�Ƃ���֐�/�ϐ��͂��ׂ� extern "C" �Ő錾���܂��B*/
#include "bonanza6/shogi.h"

#ifdef __cplusplus
extern "C" {
#endif

// for search()
//extern int detectSignalSlave();

// for server
#ifdef GODWHALE_SERVER
extern void CONV init_game_hook(const min_posi_t *posi);
extern void CONV make_move_root_hook(move_t move);
extern void CONV unmake_move_root_hook();
extern int CONV server_iterate(int *value, move_t *pvseq, int *pvseq_length);
#endif

#ifdef __cplusplus
}
#endif
