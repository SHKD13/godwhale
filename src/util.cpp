
#include "precomp.h"
#include "stdinc.h"
#include "util.h"

namespace godwhale {

const char *PieceStrTable[16]  = {
    "�Z", "��", "��", "�j", "��", "��",
    "�p", "��", "��", "�g", "��", "NK",
    "NG", "�~", "�n", "��"
};

const char *TurnStrTable[] =
{
    "��", "��", ""
};

/**
 * @brief �����Ă���r�b�g�̐��𒲂ׂ܂��B
 */ 
int popCount(int x)
{
    x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x & 0x0f0f0f0f) + ((x >> 4) & 0x0f0f0f0f);
    x = (x & 0x00ff00ff) + ((x >> 8) & 0x00ff00ff);
    return ((x & 0x0000ffff) + ((x >> 16) & 0x0000ffff));
}

/**
 * @brief �w��̇_�b�����X���[�v���܂��B
 */ 
void sleep(int milliseconds)
{
    boost::this_thread::sleep(boost::posix_time::milliseconds(milliseconds));
}

/**
 * @brief �w����𕪂���₷���悤�ɕ����񉻂��܂��B
 */
std::string ToString(move_t move)
{
    int ifrom, ito, ipiece_move, is_promote;
    char str[7];

    is_promote  = (int)I2IsPromote(move);
    ipiece_move = (int)I2PieceMove(move);
    ifrom       = (int)I2From(move);
    ito         = (int)I2To(move);
  
    if (is_promote) {
        snprintf(str, 7, "%d%d%d%d%s",
                 9-aifile[ifrom], airank[ifrom]+1,
                 9-aifile[ito],   airank[ito]  +1,
                 astr_table_piece[ipiece_move + promote]);
    }
    else if (ifrom < nsquare) {
        snprintf(str, 7, "%d%d%d%d%s",
                 9-aifile[ifrom], airank[ifrom]+1,
                 9-aifile[ito],   airank[ito]  +1,
                 astr_table_piece[ipiece_move]);
    }
    else {
        snprintf(str, 7, "00%d%d%s",
                 9-aifile[ito], airank[ito]+1,
                 astr_table_piece[From2Drop(ifrom)]);
    }

    return str;
}

} // namespace godwhale
