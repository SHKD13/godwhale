
#ifndef GODWHALE_SERVER_MOVE_H
#define GODWHALE_SERVER_MOVE_H

namespace godwhale {
namespace server {

/**
 * @brief �w������������߂̃N���X�ł��B
 */
class Move
{
public:
    static Move Make(int from, int to, int piece, int capture, bool isPromote) {
        return Move(
            From2Move(from) | To2Move(to) | Piece2Move(piece) |
            Cap2Move(capture) | ((int)isPromote << 14));
    }

    static Move MakeDrop(int to, int piece) {
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
        return Get();
    }

    /**
     * @brief �w���肪�ݒ肳��Ă��邩�ǂ����𒲂ׂ܂��B
     */
    bool IsEmpty() const {
        return (m_move == MOVE_NA);
    }

    /**
     * @brief move_t�^�̒l���擾���܂��B
     */
    move_t Get() const {
        return m_move;
    }

    /**
     * @brief �w����̈ړ���i��ł��̏ꍇ�͑łꏊ�j�̃}�X���擾���܂��B
     */
    int GetTo() const {
        return I2To(m_move);
    }

    /**
     * @brief �w����̈ړ������擾���܂��B��ł��̏ꍇ�͖����ł��B
     */
    int GetFrom() const {
        return I2From(m_move);
    }

    /**
     * @brief �w����̈ړ����ƈړ�������킹�Ď擾���܂��B
     */
    int GetFromTo() const {
        return I2FromTo(m_move);
    }

    /**
     * @brief ��ł����ǂ����𒲂ׂ܂��B
     */
    bool IsDrop() const {
        return (GetFrom() >= nsquare);
    }

    /**
     * @brief ����邩�ǂ����𒲂ׂ܂��B
     */
    bool IsPromote() const {
        return (I2IsPromote(m_move) != 0);
    }

    /**
     * @brief ������������擾���܂��B
     */
    int GetPiece() const {
        assert(!IsDrop());
        return I2PieceMove(m_move);
    }

    /**
     * @brief �ł�������擾���܂��B
     */
    int GetDrop() const {
        assert(IsDrop());
        return From2Drop(GetFrom());
    }

    /**
     * @brief �������̎��(����s������܂�)���擾���܂��B
     */
    int GetCapture() const {
        return UToCap(m_move);
    }

    /**
     * @brief �w����𕪂���₷���悤�ɕ����񉻂��܂��B
     */
    std::string String() const {
        return std::move(ToString(m_move));
    }

private:
    move_t m_move;
};

inline std::ostream &operator<<(std::ostream &stream, Move move)
{
    stream << move.String();
    return stream;
}

} // namespace server
} // namespace godwhale

#endif
