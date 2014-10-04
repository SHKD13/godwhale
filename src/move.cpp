#include "precomp.h"
#include "stdinc.h"
#include "move.h"

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
 * @brief �w����𕪂���₷���悤�ɕ����񉻂��܂��B
 */
std::string toString(move_t move, int turn/*=turn_none*/)
{
    char str[32];

    const char *turnStr = TurnStrTable[turn];
    int is_promote  = (int)I2IsPromote(move);
    int ipiece_move = (int)I2PieceMove(move);
    int ifrom       = (int)I2From(move);
    int ito         = (int)I2To(move);

    if (is_promote) {
        // ��) 87�␬(86)
        snprintf(str, sizeof(str), "%s%d%d%s��(%d%d)",
                 turnStr,
                 9-aifile[ito],   airank[ito]  +1,
                 PieceStrTable[ipiece_move + promote],
                 9-aifile[ifrom], airank[ifrom]+1);
    }
    else if (ifrom < nsquare) {
        // ��) 87��(67)
        snprintf(str, sizeof(str), "%s%d%d%s(%d%d)",
                 turnStr,
                 9-aifile[ifrom], airank[ifrom]+1,
                 PieceStrTable[ipiece_move],
                 9-aifile[ito],   airank[ito]  +1);
    }
    else {
        // ��) 99���
        snprintf(str, sizeof(str), "%s%d%d%s��",
                 turnStr,
                 9-aifile[ito], airank[ito]+1,
                 PieceStrTable[From2Drop(ifrom)]);
    }

    return str;
}

} // namespace godwhale
