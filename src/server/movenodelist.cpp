#include "precomp.h"
#include "stdinc.h"
#include "movenodelist.h"

namespace godwhale {
namespace server {

MoveNodeList::MoveNodeList()
{
    // ������Ƃ���������
    m_nodeList.reserve(200);
}

MoveNodeList::~MoveNodeList()
{
}

/**
 * @brief �ݒ肳�ꂽ�w����ꗗ�����ׂăN���A���܂��B
 */
void MoveNodeList::clear()
{
    m_nodeList.clear();
}

/**
 * @brief �V���Ȍ�������ݒ肵�܂��B
 */
void MoveNodeList::addNewMove(Move move)
{
    m_nodeList.push_back(MoveNode(move));
}

/**
 * @brief �^����ꂽ�w��������m�[�h�̃C���f�b�N�X���擾���܂��B
 */
int MoveNodeList::indexOf(Move move) const
{
    int size = m_nodeList.size();

    for (int i = 0; i < size; ++i) {
        if (m_nodeList[i].getMove() == move) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief �v�Z���������Ă��Ȃ��w����̃C���f�b�N�X���擾���܂��B
 */
int MoveNodeList::indexOfUndone(int depth, int A, int B) const
{
    int size = m_nodeList.size();

    for (int i = 0; i < size; ++i) {
        if (!m_nodeList[i].isDone(depth, A, B)) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief �v�Z���������Ă���m�[�h�̐����擾���܂��B
 */
int MoveNodeList::getDoneCount(int depth, int A, int B) const
{
    int size = m_nodeList.size();
    int count = 0;

    for (int i = 0; i < size; ++i) {
        if (m_nodeList[i].isDone(depth, A, B)) {
            ++count;
        }
    }

    return count;
}
   
} // namespace server
} // namespace godwhale
