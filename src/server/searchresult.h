
#ifndef GODWHALE_SERVER_SEARCHRESULT_H
#define GODWHALE_SERVER_SEARCHRESULT_H

#include "movenode.h"

namespace godwhale {
namespace server {

/**
 * @brief ���[�g����̒T�����s���A���̌��ʂ�ێ����邽�߂̃N���X�ł��B
 */
class SearchResult
{
public:
    explicit SearchResult();

    /**
     * @brief �����[���̒T���[�����擾���܂��B
     */
    int getIterationDepth() const
    {
        return m_iterationDepth;
    }

    /**
     * @brief �T�����ʂ̕]���l���擾���܂��B
     */
    int getValue() const
    {
        return m_value;
    }

    /**
     * @brief ULEType(exact,lower,upper)���擾���܂��B
     */
    ULEType getULEType() const
    {
        return m_ule;
    }

    /**
     * @brief PV�Ɋ܂܂��w����̐����擾���܂��B
     */
    int getPVSize() const
    {
        return m_pv.size();
    }

    /**
     * @brief PV�Ɋ܂܂��w������擾���܂��B
     */
    Move getPV(int ply) const
    {
        return m_pv[ply];
    }

    /**
     * @brief PV���擾���܂��B
     */
    std::vector<Move> const & getPV() const
    {
        return m_pv;
    }

    void initialize(tree_t * restrict ptree, int iterationDepth, int value);

private:
    int m_iterationDepth;
    std::vector<Move> m_pv;

    int m_value;
    ULEType m_ule;
};

}
}

#endif
