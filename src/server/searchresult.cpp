#include "precomp.h"
#include "stdinc.h"
#include "searchresult.h"

namespace godwhale {
namespace server {

SearchResult::SearchResult()
    : m_iterationDepth(-1), m_value(0), m_ule(ULE_NONE)
{
}

/**
 * @brief bonanza���T������PV�Ȃǂ�ۑ����܂��B
 */
void SearchResult::initialize(tree_t * restrict ptree, int iterationDepth,
                              int value)
{
    int len = ptree->pv[0].length;

    // PV����ۑ����܂��B
    m_pv.resize(len);
    for (int i = 0; i < len; ++i) {
        m_pv[i] = ptree->pv[0].a[i+1];
    }

    // �]���l�Ȃǂ�ۑ����܂��B
    m_iterationDepth = iterationDepth;
    m_ule = ULE_EXACT;
    m_value = value;

    assert(m_pv.size() >= 1 || m_value < -score_max_eval);

    // ���O�o��
    std::list<std::string> list;
    int turn = root_turn;
    for (int i = 0; i < len; ++i) {
        list.push_back(m_pv[i].str(turn));
        turn = Flip(turn);
    }
    LOG_NOTIFICATION() << "length=" << len << " pv=" << boost::join(list, "");
}

} // namespace godwhale
} // namespace server
