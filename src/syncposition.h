
#ifndef GODWHALE_SYNCPOSITION_H
#define GODWHALE_SYNCPOSITION_H

#include "move.h"
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
    void initialize(min_posi_t const & posi);

    Position getPosition() const;

    bool makeMoveRoot(Move move);
    void makeMove(Move move);
    void unmakeMove();

    void rewind();
    void resetFromRoot(const std::vector<Move> & pv);

    void extendPV();
    void getMoveList(Move exclude, bool firstMoveOnly, std::vector<Move> * result);

private:
    void initBonanza(tree_t * restrict ptree);

private:
    static shared_ptr<SyncPosition> ms_instance;

    Move m_moveList[128];
    bool m_checksList[128+2];
    int m_moveSize;
};

} // namespace godwhale

#endif
