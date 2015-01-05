#include "precomp.h"
#include "stdinc.h"
#include "syncposition.h"

namespace godwhale {

shared_ptr<SyncPosition> SyncPosition::ms_instance(new SyncPosition);

SyncPosition::SyncPosition()
    : m_positionId(-1), m_moveSize(0)
{
    memset(m_moveList, 0, sizeof(m_moveList));
    memset(m_checksList, 0, sizeof(m_checksList));
}

/**
 * @brief �ǖʂ����[�g�ǖʂɖ߂��Abona�̏��������s���܂��B
 */
void SyncPosition::initialize()
{
    tree_t * restrict ptree = g_ptree;

    rewind();

    // bonanza�̒T���p�f�[�^�����������܂��B
    initBonanza(ptree);
}

#if 1 || defined(GODWHALE_CLIENT)
/**
 * @brief �ǖʂ��w��̋ǖʂɐݒ肵�Abona�̏��������s���܂��B
 */
void SyncPosition::reset(int positionId, Position const & position)
{
    tree_t * restrict ptree = g_ptree;
    min_posi_t posi;

    rewind();

    // bonanza���̋ǖʂ��ݒ肵�܂��B
    posi = position.getMinPosi();
    ini_game(ptree, &posi, 0, NULL, NULL);

    m_positionId = positionId;
    m_rootPosition = Position(posi);

    // bonanza�̒T���p�f�[�^�����������܂��B
    initialize();
}
#endif

/**
 * @brief �T���̂��߂̏������������s���܂��B
 */
void SyncPosition::initBonanza(tree_t * restrict ptree)
{
    ptree->node_searched         =  0;
    ptree->null_pruning_done     =  0;
    ptree->null_pruning_tried    =  0;
    ptree->check_extension_done  =  0;
    ptree->recap_extension_done  =  0;
    ptree->onerp_extension_done  =  0;
    ptree->nfour_fold_rep        =  0;
    ptree->nperpetual_check      =  0;
    ptree->nsuperior_rep         =  0;
    ptree->nrep_tried            =  0;
    ptree->neval_called          =  0;
    ptree->nquies_called         =  0;
    ptree->ntrans_always_hit     =  0;
    ptree->ntrans_prefer_hit     =  0;
    ptree->ntrans_probe          =  0;
    ptree->ntrans_exact          =  0;
    ptree->ntrans_lower          =  0;
    ptree->ntrans_upper          =  0;
    ptree->ntrans_superior_hit   =  0;
    ptree->ntrans_inferior_hit   =  0;
    ptree->fail_high             =  0;
    ptree->fail_high_first       =  0;
    ptree->current_move[0]       =  0;
    ptree->save_eval[0]          =  INT_MAX;
    ptree->save_eval[1]          =  INT_MAX;
    ptree->pv[0].a[0]            =  0;
    ptree->pv[0].a[1]            =  0;
    ptree->pv[0].depth           =  0;
    ptree->pv[0].length          =  0;
    ptree->nsuc_check[0]         =  0;
    ptree->nsuc_check[1]         =  InCheck(root_turn) ? 1 : 0;
    ptree->move_last[0]          =  ptree->amove;
    ptree->move_last[1]          =  ptree->amove;
    iteration_depth              =  0;
    easy_value                   =  0;
    easy_abs                     =  0;
    root_abort                   =  0;
    root_nmove                   =  0;
    root_value                   = -score_bound;
    root_alpha                   = -score_bound;
    root_beta                    =  score_bound;
    root_index                   =  0;
    root_move_list[0].status     = flag_first;
    node_last_check              =  0;
    time_last_check              =  time_start;
    game_status                 &= ~( flag_move_now /*| flag_suspend*/
                                    | flag_quit_ponder | flag_search_error );

#if defined(TLP)
    ptree->tlp_abort             = 0;
    tlp_nsplit                   = 0;
    tlp_nabort                   = 0;
    tlp_nslot                    = 0;
#endif

    for (int ply = 0; ply < PLY_MAX; ply++) {
        ptree->amove_killer[ply].no1 =
        ptree->amove_killer[ply].no2 = 0U;
        ptree->killers[ply].no1 =
        ptree->killers[ply].no2 = 0x0U;
    }

    // �����ǖʂ̉���`�F�b�N
    bool checked = (InCheck(root_turn) != 0);
    m_checksList[0] = checked; // �C���f�b�N�X��0�ŊJ�n

    ptree->nsuc_check[0] = 0;
    ptree->nsuc_check[1] = checked ? 1 : 0;
}

/**
 * @brief ���̋ǖʂɑΉ�����Position�N���X���擾���܂��B
 */
Position SyncPosition::getPosition() const
{
    tree_t * restrict ptree = g_ptree;

    min_posi_t mposi;
    memcpy(mposi.asquare, ptree->posi.asquare, sizeof(mposi.asquare));
    mposi.hand_black = ptree->posi.hand_black;
    mposi.hand_white = ptree->posi.hand_white;
    mposi.turn_to_move = root_turn;

    return Position(mposi);
}

/**
 * @brief ���[�g�̎w������P��i�߂܂��B
 */
bool SyncPosition::makeMoveRoot(int positionId, Move move)
{
    tree_t * restrict ptree = g_ptree;

    rewind();
    m_positionId = positionId;

    if (!m_rootPosition.makeMove(move)) {
        LOG_ERROR() << move << ": �w����𐳂����w�����Ƃ��ł��܂���B";
        return false;
    }

    int status = 
#if defined(GODWHALE_SERVER)
        // �T�[�o�[�̏ꍇ��bonanza���������makemoveroot���邽��
        // �����ł͋ǖʂ̓��������s���悤�ɂ��܂��B
        0;
#else
        make_move_root(ptree, move, flag_time);
#endif

    initialize();

    return (status >= 0);
}

/**
 * @brief �w������P��i�߂܂��B
 */
void SyncPosition::makeMove(Move move)
{
    tree_t * restrict ptree = g_ptree;
    int i = m_moveSize;
    int turn = countFlip(root_turn, i);

    // �w����̊m�F�Ȃ�
    assert(is_move_valid(ptree, move.get(), turn));
    m_moveList[i] = move;
    MakeMove(turn, move.get(), i+1);
    assert(!InCheck(turn));

    // ���肳��Ă��邩�`�F�b�N
    bool checked = (InCheck(Flip(turn)) != 0);
    m_checksList[i+1] = checked;
    ptree->nsuc_check[i+2] = checked ? ptree->nsuc_check[i]+1 : 0;

    m_moveSize += 1;
}

/**
 * @brief �w������P��߂��܂��B
 */
void SyncPosition::unmakeMove()
{
    tree_t * restrict ptree = g_ptree;
    int i = m_moveSize - 1;
    int turn = countFlip(root_turn, i);

    assert(m_moveSize > 0);

    UnMakeMove(turn, m_moveList[i].get(), i+1);
    m_checksList[i+1] = false;

    m_moveSize -= 1;
}

/**
 * @brief bonanza�̋ǖʂ����ɖ߂��܂��B
 */
void SyncPosition::rewind()
{
    while (m_moveSize > 0) {
        unmakeMove();
    }
}

/**
 * @brief ���[�g�ǖʂ���w�����ݒ肵�Ȃ����܂��B
 */
void SyncPosition::resetFromRoot(const std::vector<Move> & pv)
{
    tree_t * restrict ptree = g_ptree;
    int len = pv.size();

    // ���Ɠ����ǖʂ̏ꍇ�́A�������܂���B
    if (pv.size() == m_moveSize &&
        std::equal(pv.begin(), pv.end(), &m_moveList[0])) {
        return;
    }

    // �Ƃ肠�����ǖʂ�߂��܂��B
    rewind();

    BOOST_FOREACH(auto move, pv) {
        makeMove(move);
    }
}

void SyncPosition::extendPV()
{
}

// copied from searchr.c, modified
static int next_root_move(tree_t * restrict ptree)
{
    int n = root_nmove;

    for (int i = 0; i < n; i++ ) {
        if ( root_move_list[i].status & flag_searched ) continue;

        root_move_list[i].status |= flag_searched;
        ptree->current_move[1]    = root_move_list[i].move;
        return 1;
    }

    return 0;
}

/**
 * @brief ����ǖʂŒ���\�Ȏw��������X�g�A�b�v���܂��B
 */
std::vector<Move> SyncPosition::getMoveList(Move exclude, bool firstMoveOnly)
{
    tree_t * restrict ptree = g_ptree; 
    
    std::vector<Move> result;
    int ply = m_moveSize + 1;
    int turn = countFlip(root_turn, ply-1);
    int i = 0;

    ptree->move_last[ply  ] =
    ptree->move_last[ply-1] = ptree->amove;
    ptree->anext_move[ply].next_phase = next_move_hash;

    if (ply == 1) {
        make_root_move_list(ptree);
        /*for (int i = 0; i < root_nmove; ++i) {
            root_move_list[i].status = 0;
        }*/
    }

    while (ply == 1 ? next_root_move(ptree) :
           ptree->nsuc_check[ply] ?
               gen_next_evasion(ptree, ply, turn) :
               gen_next_move(ptree, ply, turn)) {
        //assert(is_move_valid(ptree, MOVE_CURR, turn));
        // ���܂ɃG���[���������邽�߁A�ꎞ�I�ɓ����悤�ɂ��Ă��܂��B:TODO
        if (!is_move_valid(ptree, MOVE_CURR, turn)) {
            LOG_ERROR() << toString(MOVE_CURR, turn);
            continue;
        }

        if (MOVE_CURR == exclude) continue;
        result.push_back(MOVE_CURR);
        //if (firstMoveOnly && result.size() >= 2) break;
    }

    return result;
}

} // namespace godwhale
