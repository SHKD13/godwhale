#include "precomp.h"
#include "stdinc.h"
#include "move.h"
#include "position.h"

namespace godwhale {

/* promote = 8, empty = 0,
 * pawn, lance, knight, silver, gold, bishop, rook, king, pro_pawn,
 * pro_lance, pro_knight, pro_silver, piece_null, horse, dragon };
 */
static char SfenPieceTable[] =
{
    '?', // empty = 0
    'P', // pawn = 1
    'L', // lance = 2
    'N', // knight = 3
    'S', // silver = 4
    'G', // gold = 5
    'B', // bishop = 6
    'R', // rook = 7
    'K', // king = 8
};

/**
 * @brief SFEN�`���̋����̃C���f�b�N�X�ɕϊ����܂��B
 *
 * ���̋ʁFK�A���̋ʁFk �iKing�̓������j
 * ���̔�ԁFR�A���̔�ԁFr �iRook�̓������j
 * ���̊p�FB�A���̊p�Fb �iBishop�̓������j
 * ���̋��FG�A���̋��Fg �iGold�̓������j
 * ���̋�FS�A���̋�Fs �iSilver�̓������j
 * ���̌j�n�FN�A���̌j�n�Fn �ikNight���j
 * ���̍��ԁFL�A���̍��ԁFl �iLance�̓������j
 * ���̕��FP�A���̕��Fp �iPawn�̓������j
 */
piece_t sfenToPiece(char piece)
{
    int upperPiece = toupper(piece);

    if (isupper(piece)) {
        for (int i = 0; i < ArraySize(SfenPieceTable); ++i) {
            if (piece == SfenPieceTable[i]) {
                return (piece_t)i;
            }
        }
    }
    else {
        piece = toupper(piece);

        for (int i = 0; i < ArraySize(SfenPieceTable); ++i) {
            if (piece == SfenPieceTable[i]) {
                return -(piece_t)i;
            }
        }
    }

    return (piece_t)0;
}

std::string moveToSfen(Move const & move)
{
    char toFileChr = (char)((int)'1' + (move.getToFile() - 1));
    char toRankChr = (char)((int)'a' + (move.getToRank() - 1));
    char buf[32];

    if (move.isDrop())
    {
        // ��ł��̏ꍇ
        int ipiece = move.getDrop();
        assert(0 <= ipiece && ipiece < ArraySize(SfenPieceTable));

        snprintf(buf, sizeof(buf), "%s*%c%c",
                 SfenPieceTable[ipiece],
                 toFileChr, toRankChr);
    }
    else
    {
        // ��̈ړ��̏ꍇ
        int fromFileChr = (char)((int)'1' + (move.getFromFile() - 1));
        int fromRankChr = (char)((int)'a' + (move.getFromRank() - 1));

        snprintf(buf, sizeof(buf), "%c%c%c%c%s",
                 fromFileChr, fromRankChr, toFileChr, toRankChr,
                 (move.isPromote() ? "+" : ""));
    }

    return buf;
}

Move sfenToMove(Position const & position, std::string const & sfen)
{
    if (sfen.empty()) {
        throw std::runtime_error("sfen������������܂���B");
    }

    int dropPiece = sfenToPiece(sfen[0]);
    if (dropPiece != 0)
    {
        // ��ł��̏ꍇ
        if ((sfen[1] != '*') ||
            (sfen[2] < '1' || '9' < sfen[2]) ||
            (sfen[3] < 'a' || 'i' < sfen[3]) )
        {
            return Move();
        }

        int toFile = (sfen[2] - '1') + 1;
        int toRank = (sfen[3] - 'a') + 1;
        int to = makeSquare(toFile, toRank);

        return Move::createDrop(to, dropPiece);
    }
    else
    {
        // ��̈ړ��̏ꍇ
        if ((sfen[0] < '1' || '9' < sfen[0]) ||
            (sfen[2] < '1' || '9' < sfen[2]) ||
            (sfen[1] < 'a' || 'i' < sfen[1]) ||
            (sfen[3] < 'a' || 'i' < sfen[3]))
        {
            return Move();
        }

        int fromFile = (sfen[0] - '1') + 1;
        int fromRank = (sfen[1] - 'a') + 1;
        int from = makeSquare(fromFile, fromRank);

        int toFile = (sfen[2] - '1') + 1;
        int toRank = (sfen[3] - 'a') + 1;
        int to = makeSquare(toFile, toRank);

        int piece = position[from];
        if (piece == 0)
        {
            return Move();
        }

        int capture = abs(position[to]);
        bool promote = (sfen.length() > 4 && sfen[4] == '+');
        return Move::create(from, to, piece, capture, promote);
    }
}

/*std::string PositionToSfen(Position const & position);
Position SfenToPosition(std::string const & sfen);*/

} // namespace godwhale
