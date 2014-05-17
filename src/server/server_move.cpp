#include "precomp.h"
#include "stdinc.h"
#include "move.h"
#include "server.h"
#include "client.h"

/*
 * Server�N���X�̒��ł����Ɏw���萶���Ɋւ�镔���݂̂������Ă��܂��B
 */

namespace godwhale {
namespace server {

#define FOREACH_CLIENT(VAR) \
    auto BOOST_PP_CAT(temp, __LINE__) = std::move(GetClientList());  \
    BOOST_FOREACH(auto VAR, BOOST_PP_CAT(temp, __LINE__))

void Server::InitGame()
{
    FOREACH_CLIENT(client) {
        client->InitGame();
    }

    m_board = Board();
    m_gid = 0;

    //sec_limit = 1500;
    //sec_limit_up = 0;
}

void Server::ResetPosition(const min_posi_t *posi)
{
    FOREACH_CLIENT(client) {
        client->ResetPosition(posi);
    }

    m_board = *posi;
}

void Server::MakeRootMove(move_t move)
{
    m_board.DoMove(move);
    m_gid += 10;

    LOG(Notification) << "root move: " << ToString(move);
    LOG(Notification) << m_board;

    FOREACH_CLIENT(client) {
        client->MakeRootMove(move, m_gid);
    }
}

void Server::UnmakeRootMove()
{
    m_board.DoUnmove();
    m_gid += 10; // �ǖʂ�߂����ꍇ���AID�͐i�߂܂��B

    LOG(Notification) << "root unmove";
}

void Server::AdjustTimeHook(int turn)
{
    auto sec = turn ? sec_w_total : sec_b_total;
    auto fmt = format("info %1% %2%") % (turn ? "wt" : "bt") % sec;
    auto str = fmt.str();

    FOREACH_CLIENT(client) {
        client->SendCommand(str);
    }
}

int Server::Iterate(tree_t *restrict ptree, int *value, std::vector<move_t> &pvseq)
{
    timer::cpu_timer timer, sendTimer;

    for ( ; ; ) {
        Score score;

        auto clientList = GetClientList();
        BOOST_FOREACH(auto client, clientList) {
            if (client->HasMove()) {
                score.UpdateNodes(client);
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
                        continue;
                    }

                    client2->AddIgnoreMove(move);
                }
            }

            // �]���l�������A�m�[�h���������܂ŒႭ�Ȃ����I�т܂��B
            // �i�m�[�h���̔�����Ă���ł����̂��c�H���j
            bool flag1 = (client->GetNodeCount() > score.MaxNodes * 0.7 &&
                          client->GetValue() > score.Value);
            if (client->HasMove() && (!score.IsValid || flag1)) {
                score.Set(client);
            }
        }

        score.SetNps(timer);
        if (sendTimer.elapsed().wall > 5LL*1000*1000*1000) {
            SendCurrentInfo(clientList, score);
            SendPV(clientList, score);

            // ����Ŏ��Ԃ����Z�b�g����܂��B
            sendTimer.start();
        }

        if (score.IsValid && IsEndIterate(ptree, timer)) {
            LOG(Notification) << "  my move: " << score.GetMove();
            LOG(Notification) << "real move: " << score.PVSeq[0];

            *value = score.Value;

            const auto &tmpseq = score.PVSeq;
            pvseq.insert(pvseq.end(), tmpseq.begin(), tmpseq.end());
            return 0;
        }

        this_thread::yield();
    }

    *value = 0;
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

void Server::SendCurrentInfo(std::vector<shared_ptr<Client> > &clientList,
                             Score &score)
{
    // �]���l�̓N�W������񂩂�݂������𑗂�܂��B
    auto fmt = format("info current %1% %2% %3%")
        % clientList.size()
        % score.Nps % score.Value;
    auto str = fmt.str();

    LOG(Notification) << str;

    BOOST_FOREACH(auto client, clientList) {
        client->SendCommand(str, false);
    }
}

void Server::SendPV(std::vector<shared_ptr<Client> > &clientList,
                    Score &score)
{
    if (!score.IsValid) {
        return;
    }

    std::vector<std::string> v;
    std::transform(
        score.PVSeq.begin(), score.PVSeq.end(),
        std::back_inserter(v),
        [](Move _) { return _.String(); });

    auto fmt = format("info %1% %2% n=%3%")
        % ((double)score.Value / 100.0)
        % algorithm::join(v, " ")
        % score.Nodes;
    std::string command = fmt.str();

    BOOST_FOREACH(auto client, clientList) {
        if (client->IsSendPV()) {
            client->SendCommand(command);
        }
    }
}

} // namespace server
} // namespace godwhale
