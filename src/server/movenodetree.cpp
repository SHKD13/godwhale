#include "precomp.h"
#include "stdinc.h"
#include "server.h"
#include "commandpacket.h"
#include "movenodetree.h"

namespace godwhale {
namespace server {

MoveNodeTree::MoveNodeTree(int iterationDepth)
    : m_positionId(-1), m_iterationDepth(iterationDepth), m_lastPlyDepth(-1)
{
}

MoveNodeTree::~MoveNodeTree()
{
}

/**
 * @brief pld�܂ł̐[����PV���A����PV�Ɠ�������r���܂��B
 */
bool MoveNodeTree::hasSamePV(int pld, std::vector<Move> const & pv)
{
    if (pld >= (int)m_pvFromRoot.size()) {
        return false;
    }

    for (int d = 0; d < pld; ++d) {
        if (pv[d] != m_pvFromRoot[d]) {
            return false;
        }
    }

    return true;
}

/**
 * @brief pld�܂ł̐[����PV���A����PV�Ɠ�������r���܂��B
 */
bool MoveNodeTree::isDoneExact(int pld, int srd)
{
    return false;
}

/**
 * @brief �ǖ�ID��ō��m�[�h�ƂȂ�PV��ݒ肵�܂��B
 */
void MoveNodeTree::initialize(int positionId, int iterationDepth,
                              std::vector<Move> const & pv)
{
    //m_nodeBranches.resize(pv.size());
    m_nodeBranches.clear();

    for (int pld = 0; pld < (int)pv.size(); ++pld) {
        auto branch = MoveNodeBranch(iterationDepth, pld);

        // �e�[���̃u�����`�����������܂��B
        branch.initialize(positionId);
        m_nodeBranches.push_back(branch);
    }

    m_positionId     = positionId;
    m_iterationDepth = iterationDepth;
    m_pvFromRoot     = pv;
    m_lastPlyDepth   = pv.size() - 1;
}

/**
 * @brief �e�w����[���̌���ꗗ��ݒ肵�܂��B
 */
void MoveNodeTree::setMoveList(int pld, std::vector<Move> const & list)
{
    if (pld >= (int)m_nodeBranches.size()) {
        LOG_ERROR() << "pld=" << pld << ": �Ή����Ă��Ȃ��w����̐[���ł��B";
        return;
    }

    m_nodeBranches[pld].setMoveList(list);
}

/**
 * @brief �w��̐[������A�^����ꂽ�l����ɒT�����J�n���܂��B
 */
void MoveNodeTree::start(int startPld, int alpha, int beta)
{
    if (startPld >= (int)m_nodeBranches.size()) {
        LOG_ERROR() << "pld=" << startPld << ": �Ή����Ă��Ȃ��w����̐[���ł��B";
        return;
    }

    auto command = CommandPacket::createStart(m_positionId, m_iterationDepth,
                                              startPld, alpha, beta);
    Server::get()->sendCommandAll(command);
}

} // namespace godwhale
} // namespace server
