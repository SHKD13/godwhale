#include "precomp.h"
#include "stdinc.h"
#include "board.h"

namespace godwhale {
namespace server {

Board::Board(bool init/*= true*/)
    : m_handBlack(0), m_handWhite(0), m_turn(black)
{
    if (init) {
        // m_asquare��min_posi_t.asquare�͌^���Ⴄ�B
        std::copy_n(min_posi_no_handicap.asquare, (int)nsquare, m_asquare);
    }
    else {
        memset(m_asquare, 0, sizeof(m_asquare));
    }
}

Board::Board(const Board &other)
{
    ScopedLock lock(other.m_guard);

    m_handBlack = other.m_handBlack;
    m_handWhite = other.m_handWhite;
    m_turn = other.m_turn;
    m_moveList = other.m_moveList;
    std::copy_n(other.m_asquare, (int)nsquare, m_asquare);
}

Board::Board(const min_posi_t &posi)
    : m_handBlack(posi.hand_black), m_handWhite(posi.hand_white)
    , m_turn(posi.hand_white)
{
    std::copy_n(posi.asquare, (int)nsquare, m_asquare);
}

Board &Board::operator =(const Board &other)
{
    ScopedLock lock(other.m_guard);

    m_handBlack = other.m_handBlack;
    m_handWhite = other.m_handWhite;
    m_turn = other.m_turn;
    m_moveList = other.m_moveList;
    std::copy_n(other.m_asquare, (int)nsquare, m_asquare);
    return *this;
}

Board &Board::operator =(const min_posi_t &posi)
{
    m_handBlack = posi.hand_black;
    m_handWhite = posi.hand_white;
    m_turn = posi.hand_white;
    std::copy_n(posi.asquare, (int)nsquare, m_asquare);

    m_moveList.clear();
    return *this;
}

bool operator==(Board const & lhs, Board const & rhs)
{
    if (lhs.m_handBlack != rhs.m_handBlack ||
        lhs.m_handWhite != rhs.m_handWhite ||
        lhs.m_turn != rhs.m_turn) {
        return false;
    }

    if (!std::equal(boost::begin(lhs.m_asquare), boost::end(lhs.m_asquare),
                    rhs.m_asquare)) {
        return false;
    }

    return true;
}

/**
 * @brief min_posi_t�I�u�W�F�N�g�ɕϊ����܂��B
 */
min_posi_t Board::getMinPosi() const
{
    min_posi_t posi;

    posi.hand_black = m_handBlack;
    posi.hand_white = m_handWhite;
    posi.turn_to_move = m_turn;
    std::copy_n(m_asquare, (int)nsquare, posi.asquare);
    return posi;
}

int Board::GetHand(int turn, int piece) const
{
    assert(turn == black || turn == white);
    assert(piece > 0);
    int hand = (turn == black ? m_handBlack : m_handWhite);

    switch (piece) {
    case pawn:   return I2HandPawn(hand);
    case knight: return I2HandKnight(hand);
    case lance:  return I2HandLance(hand);
    case silver: return I2HandSilver(hand);
    case gold:   return I2HandGold(hand);
    case bishop: return I2HandBishop(hand);
    case rook:   return I2HandRook(hand);
    default:     unreachable(); break;
    }
}

void Board::SetHand(int turn, int piece, int count)
{
    assert(turn == black || turn == white);
    assert(piece > 0 && count >= 0);
    int hand = (turn == black ? m_handBlack : m_handWhite);

    switch (piece) {
    case pawn:   hand&=~IsHandPawn(~0);   hand|=(flag_hand_pawn  *count); break;
    case knight: hand&=~IsHandKnight(~0); hand|=(flag_hand_knight*count); break;
    case lance:  hand&=~IsHandLance(~0);  hand|=(flag_hand_lance *count); break;
    case silver: hand&=~IsHandSilver(~0); hand|=(flag_hand_silver*count); break;
    case gold:   hand&=~IsHandGold(~0);   hand|=(flag_hand_gold  *count); break;
    case bishop: hand&=~IsHandBishop(~0); hand|=(flag_hand_bishop*count); break;
    case rook:   hand&=~IsHandRook(~0);   hand|=(flag_hand_rook  *count); break;
    default:     unreachable(); break;
    }

    (turn == black ? m_handBlack : m_handWhite) = hand;
}

/**
 * @brief �w���肪�������w���肩�ǂ������ׂ܂��B
 */
bool Board::IsValidMove(Move move) const
{
    ScopedLock lock(m_guard);
    const int from = move.GetFrom();
    const int to   = move.GetTo();

    if (move.IsEmpty()) {
        return false;
    }

    if (move.IsDrop()) {
        // ��ł��̏ꍇ
        if (Get(to) != 0) {
            return false;
        }

        unsigned int u = GetCurrentHand();
        switch (move.GetDrop()) {
        case pawn:    if (IsHandPawn(u))   { return true; } break;
        case lance:   if (IsHandLance(u))  { return true; } break;
        case knight:  if (IsHandKnight(u)) { return true; } break;
        case silver:  if (IsHandSilver(u)) { return true; } break;
        case gold:    if (IsHandGold(u))   { return true; } break;
        case bishop:  if (IsHandBishop(u)) { return true; } break;
        case rook:    if (IsHandRook(u))   { return true; } break;
        default:  unreachable(); break;
        }

        return false;
    }
    else {
        // ��̈ړ��̏ꍇ
        int piece_move = move.GetPiece();
        int piece_cap  = move.GetCapture();

        if (m_turn == black) {
            if (Get(from) !=  piece_move) return false;
            if (Get(to)   != -piece_cap)  return false;
        }
        else {
            if (Get(from) != -piece_move) return false;
            if (Get(to)   !=  piece_cap)  return false;
        }

        return true;
    }
}

int Board::DoMove(Move move)
{
    ScopedLock lock(m_guard);
    const int sign = (m_turn == black ? +1 : -1);

    if (move.GetCapture() == king) {
        LOG(Error) << move << " " << move.GetCapture() << " is invalid.";
        return -1;
    }
    if (move == MOVE_NA || !IsValidMove(move)) {
        return -1;
    }

    if (move.IsDrop()) {
        // ��ł��̏ꍇ
        assert(abs(Get(move.GetTo())) == 0);

        const int ipiece_drop = move.GetDrop();
        
        SubCurrentHand(ipiece_drop);
        Set(move.GetTo(), ipiece_drop * sign);
    }
    else {
        // ��̈ړ��̏ꍇ
        assert(abs(Get(move.GetFrom())) == move.GetPiece());
        assert(abs(Get(move.GetTo())) == move.GetCapture());

        Set(move.GetFrom(), 0);

        // promote=8 �ł���ȏ�̒l�Ȃ琬���ƂȂ�܂��B
        int ipiece_move = move.GetPiece();
        if (move.IsPromote()) {
            ipiece_move |= promote;
        }
        Set(move.GetTo(), ipiece_move * sign);

        const int ipiece_cap = move.GetCapture();
        if (ipiece_cap != 0) {
            AddCurrentHand(ipiece_cap);
        }
    }

    m_turn = Flip(m_turn);
    m_moveList.push_back(move);
    return 0;
}

int Board::DoUnmove()
{
    ScopedLock lock(m_guard);
    if (m_moveList.empty()) {
        return -1;
    }

    // ��Ԃ͐�Ƀt���b�v���܂��B
    m_turn = Flip(m_turn);

    Move move = m_moveList.back();
    if (move.IsDrop()) {
        // ��ł��̏ꍇ
        assert(abs(Get(move.GetTo())) == move.GetDrop());

        AddCurrentHand(move.GetDrop());
        Set(move.GetTo(), 0);
    }
    else {
        // ��̈ړ��̏ꍇ
        assert(abs(Get(move.GetFrom())) == 0);
        assert(abs(Get(move.GetTo())) ==
               (move.GetPiece() | (move.IsPromote() ? promote : 0)));

        const int sign = (m_turn == black ? +1 : -1);

        // piece �ɂ͐������ꍇ�͐���O�̋�����Ă��܂��B
        Set(move.GetFrom(), move.GetPiece() * sign);

        int ipiece_cap = move.GetCapture();
        if (ipiece_cap != 0) {
            SubCurrentHand(ipiece_cap);
        }

        Set(move.GetTo(), -ipiece_cap * sign);
    }

    m_moveList.pop_back();
    return 0;
}

/**
 * @brief ���ݎ�Ԃ̎�������擾���܂��B
 */
unsigned int Board::GetCurrentHand() const {
    return (m_turn == black ? m_handBlack : m_handWhite);
}

/**
 * @brief ���ݎ�Ԃ̎�������擾���܂��B
 */
void Board::AddCurrentHand(int capture) {
    assert(0 < capture && capture < ArraySize(HandTable));

    unsigned int &hand = (m_turn == black ? m_handBlack : m_handWhite);
    hand += HandTable[capture];
}

/**
 * @brief ���ݎ�Ԃ̎�������擾���܂��B
 */
void Board::SubCurrentHand(int capture) {
    assert(0 < capture && capture < ArraySize(HandTable));

    unsigned int &hand = (m_turn == black ? m_handBlack : m_handWhite);
    hand -= HandTable[capture];
}

/**
 * @brief CSA�`�������̎�ނ𔻕ʂ��܂��B
 */
int Board::StrToPiece(const std::string &str, std::string::size_type index) const
{
    int i;

    for (i = 0; i < ArraySize(astr_table_piece); ++i) {
        if (str.compare(index, 2, astr_table_piece[i]) == 0) break;
    }

    if (i == 0 || i == piece_null || i == ArraySize(astr_table_piece)) {
        i = -2;
    }

    return i;
}

/**
 * @brief CSA�`���̎w���������`���ɕϊ����܂��B
 */
Move Board::InterpretCsaMove(const std::string &str) const
{
    ScopedLock lock(m_guard);
    int ifrom_file, ifrom_rank, ito_file, ito_rank, ipiece;
    int ifrom, ito;
    bool is_promote;

    ifrom_file = str[0]-'0';
    ifrom_rank = str[1]-'0';
    ito_file   = str[2]-'0';
    ito_rank   = str[3]-'0';

    ito_file   = 9 - ito_file;
    ito_rank   = ito_rank - 1;
    ito        = ito_rank * 9 + ito_file;
    ipiece     = StrToPiece(str, 4);
    if (ipiece < 0) {
        return MOVE_NA;
    }

    if (ifrom_file == 0 && ifrom_rank == 0) {
        return Move::MakeDrop(ito, ipiece);
    }
    else {
        ifrom_file = 9 - ifrom_file;
        ifrom_rank = ifrom_rank - 1;
        ifrom      = ifrom_rank * 9 + ifrom_file;
        if (abs(Get(ifrom)) + promote == ipiece) {
            ipiece    -= promote;
            is_promote = true;
        }
        else {
            is_promote = false;
        }

        return Move::Make(ifrom, ito, ipiece, abs(Get(ito)), is_promote);
    }
}

void Board::Print(std::ostream &os) const
{
    ScopedLock lock(m_guard);
    const Move move      = (m_moveList.empty() ? MOVE_NA : m_moveList.back());
    const int ito        = move.GetTo();
    const int ifrom      = move.GetFrom();
    const int is_promote = move.IsPromote();

    os << "'    9   8   7   6   5   4   3   2   1" << std::endl;

    for (int irank = rank1; irank <= rank9; ++irank) {
        os << "P" << irank + 1 << " |";
        
        for (int ifile = file1; ifile <= file9; ++ifile) {
            int sq = irank * nfile + ifile;

            PrintPiece(os, Get(sq), sq, ito, ifrom, is_promote);
            os << "|";
        }

        os << std::endl;
    }

    PrintHand(os, m_handBlack, "P+");
    PrintHand(os, m_handWhite, "P-");
}

void Board::PrintPiece(std::ostream &os, int piece, int sq, int ito,
                       int ifrom, int is_promote) const
{
    if (piece != 0) {
        char ch = (piece < 0 ? '-' : '+');
        os << ch << astr_table_piece[abs(piece)];
    }
    else {
        os << " * ";
    }
}

void Board::PrintHand(std::ostream &os, unsigned int hand,
                      const std::string &prefix) const
{
    PrintHand(os, (int)I2HandPawn(hand),   prefix, "00FU");
    PrintHand(os, (int)I2HandLance(hand),  prefix, "00KY");
    PrintHand(os, (int)I2HandKnight(hand), prefix, "00KE");
    PrintHand(os, (int)I2HandSilver(hand), prefix, "00GI");
    PrintHand(os, (int)I2HandGold(hand),   prefix, "00KI");
    PrintHand(os, (int)I2HandBishop(hand), prefix, "00KA");
    PrintHand(os, (int)I2HandRook(hand),   prefix, "00HI");
}

void Board::PrintHand(std::ostream &os, int n, const std::string &prefix,
                      const std::string &str) const
{
    if (n > 0) {
        os << prefix;
        for (int i = 0; i < n; ++i) { os << str; }
        os << std::endl;
    }
}

}
}
