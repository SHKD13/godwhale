
#ifndef GODWHALE_MOVE_H
#define GODWHALE_MOVE_H

namespace godwhale {

/**
 * @brief bonanza�̎w����𕶎���ɕϊ����܂��B
 */
extern std::string toString(move_t move, int turn = turn_none);

inline int SQ(int file, int rank)
{
    return ((rank-1) * 9 + (9-file));
}

/**
 * @brief �w������������߂̃N���X�ł��B
 */
class Move
{
public:
    /// ����ړ����邽�߂̎w������쐬���܂��B
    static Move create(int from, int to, int piece, int capture, bool isPromote) {
        return Move(
            From2Move(from) | To2Move(to) | Piece2Move(piece) |
            Cap2Move(capture) | ((int)isPromote << 14));
    }

    /// ���ł��߂̎w������쐬���܂��B
    static Move createDrop(int to, int piece) {
        return Move(To2Move(to) | Drop2Move(piece));
    }

    Move() : m_move(MOVE_NA) {
    }

    Move(move_t move) : m_move(move) {
    }

    Move(const Move &move) : m_move(move.m_move) {
    }

    Move &operator =(const Move &move) {
        m_move = move.m_move;
        return *this;
    }

    Move &operator =(move_t move) {
        m_move = move;
        return *this;
    }

    /**
     * @brief move_t�^�ւ̃L���X�g�p�I�y���[�^
     */
    operator move_t() const {
        return get();
    }

    /**
     * @brief �w���肪�ݒ肳��Ă��邩�ǂ����𒲂ׂ܂��B
     */
    bool isEmpty() const {
        return (m_move == MOVE_NA);
    }

    /**
     * @brief move_t�^�̒l���擾���܂��B
     */
    move_t get() const {
        return m_move;
    }

    /**
     * @brief �w����̈ړ���i��ł��̏ꍇ�͑łꏊ�j�̃}�X���擾���܂��B
     */
    int getTo() const {
        return I2To(m_move);
    }

    /**
     * @brief �w����̈ړ���i��ł��̏ꍇ�͑łꏊ�j�̋؂��擾���܂��B
     */
    int getToFile() const {
        return (9 - (getTo() % 9));
    }

    /**
     * @brief �w����̈ړ���i��ł��̏ꍇ�͑łꏊ�j�̒i���擾���܂��B
     */
    int getToRank() const {
        return ((getTo() / 9) + 1);
    }

    /**
     * @brief �w����̈ړ������擾���܂��B��ł��̏ꍇ�͖����ł��B
     */
    int getFrom() const {
        return I2From(m_move);
    }

    /**
     * @brief �w����̈ړ����̋؂��擾���܂��B
     */
    int getFromFile() const {
        return (9 - (getFrom() % 9));
    }

    /**
     * @brief �w����̈ړ����̒i���擾���܂��B
     */
    int getFromRank() const {
        return ((getFrom() / 9) + 1);
    }

    /**
     * @brief �w����̈ړ����ƈړ�������킹�Ď擾���܂��B
     */
    int getFromTo() const {
        return I2FromTo(m_move);
    }

    /**
     * @brief ��ł����ǂ����𒲂ׂ܂��B
     */
    bool isDrop() const {
        return (getFrom() >= nsquare);
    }

    /**
     * @brief ����邩�ǂ����𒲂ׂ܂��B
     */
    bool isPromote() const {
        return (I2IsPromote(m_move) != 0);
    }

    /**
     * @brief ������������擾���܂��B
     */
    int getPiece() const {
        assert(!isDrop());
        return I2PieceMove(m_move);
    }

    /**
     * @brief �ł�������擾���܂��B
     */
    int getDrop() const {
        assert(isDrop());
        return From2Drop(getFrom());
    }

    /**
     * @brief �������̎��(����s������܂�)���擾���܂��B
     */
    int getCapture() const {
        return UToCap(m_move);
    }

    /**
     * @brief �w����𕪂���₷���悤�ɕ����񉻂��܂��B
     */
    std::string str(int turn = turn_none) const {
        return std::move(toString(m_move, turn));
    }

private:
    move_t m_move;
};

inline std::ostream &operator<<(std::ostream &stream, Move move)
{
    stream << move.str();
    return stream;
}

} // namespace godwhale

#endif
