#include "precomp.h"
#include "stdinc.h"
#include "syncposition.h"
#include "searchresult.h"
#include "movenodetree.h"
#include "server.h"

const int INITIAL_SEARCH_VALUE = (-score_bound-1);

const int ROOT_SEARCH_DEPTH_MIN = 4;
const int ROOT_SEARCH_DEPTH_MAX = 32;

const int MAX_ROOT_NODES  = 40000;
const int MAX_FIRST_NODES = 40000;

const static unsigned int state_node = (
    node_do_mate | node_do_null | node_do_futile |
    node_do_recap | node_do_recursion | node_do_hashcut);

namespace godwhale {
namespace server {

int Server::detectSignals()
{
    tree_t * restrict ptree = g_ptree;

    if (m_isRootCancelable &&
        ptree->node_searched >= m_startingNodeCount + MAX_ROOT_NODES) {
        m_isRootExceeded = 1;
        return PROCE_ABORT;
    }

    /*if (inFirst && !firstReplied &&
        preNodeCount + MAX_FIRST_NODES <= g_ptree->node_searched) {
        firstReplied = 1;
        replyFirst();
    }*/

    return PROCE_OK;
}

/**
 * @brief ���[�g���[�u���\�[�g���܂��B
 */
void Server::sortRootMoveList(tree_t * restrict ptree)
{
    root_move_t root_move_swap;
    int n = root_nmove;
    uint64_t sortv;
    int i, j, k, h;
      
    for ( k = SHELL_H_LEN - 1; k >= 0; k-- ) {
        h = ashell_h[k];
        for ( i = n-h-1; i > 0; i-- ) {
            root_move_swap = root_move_list[i];
            sortv          = root_move_list[i].nodes;
            for ( j = i+h; j < n && root_move_list[j].nodes > sortv; j += h ) {
                root_move_list[j-h] = root_move_list[j];
            }
            root_move_list[j-h] = root_move_swap;
        }
    }
}

/**
 * @brief ���[�g���炠����x�̐[���܂ŒT�����APV���擾���܂��B
 */
int Server::generateRootMove(SearchResult * result)
{
    tree_t * restrict ptree = g_ptree;

    m_isRootCancelable = false;
    m_isRootExceeded = false;

    // �Ƃ肠�����ǖʂ����ɖ߂��܂��B
    SyncPosition::get()->rewind();

    if (InCheck(Flip(root_turn))) {
        LOG_NOTIFICATION() << "������u�̋ǖʂ��n����܂����B";
        return -1;
    }
    
    LOG_NOTIFICATION() << "begom generate root moves";
    int value = make_root_move_list(ptree);

    // ���̎肪�Ȃ��ꍇ�͑΋ǂ��I�������܂��B
    if (root_nmove == 0) {
        LOG_NOTIFICATION() << "root_move�������ł��܂���ł����B�΋ǂ��I�����܂��B";
        return 0;
    }

    // �}���`�X���b�h�T�����s���ꍇ
#if defined(TLP)
    int iret = tlp_start();
    if (iret < 0) { return iret; }
#endif

    // iterative deepening search
    int iterationDepth;
    for (iterationDepth = ROOT_SEARCH_DEPTH_MIN;
         iterationDepth < ROOT_SEARCH_DEPTH_MAX;
         ++iterationDepth) {
        m_startingNodeCount = ptree->node_searched;

        LOG_NOTIFICATION() << F("begin root search depth=%1% ...") % iterationDepth;
        int value = search(ptree, -score_bound, score_bound, root_turn,
                           iterationDepth*PLY_INC + PLY_INC/2, 1/*ply*/, state_node);
        LOG_NOTIFICATION() << F("... done abort=%1% value=%2% node=%3%")
            % root_abort % value % (ptree->node_searched - m_startingNodeCount);

        if (root_abort) {
            bool exceeded = m_isRootExceeded;
            m_isRootCancelable = false;
            m_isRootExceeded = false;

            // �m�[�h�������ɂ��������ꍇ�́A�ō��m�[�h�̒T���ɓ���܂��B
            // �����łȂ��ꍇ�͒ʏ�̏I���Ȃ̂ŁA�v�l���I�������܂��B
            return (exceeded ? 1 : -1);
        }

        sortRootMoveList(ptree);

        // bonanza������T�����ʂ����o���܂��B
        result->initialize(ptree, iterationDepth, value);

        // �l�܂��ꂽor�l�܂����ꍇ
        if (abs(value) > score_max_eval) {
            return 0;
        }
        
        // �l�݂ł��l�܂���Ă����Ȃ��ꍇ
        m_isRootCancelable = true;
    }

    return 0;
}

/**
 * @brief �w���PV�ʂ�Ɏw�����i�߁A�w����ꗗ�𐶐����܂��B
 */
void Server::makeMoveAndMoveList(SearchResult const & sdata)
{
    assert(sdata.getPVSize() < 128);
    int pvSize = sdata.getPVSize();

    for (int pld = 0; pld < pvSize; ++pld) {
        //if ((singleCmd.haveList & (1 << i)) == 0) continue;

        if (pld == 0 && root_nmove == -1) {
            //makeRootMoveList();
            if (root_nmove <= 1) {
                return;
            }
        }

        // ���݂̋ǖʂ�\��
        LOG_DEBUG() << SyncPosition::get()->getPosition();

        // ���̋ǖʂŎw�����̈ꗗ���擾���܂��B
        auto list = SyncPosition::get()->getMoveList(sdata.getPV(pld), false);
        m_ntree->setMoveList(pld, list);

        // make a move
        SyncPosition::get()->makeMove(sdata.getPV(pld));
    }
}

/**
 * @brief �ō��m�[�h�����߂܂��B
 *
 * �^����ꂽPV�𖖒[����T�����A�ō��m�[�h���X�V���Ă����܂��B
 * �m�[�h�������l�𒴂����ꍇ�͒T����ł��؂�܂��B
 */
void Server::generateFirstMoves(SearchResult const & sdata)
{
    tree_t * restrict ptree = g_ptree;

    int itd = sdata.getIterationDepth();
    int pvSize = sdata.getPVSize();
    int A = -score_bound;
    int B =  score_bound;
    int turn = countFlip(root_turn, pvSize);

    LOG_NOTIFICATION() << "begin first";

    m_isFirstCancelable = false;
    m_isFirstExceeded = false;

    // �����ǖʂ����������܂��B
    SyncPosition::get()->initialize();

    // PV��i�߂A�w����ꗗ�𐶐����܂��B
    makeMoveAndMoveList(sdata);

    // �ǖʂ����ɖ߂��܂��B
    SyncPosition::get()->rewind();

    /*m_startingNodeCount = ptree->node_searched;

    // �w�����߂��Ȃ���A���[����T�����܂��B
    for (int ply = pvSize; ply >= 1; --ply) {
        // �ō��m�[�h�����Ȃ���΁A�����K���̗p���܂��B
        //if (s_eachPlyMoveList[ply].empty()) {
        //}
    }*/

#if 0
    // NOTE singlCmd.exd set is needed only after one srch is done (not here)
    // 12/11/2010 #5 was <=1
    for (ply = pvSize; ply >= 1; ply--) {
        if ((unsigned)(pren + MAX_FIRST_NODES) < g_ptree->node_searched)
            break;

        // ����\�Ȏw����̐�
        int n = (ply == pvSize) ? 2 : firstMvcnt[ply] + 1;

        if (n == 1 && ply != pvSize) {
            assert(ply>0);
            for (int i=singleCmd.bestseqLeng-1; i>=0; i--) {
                singleCmd.bestseq[i+1] = singleCmd.bestseq[i];
            }
            singleCmd.bestseq[0] = singleCmd.pv[ply-1];
            singleCmd.bestseqLeng++;
            singleCmd.exd--;
            singleCmd.val = -singleCmd.val;
            //running.val = -running.val;

            SyncPosition::get()->unmakeMove();
            continue;
        }

        //**** now perform search

        g_ptree->move_last[ply] = g_ptree->amove;
        g_ptree->save_eval[ply  ] =
        g_ptree->save_eval[ply+1] = INT_MAX;

        // srch each exd.  there s/b >=2 mvs here
        int depth = (itd + ply) * PLY_INC / 2;
        MOVE_LAST = singleCmd.pv[ply-1].v;

        SLTOut("-------- FIRST srch dep=%d ply=%d...", depth, ply);

        iteration_depth = itd;
        int value = -search(g_ptree, A, B, turn, newdep, ply+1, state_node);
            
        SLTOut(" ..done abt=%d val=%d len=%d #nod=%d mvs %07x %07x..\n",
               root_abort, value, g_ptree->pv[ply].length,
               g_ptree->node_searched - preNodeCount,
               readable_c(g_ptree->pv[ply].a[ply+1]),
               readable_c(g_ptree->pv[ply].a[ply+2]));

        if (root_abort) {
            SyncPosition::get()->rewind();
            s_isFirstCancelable = 0;
            return;
        }

        s_isFirstCancelable = true;

        //**** extend pv by using hashmv if leng insufficient

        /*if (ply == 1 && g_ptree->pv[ply].length < ply+1) {
            hash_probe(g_ptree, ply+1, newdep, turn,
                       -score_bound, score_bound, &state_node_dmy);
            int hashmv = g_ptree->amove_hash[ply+1];

            // add it to pv if it's valid mv
            if (hashmv != MOVE_NA && hashmv != MOVE_PASS &&
                is_move_valid(g_ptree, hashmv, turn)) {
                MakeMove(turn, hashmv, ply+1);
                if (!InCheck(turn)) {
                    g_ptree->pv[ply].length = ply+1;
                    g_ptree->pv[ply].a[ply+1] = hashmv;
                }
                UnMakeMove(turn, hashmv, ply+1);
            }
        }*/

        // if srch stops at ply, exd will be ply-1. bestseq[0] is pv[ply-1]
        int newleng = ptree->pv[ply].length - ply;
        if (newleng == 1 &&
            singleCmd.pvleng > ply &&
            singleCmd.bestseqLeng > 0 &&
            singleCmd.bestseq[0].v == (int)ptree->pv[ply].a[ply+1]) {
            newleng = singleCmd.bestseqLeng;
            for (int i = newleng-1; i>=0; i--) {
                singleCmd.bestseq[i+1] = singleCmd.bestseq[i];
            }
        }
        else {
            // 12/15/2010 #41 length*-ply* was missing
            forr (i, 0, newleng-1) {  // FIXME? ply+1?
                singleCmd.bestseq[i+1] = ptree->pv[ply].a[ply+i+1];
            }
        }

        singleCmd.val = running.val = val; // FIXME? running.val not needed?
        singleCmd.exd = ply-1;
        singleCmd.bestseq[0] = singleCmd.pv[ply-1];
        singleCmd.bestseqLeng = newleng+1;
        // FIXME follow hash in order to extend leng?

        // 12/11/2010 #9 was turn/ply/ply+1
        SyncPosition::get()->unmakeMove();
        val = -val;
    }

    inFirst = 0;

    //singleCmd.exd = ply;   // 12/11/2010 #12 was ply--
    //singleCmd.val = running.val;
    singleCmd.ule = ULE_EXACT;

    SyncPosition::get()->rewind();

    if (!firstReplied) {
        singleCmd.exd++;
        singleCmd.val = -singleCmd.val;
        //replyFirst();
    }
#endif
}

} // namespace godwhale
} // namespace server
