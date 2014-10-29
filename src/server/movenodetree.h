
#ifndef GODWHALE_SERVER_MOVENODETREE_H
#define GODWHALE_SERVER_MOVENODETREE_H

#include "movenodebranch.h"

namespace godwhale {
namespace server {

/**
 * @brief ����ǖʂ̒T�����ʂ������Ǘ����܂��B
 */
class MoveNodeTree
{
public:
    explicit MoveNodeTree(int iterationDepth);
    ~MoveNodeTree();

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
     * @brief 
     */
    int getLastPlyDepth() const
    {
        return m_lastPlyDepth;
    }

    /**
     * @brief �w��̐[���̃m�[�h�u�����`���擾���܂��B
     */
    MoveNodeBranch &getBranch(int pld)
    {
        return m_nodeBranches[pld];
    }

    /**
     * @brief �w��̐[���̃m�[�h�u�����`���擾���܂��B
     */
    MoveNodeBranch const &getBranch(int pld) const
    {
        return m_nodeBranches[pld];
    }

    bool hasSamePV(int pld, std::vector<Move> const & pv);
    bool isDoneExact(int pld, int srd);

    void initialize(int positionId, std::vector<Move> const & pv);
    void setMoveList(int pld, std::vector<Move> const & list);
    void start(int startPld, int alpha, int beta);

private:
    int m_positionId;
    int m_iterationDepth;
    int m_lastPlyDepth;

    std::vector<Move> m_pvFromRoot;
    std::vector<MoveNodeBranch> m_nodeBranches;
};

} // namespace server
} // namespace godwhale

#endif
