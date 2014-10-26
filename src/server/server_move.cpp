#include "precomp.h"
#include "stdinc.h"
#include "move.h"
#include "server.h"
#include "serverclient.h"

/*
 * Server�N���X�̒��ł����Ɏw���萶���Ɋւ�镔���݂̂������Ă��܂��B
 */

namespace godwhale {
namespace server {
    
using namespace boost;

extern bool IsThinkEnd(tree_t *restrict ptree, unsigned int turnTimeMS);

#define FOREACH_CLIENT(VAR) \
    auto BOOST_PP_CAT(temp, __LINE__) = std::move(getClientList());  \
    BOOST_FOREACH(auto VAR, BOOST_PP_CAT(temp, __LINE__))

void Server::InitGame()
{
    LOG(Notification) << "Init Game";

    FOREACH_CLIENT(client) {
        //client->InitGame();
    }

    m_board = Position();
    m_gid = 0;
    m_isPlaying = true;
    m_currentValue = 0;
    m_turnTimer.start();
}

void Server::QuitGame()
{
    m_isPlaying = false;
    m_currentValue = 0;

    FOREACH_CLIENT(client) {
        //client->SendCommand("idle", false);
    }

    LOG(Notification) << "Quit Game";
}

void Server::ResetPosition(const min_posi_t *posi)
{
    FOREACH_CLIENT(client) {
        //client->ResetPosition(posi);
    }

    m_gid = 0;
    m_board = *posi;
    m_currentValue = 0;
    m_turnTimer.start();
}

void Server::MakeRootMove(Move move)
{
    m_board.makeMove(move);
    m_gid += 10;

    LOG(Notification) << "root move: " << move;
    LOG(Notification) << m_board;

    FOREACH_CLIENT(client) {
        //client->MakeRootMove(move, m_gid);
    }

    m_turnTimer.start();
}

void Server::UnmakeRootMove()
{
    m_board.unmakeMove();
    m_gid += 10; // �ǖʂ�߂����ꍇ���AID�͐i�߂܂��B

    LOG(Notification) << "root unmove";
}

void Server::AdjustTimeHook(int turn)
{
    /*auto sec = turn ? sec_w_total : sec_b_total;
    auto fmt = F("info %1% %2%") % (turn ? "wt" : "bt") % sec;
    auto str = fmt.str();

    FOREACH_CLIENT(client) {
        client->SendCommand(str, false);
    }

    LOG(Notification) << "All client send: " << str;*/
}

int Server::Iterate(tree_t *restrict ptree, int *value, std::vector<move_t> &pvseq)
{
#if 0
    LOG(Notification) << std::endl << std::endl;
    LOG(Notification) << "------------------ Begin Iterate.";
    LOG(Notification) << "thinking: " << ((game_status & flag_thinking) != 0);
    LOG(Notification) << "puzzling: " << ((game_status & flag_puzzling) != 0);
    LOG(Notification) << "pondering: " << ((game_status & flag_pondering) != 0);

    timer::cpu_timer sendTimer;
    for ( ; ; ) {
        auto clientList = GetClientList();
        long maxNodes = 0;
        Score score;

        // �L���Ȏw��������o�b�̒�����A�����Ƃ������T���m�[�h���𒲂ׂ܂��B
        BOOST_FOREACH(auto client, clientList) {
            if (client->HasMove()) {
                maxNodes = std::max(maxNodes, client->GetNodeCount());
            }
        }

        int size = clientList.size();
        for (int i = 0; i < size; ++i) {
            auto client = clientList[i];
            ScopedLock locker(client->GetGuard());

            if (client->GetNodeCount() > 30 * 10000 &&
                !client->HasPlayedMove() &&
                client->GetMove() != MOVE_NA) {
                Move move = client->GetMove();

                // ��ǂ݂̎�����肵�A�����ۂɐi�߂܂��B
                client->SetPlayedMove(move);

                // ignore���e�N���C�A���g�ɑ��M
                // ignore�͂قڂ��ׂẴN���C�A���g�ɑ��M���܂����A
                // �璷�����m�ۂ��邽�� chunk ���ƂɂP�͑���Ȃ����̂��c���܂��B
                int chunk = 5;
                int start = i % chunk;
                for (int j = start; j < size; ++j) {
                    auto client2 = clientList[j];

                    if (client2->HasPlayedMove()) {
                        continue;
                    }

                    // chunk���ƂɂP�͖�������w����𑗂�܂���B
                    if ((j % chunk) == start) {
                        client2->SetPlayedMove(move);
                    }
                    else {
                        client2->AddIgnoreMove(move);
                    }
                }
            }

            // �]���l�������A�m�[�h���������܂ŒႭ�Ȃ����I�т܂��B
            // �i�m�[�h���̔�����Ă���ł����̂��c�H���j
            bool flag1 = (client->GetNodeCount() > maxNodes * 0.7 &&
                          client->GetValue() > score.GetValue());
            if (client->HasMove() && (!score.IsValid() || flag1)) {
                score.Set(client);
            }
        }

        if (sendTimer.elapsed().wall > 5LL*1000*1000*1000) {
            if (score.IsValid()) {
                m_currentValue = score.GetValue() *
                                 (root_turn == client_turn ? +1 : -1);

                //SendPV(clientList, m_currentValue, score.GetNodes(),
                //       score.GetPVSeq());
            }

            // ����Ŏ��Ԃ����Z�b�g����܂��B
            sendTimer.start();
        }

        if (score.IsValid() && IsEndIterate(ptree, m_turnTimer)) {
            LOG(Notification) << "  my move: " << score.GetMove();
            LOG(Notification) << "real move: " << score.GetPVSeq()[0];

            *value = score.GetValue();

            const auto &tmpseq = score.GetPVSeq();
            pvseq.insert(pvseq.end(), tmpseq.begin(), tmpseq.end());
            return 0;
        }

        this_thread::yield();
    }

    *value = 0;
#endif
    return 0;
}

bool Server::IsEndIterate(tree_t *restrict ptree, timer::cpu_timer &timer)
{
    if (detect_signals(ptree) != 0) {
        return true;
    }

    if (!(game_status & flag_puzzling)) {
        auto ms = (unsigned int)(timer.elapsed().wall / 1000 / 1000);
        return IsThinkEnd(ptree, ms); //(ms > 10*1000);
    }

    return false;
}

} // namespace server
} // namespace godwhale
