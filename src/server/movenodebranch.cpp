#include "precomp.h"
#include "stdinc.h"
#include "server.h"
#include "commandpacket.h"
#include "movenodebranch.h"

namespace godwhale {
namespace server {

MoveNodeBranch::MoveNodeBranch()
{
    throw std::logic_error("Not supported.");
}

MoveNodeBranch::MoveNodeBranch(int iterationDepth, int plyDepth)
    : m_iterationDepth(iterationDepth), m_plyDepth(plyDepth)
    , m_alpha(-score_bound), m_beta(score_bound)
    , m_bestValue(-score_bound), m_bestULE(ULE_NONE)
    , m_moveListInited(false)
{
    m_clientNodeLists.resize(CLIENT_SIZE);
}

MoveNodeBranch::~MoveNodeBranch()
{
}

/**
 * @brief �ǖ�ID�Ȃǂ̏��������s���܂��B
 */
void MoveNodeBranch::initialize(int positionId)
{
    BOOST_FOREACH (auto nodeList, m_clientNodeLists) {
        nodeList.clear();
    }

    m_positionId     = positionId;
    m_alpha          = -score_bound;
    m_beta           =  score_bound;
    m_bestValue      = -score_bound;
    m_bestULE        = ULE_NONE;
    m_moveListInited = false;
}

/**
 * @brief �e�N���C�A���g�Ɏw����𕪔z���܂��B
 */
void MoveNodeBranch::setMoveList(std::vector<Move> const & list)
{
    if (m_moveListInited) {
        return;
    }

    int i = 0;

    // �w����̕��z���s���܂��B
    while (true) {
        for (int ci = 0; ci < (int)m_clientNodeLists.size(); ++ci) {
            auto & nodeList = m_clientNodeLists[ci];

            if (i >= (int)list.size()) {
                break;
            }
            nodeList.addNewMove(list[i++]);
        }
    }

    // ���z���ꂽ�w������N���C�A���g�ɒʒm���܂��B
    for (int ci = 0; ci < (int)m_clientNodeLists.size(); ++ci) {
        auto & nodeList = m_clientNodeLists[ci];

        auto command = CommandPacket::createSetMoveList(m_positionId,
                                                        m_iterationDepth,
                                                        m_plyDepth,
                                                        nodeList.getMoveList());
        Server::get()->sendCommand(ci, command);
    }

    m_moveListInited = true;
}

/**
 * @brief �]���l�̍X�V���s���܂��B
 */
void MoveNodeBranch::updateValue(int value, ValueType vtype)
{
    switch (vtype) {
    case VALUETYPE_ALPHA: m_alpha = value; break;
    case VALUETYPE_BETA:  m_beta  = value; break;
    default:              unreachable();   break;
    }
}

/**
 * @brief �őP��Ƃ��̕]���l�̍X�V���s���܂��B
 */
void MoveNodeBranch::updateBest(int value, Move move, std::vector<Move> const & pv)
{
    if (move.isEmpty()) {
        LOG_ERROR() << "��̎w���肪�ݒ肳��܂����B";
        return;
    }

    m_bestValue = value;
    m_bestULE = (value >= m_beta ? ULE_LOWER : ULE_EXACT);

    m_bestPV.clear();
    m_bestPV.push_back(move);
    m_bestPV.insert(m_bestPV.end(), pv.begin(), pv.end());

    // ���l�̍X�V���s���܂��B
    updateValue(value, VALUETYPE_ALPHA);
}

} // namespace godwhale
} // namespace server
