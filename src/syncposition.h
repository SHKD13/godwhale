
#ifndef GODWHALE_SYNCPOSITION_H
#define GODWHALE_SYNCPOSITION_H

#include "position.h"

namespace godwhale {

/**
 * @brief bonanza�̓��e�Ɠ��������ǖʂ��Ǘ����܂��B
 */
class SyncPosition : private boost::noncopyable
{
private:
    explicit SyncPosition();

public:
    /**
     * @brief �V���O���g���C���X�^���X���擾���܂��B
     */
    static shared_ptr<SyncPosition> get()
    {
        return ms_instance;
    }

    /**
     * @brief �ǖ�ID���擾���܂��B
     */
    int getPositionId() const
    {
        return m_positionId;
    }

    /**
     * @brief �ǖ�ID���擾���܂��B
     */
    Position const &getRootPosition() const
    {
        return m_rootPosition;
    }

    /**
     * @brief �w��̋ǖʂŉ��肳��Ă��邩���ׂ܂��B
     */
    bool isChecked(int ply)
    {
        if (ply < 0 || m_moveSize < ply) {
            throw std::out_of_range("ply");
        }

        return m_checksList[ply];
    }

    void initialize();
#if 1 || defined(GODWHALE_CLIENT)
    void reset(int positionId, Position const & position);
#endif

    Position getPosition() const;

    bool makeMoveRoot(int positionId, Move move);

    void makeMove(Move move);
    void unmakeMove();

    void rewind();
    void resetFromRoot(const std::vector<Move> & pv);

    void extendPV();
    std::vector<Move> getMoveList(Move exclude, bool firstMoveOnly);

private:
    void initBonanza(tree_t * restrict ptree);

private:
    static shared_ptr<SyncPosition> ms_instance;

    int m_positionId;
    Position m_rootPosition;
    Move m_moveList[128];
    bool m_checksList[128+2];
    int m_moveSize;
};

} // namespace godwhale

#endif
