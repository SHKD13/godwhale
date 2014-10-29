#ifndef GODWHALE_SERVER_MOVENODELIST_H
#define GODWHALE_SERVER_MOVENODELIST_H

#include "movenode.h"

namespace godwhale {
namespace server {

/**
 * @brief �w����Ƃ��̒T�����ʂ��܂Ƃ߂ĕێ����܂��B
 */
class MoveNodeList
{
public:
    explicit MoveNodeList();
    ~MoveNodeList();

    /**
     * @brief �m�[�h���X�g���擾���܂��B
     */
    std::vector<MoveNode> const &getList() const
    {
        return m_nodeList;
    }

    /**
     * @brief �w����̃��X�g���擾���܂��B
     */
    std::vector<Move> getMoveList() const
    {
        std::vector<Move> result;

        BOOST_FOREACH (auto node, m_nodeList) {
            result.push_back(node.getMove());
        }

        return result;
    }

    /**
     * @brief �m�[�h�����擾���܂��B
     */
    int getSize() const
    {
        return m_nodeList.size();
    }

    /**
     * @brief �w��̃C���f�b�N�X�̃m�[�h���擾���܂��B
     */
    MoveNode &operator[](int index)
    {
        return m_nodeList[index];
    }

    /**
     * @brief �w��̃C���f�b�N�X�̃m�[�h���擾���܂��B
     */
    MoveNode const &operator[](int index) const
    {
        return m_nodeList[index];
    }

    /**
     * @brief ���X�g���̎w����̌v�Z�����ׂĊ������Ă��邩���ׂ܂��B
     */
    bool isDone(int depth, int A, int B) const
    {
        return (getUndoneCount(depth, A, B) == 0);
    }

    /**
     * @brief ���X�g���̎w����̌v�Z�����ׂĊ������Ă��邩���ׂ܂��B
     */
    void setRetrying(Move move)
    {
        int index = indexOf(move);
        if (index >= 0) {
            m_nodeList[index].setRetrying();
        }
    }

    /**
     * @brief �v�Z���������Ă��Ȃ��m�[�h�̐����擾���܂��B
     */
    int getUndoneCount(int depth, int A, int B) const
    {
        return (m_nodeList.size() - getDoneCount(depth, A, B));
    }

    void clear();
    void addNewMove(Move move);

    int indexOf(Move move) const;
    int indexOfUndone(int depth, int A, int B) const;
    int getDoneCount(int depth, int A, int B) const;

private:
    std::vector<MoveNode> m_nodeList;
};

} // namespace server
} // namespace godwhale

#endif
