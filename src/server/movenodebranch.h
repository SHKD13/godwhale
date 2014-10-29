
#ifndef GODWHALE_SERVER_MOVENODEBRANCH_H
#define GODWHALE_SERVER_MOVENODEBRANCH_H

#include "movenodelist.h"

namespace godwhale {
namespace server {

/**
 * @brief ����ǖʂ̒T�����ʂ������Ǘ����܂��B
 */
class MoveNodeBranch
{
public:
    explicit MoveNodeBranch();
    explicit MoveNodeBranch(int iterationDepth, int plyDepth);
    ~MoveNodeBranch();

    /**
     * @brief �ǖ�ID���擾���܂��B
     */
    int getPositionId() const
    {
        return m_positionId;
    }

    /**
     * @brief �����[���̒T���[�����擾���܂��B
     */
    int getIterationDepth() const
    {
        return m_iterationDepth;
    }

    /**
     * @brief ���[�g�ǖʂ���̎w����̐����擾���܂��B
     */
    int getPlyDepth() const
    {
        return m_plyDepth;
    }

    /**
     * @brief �T�����ʂ̕]���l���擾���܂��B
     */
    int getBestValue() const
    {
        return m_bestValue;
    }

    /**
     * @brief ULEType(exact,lower,upper)���擾���܂��B
     */
    ULEType getBestULEType() const
    {
        return m_bestULE;
    }

    /**
     * @brief PV�Ɋ܂܂��w����̐����擾���܂��B
     */
    int getBestPVSize() const
    {
        return m_bestPV.size();
    }

    /**
     * @brief PV�Ɋ܂܂��w������擾���܂��B
     */
    Move getBestPV(int ply) const
    {
        return m_bestPV[ply];
    }

    /**
     * @brief ���̃I�u�W�F�N�g���Ή�����m�[�h�����J�b�g���ꂽ�����ׂ܂��B
     */
    bool isBetaCut() const
    {
        return (m_bestValue >= m_beta);
    }

    void initialize(int positionId);
    void setMoveList(std::vector<Move> const & list);
    //void clear();

    void updateValue(int value, ValueType vtype);
    void updateBest(int value, Move move, std::vector<Move> const & pv);

private:
    int m_positionId;
    int m_iterationDepth;
    int m_plyDepth;

    int m_alpha;
    int m_beta;
    int m_bestValue;
    ULEType m_bestULE;
    std::vector<Move> m_bestPV;

    bool m_moveListInited;
    std::vector<MoveNodeList> m_clientNodeLists;
};

}
}

#endif
