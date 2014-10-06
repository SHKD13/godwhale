#ifndef GODWHALE_MOVENODE_H
#define GODWHALE_MOVENODE_H

#include "move.h"

namespace godwhale {

/**
 * @brief �]���l�̎�ނ������܂��B
 */
typedef enum
{
    /// ���ɂȂ��H
    ULE_NONE  = 0,
    /// �������]���l�̉����l�ł��邱�Ƃ������B
    ULE_LOWER = 1,
    /// �������]���l�̏���l�ł��邱�Ƃ������B
    ULE_UPPER = 2,
    /// �������]���l���̂��̂ł��邱�Ƃ������B
    ULE_EXACT = 3,
} ULEType;


/**
 * @brief �w����Ƃ��̒T�����ʂ�ێ����܂��B
 */
class MoveNode
{
public:
    explicit MoveNode(Move move);
    ~MoveNode();

    /**
     * @brief ���̃m�[�h�ɑΉ�����w������擾���܂��B
     */
    Move getMove() const
    {
        return m_move;
    }

    /**
     * @brief �T���������̎�̍őP����擾���܂��B
     */
    Move getBestMove() const
    {
        return m_bestMove;
    }

    /**
     * @brief �w����̒T���[�����擾���܂��B
     */
    int getDepth() const
    {
        return m_depth;
    }

    /**
     * @brief �T�������m�[�h�����擾���܂��B
     */
    int getNodes() const
    {
        return m_nodes;
    }

    /**
     * @brief �]���l�̏���l���擾���܂��B
     */
    int getUpper() const
    {
        return m_upper;
    }

    /**
     * @brief �]���l�̉����l���擾���܂��B
     */
    int getLower() const
    {
        return m_lower;
    }

    /**
     * @brief �T�����ɕ]���l�̍X�V���s��ꂽ���ǂ������擾���܂��B
     */
    bool isRetrying() const
    {
        return m_retrying;
    }

    /**
     * @brief �T�����ɕ]���l�̍X�V���s��ꂽ�Ƃ��ɌĂ�ł��������B
     */
    void setRetrying()
    {
        m_retrying = true;
    }

    bool isDone(int depth, int A, int B) const;
    bool isExact(int depth) const;

    void update(int depth, int value, ULEType ule, int nodes, Move bestMove);
    void update(int depth, int value, int lower, int upper, int nodes, Move bestMove);

private:
    Move m_move;
    Move m_bestMove;
    int m_depth, m_nodes;
    short m_upper, m_lower;
    bool m_retrying;
};


/**
 * @brief �w����Ƃ��̒T�����ʂ����X�g�ŕێ����܂��B
 */
class MoveNodeList
{
public:
    explicit MoveNodeList();
    ~MoveNodeList();

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

    int indexOf(Move move) const;
    int indexOfUndone(int depth, int A, int B) const;
    int getDoneCount(int depth, int A, int B) const;

private:
    std::vector<MoveNode> m_nodeList;
};

} // namespace godwhale

#endif
