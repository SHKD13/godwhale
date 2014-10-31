#include "precomp.h"
#include "stdinc.h"
#include "exceptions.h"
#include "io_sfen.h"

namespace godwhale {
namespace server {

typedef int piece_t;

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
 * @brief �����łȂ���̃C���f�b�N�X��SFEN�`���ɕϊ����܂��B
 */
static std::string nonpromotedPieceToSfen(piece_t piece)
{
    assert(-king <= piece && piece <= king);
    char ch;

    // +�Ȃ���A-�Ȃ���ł��B
    if (piece > 0) {
        ch = SfenPieceTable[piece];
    }
    else {
        ch = tolower(SfenPieceTable[-piece]);
    }

    return std::string(1, ch);
}

/**
 * @brief ��̃C���f�b�N�X��SFEN�`���ɕϊ����܂��B
 */
static std::string pieceToSfen(piece_t piece)
{
    assert(-dragon <= piece && piece <= dragon);
    piece_t apiece = abs(piece);
    piece_t simple = apiece;
    std::string result;

    if (apiece != king && apiece & promote) {
        simple = apiece & ~promote;
        result += "+";
    }

    // +�Ȃ���A-�Ȃ���ł��B
    if (piece > 0) {
        result += SfenPieceTable[simple];
    }
    else {
        result += tolower(SfenPieceTable[simple]);
    }

    return result;
}

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
static piece_t sfenToPiece(char pieceChr)
{
    int upperPiece, sign;

    // �啶���Ȃ���A�������Ȃ���ł��B
    if (isupper(pieceChr)) {
        upperPiece = pieceChr;
        sign = +1;
    }
    else {
        upperPiece = toupper(pieceChr);
        sign = -1;
    }

    for (int i = 0; i < ArraySize(SfenPieceTable); ++i) {
        if (upperPiece == SfenPieceTable[i]) {
            return (piece_t)(i * sign);
        }
    }

    return (piece_t)0;
}


/////////////////////////////////////////////////////////////////////
#pragma region �w����
/**
 * @brief �w�����SFEN�`���̕�����ɕϊ����܂��B
 */
std::string moveToSfen(Move const & move)
{
    char toFileChr = (char)((int)'1' + (move.GetToFile() - 1));
    char toRankChr = (char)((int)'a' + (move.GetToRank() - 1));
    char buf[32];

    if (move.IsDrop())
    {
        // ��ł��̏ꍇ
        int ipiece = move.GetDrop();
        assert(0 <= ipiece && ipiece < ArraySize(SfenPieceTable));

        snprintf(buf, sizeof(buf), "%c*%c%c",
                 SfenPieceTable[ipiece],
                 toFileChr, toRankChr);
    }
    else
    {
        // ��̈ړ��̏ꍇ
        int fromFileChr = (char)((int)'1' + (move.GetFromFile() - 1));
        int fromRankChr = (char)((int)'a' + (move.GetFromRank() - 1));

        snprintf(buf, sizeof(buf), "%c%c%c%c%s",
                 fromFileChr, fromRankChr, toFileChr, toRankChr,
                 (move.IsPromote() ? "+" : ""));
    }

    return buf;
}

/**
 * @brief SFEN�`���̕�������w����ɕϊ����܂��B
 */
Move sfenToMove(Board const & position, std::string const & sfen)
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
        int to = SQ(toFile, toRank);

        return Move::MakeDrop(to, dropPiece);
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
        int from = SQ(fromFile, fromRank);

        int toFile = (sfen[2] - '1') + 1;
        int toRank = (sfen[3] - 'a') + 1;
        int to = SQ(toFile, toRank);

        int piece = position[from];
        if (piece == 0)
        {
            return Move();
        }

        int capture = abs(position[to]);
        bool promote = (sfen.length() > 4 && sfen[4] == '+');
        return Move::Make(from, to, abs(piece), capture, promote);
    }
}
#pragma endregion


/////////////////////////////////////////////////////////////////////
#pragma region �ǖʂ𕶎���ɕϊ�
/**
 * @brief ��Ԃ�SFEN�`���ɕϊ����܂��B
 */
static std::string turnToSfen(int turn)
{
    if (turn != black && turn != white) {
        throw SfenException(
            "�ǖʂ̎�Ԃ�����������܂���B");
    }

    return (turn == black ? "b" : "w");
}

/**
 * @brief �e�i�̋����SFEN�`���ŕԂ��܂��B
 */
static std::string rankToSfen(Board const & position, int rank)
{
    std::string result;
    int nspaces = 0;

    for (int file = nfile; file >= 1; --file) {
        piece_t piece = position[SQ(file, rank)];
        if (piece == 0) {
            // ��Ȃ��ꍇ
            nspaces += 1;
        }
        else {
            // �����ꍇ
            if (nspaces > 0) {
                result += toString(nspaces);
                nspaces = 0;
            }

            result += pieceToSfen(piece);
        }
    }

    // �󔒂̐��͐����Ŏ����܂��B
    if (nspaces > 0) {
        result += toString(nspaces);
    }

    return result;
}

/**
 * @brief �ǖʂ�SFEN�`���ɕϊ����܂��B
 */
static std::string position0ToSfen(Board const & position)
{
    std::vector<std::string> v;

    for (int rank = 1; rank <= nrank; ++rank) {
        v.push_back(rankToSfen(position, rank));
    }

    return boost::join(v, "/");
}

/**
 * @brief ��Ԃ��Ƃ̎������SFEN�`���ɕϊ����܂��B
 */
static std::string hand0ToSfen(Board const & position, int turn)
{
    std::string result;

    for (piece_t piece = rook; piece >= pawn; --piece) {
        int count = position.GetHand(turn, piece);

        if (count > 1) {
            result += toString(count);
        }

        if (count > 0) {
            result += nonpromotedPieceToSfen(piece * (turn == black ? +1 : -1));
        }
    }

    return result;
}

/**
 * @brief �������SFEN�`���ɕϊ����܂��B
 */
static std::string handToSfen(Board const & position)
{
    std::string result;
    result += hand0ToSfen(position, black);
    result += hand0ToSfen(position, white);

    return (!result.empty() ? result : "-");
}

/**
 * @brief �ǖʂ�SFEN�`���̕�����ɂɕϊ����܂��B
 */
std::string positionToSfen(Board const & position)
{
    return (boost::format("%1% %2% %3% 1")
        % position0ToSfen(position)
        % turnToSfen(position.GetTurn())
        % handToSfen(position))
        .str();
}
#pragma endregion


/////////////////////////////////////////////////////////////////////
#pragma region ��������ǖʂɕϊ�
/**
 * @brief �ǖʂ̎�Ԃ��p�[�X���܂��B
 */
static int parseTurn(std::string const & text)
{
    if (text.length() != 1 || (text[0] != 'b' && text[0] != 'w'))
    {
        throw std::invalid_argument(
            "SFEN�`���̎�ԕ\��������������܂���B");
    }

    return (text[0] == 'b' ? black : white);
}

/**
 * @brief SFEN�`���̋ǖʕ����݂̂��p�[�X���܂��B
 */
static void parseBoard0(Board & position, std::string const & sfen)
{
    int rank = 1;
    int file = 9;
    int promoted = false;

    BOOST_FOREACH(auto c, sfen) {
        if (rank > 9) {
            throw SfenException(
                "�ǖʂ̒i�����X�𒴂��܂��B");
        }

        if (c == '/') {
            if (file != 0) {
                throw SfenException(
                    "SFEN�`����%d�i�̋�������܂���B", rank);
            }

            rank += 1;
            file = 9;
            promoted = false;
        }
        else if (c == '+') {
            promoted = true;
        }
        else if ('1' <= c && c <= '9') {
            file -= (c - '0');
            promoted = false;
        }
        else {
            if (file < 1) {
                throw SfenException(
                    "SFEN�`����%d�i�̋���������܂��B", rank);
            }

            int piece = sfenToPiece(c);
            if (piece == 0) {
                throw SfenException(
                    "SFEN�`���̋�'%c'������������܂���B", c);
            }

            if (promoted && (piece == gold || piece == king)) {
                throw SfenException(
                    "����Ȃ���𐬂��Ă��܂��Ă��܂��B");
            }

            if (promoted) {
                piece |= promote;
            }

            position.Set(SQ(file, rank), piece);
            file -= 1;
            promoted = false;
        }
    }

    if (file != 0) {
        throw SfenException(
            "SFEN�`����%d�i�̋�������܂���B", rank);
    }
}

/**
 * @brief �������ǂݍ��݂܂��B
 */
static void parseHand(Board & position, std::string const & sfen)
{
    if (sfen[0] == '-') {
        // ��������K�v������܂���B
        return;
    }

    int count = 0;
    BOOST_FOREACH (auto c, sfen) {
        if ('1' <= c && c <= '9') {
            count = count * 10 + (c - '0');
        }
        else {
            int piece = sfenToPiece(c);
            if (piece == 0) {
                throw SfenException(
                    "SFEN�`���̎�����'%c'������������܂���B", c);
            }

            int count1 = std::max(1, count);
            position.SetHand((piece > 0 ? black : white), abs(piece), count1);
            count = 0;
        }
    }
}

/**
 * @brief SFEN�`���̋ǖʂ����ۂ̋ǖʂɕϊ����܂��B
 */
Board sfenToPosition(std::string const & sfen)
{
    if (sfen.empty()) {
        throw std::invalid_argument("sfen");
    }

    std::vector<std::string> result;
    boost::split(result, sfen, boost::is_any_of(" "));
    result.erase(std::remove(result.begin(), result.end(), ""), result.end());
    if (result.size() < 3) {
        throw SfenException(
            sfen + ": SFEN�`���̔Օ\��������������܂���B");
    }

    Board position(false);
    parseBoard0(position, result[0]);
    parseHand(position, result[2]);
    position.SetTurn(parseTurn(result[1]));
    return position;
}
#pragma endregion

} // namespace server
} // namespace godwhale
