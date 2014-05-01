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

void Server::InitGame(const min_posi_t *posi)
{
    FOREACH_CLIENT(client) {
        client->InitGame(posi);
    }

    m_board = *posi;
    m_gid = 0;
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

struct Score
{
    bool IsValid;
    long TotalNodes;
    long MaxNodes;
    Move Move;
    long Nodes;
    int Value;
    std::vector<server::Move> PVSeq;

    explicit Score()
        : IsValid(false), TotalNodes(0), MaxNodes(0)
        , Move(MOVE_NA), Nodes(-1), Value(0) {
    }

    void MakeInvalid() {
        IsValid = false;
        TotalNodes = 0;
        MaxNodes = 0;
    }

    void UpdateNodes(shared_ptr<Client> client) {
        TotalNodes += client->GetNodeCount();
        MaxNodes    = std::max(MaxNodes, client->GetNodeCount());
    }

    void Set(const shared_ptr<Client> &client) {
        Move = (client->HasPlayedMove() ?
            client->GetPlayedMove() : client->GetMove());
        Nodes = client->GetNodeCount();
        Value = client->GetValue();
        PVSeq = client->GetPVSeq();
        IsValid = true;
    }
};

int Server::Iterate(tree_t *restrict ptree, int *value, std::vector<move_t> &pvseq)
{
    timer::cpu_timer timer, sendTimer;
    Score score;
    bool first = true;

    do {
        if (!first) {
            this_thread::yield();
            //this_thread::sleep(posix_time::milliseconds(100));
            score.MakeInvalid();
        }
        first = false;

        auto list = GetClientList();
        BOOST_FOREACH(auto client, list) {
            score.UpdateNodes(client);
        }

        int size = list.size();
        for (int i = 0; i < size; ++i) {
            auto client = list[i];
            ScopedLock locker(client->GetGuard());

            if (client->GetNodeCount() > 1L * 10000 &&
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
                    auto client2 = list[j];

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

        if (sendTimer.elapsed().wall > 5LL*1000*1000*1000) {
            auto ns  = timer.elapsed().wall;
            auto nps = (long)(score.TotalNodes / ((double)ns/1000/1000/1000));

            BOOST_FOREACH(auto client, list) {
                // �]���l�͐���+�Ƃ��������𑗂�܂��B
                client->SendCurrentInfo(list.size(), nps,
                                        score.Value * (client_turn == black ? +1 : -1));
            }

            // ����Ŏ��Ԃ����Z�b�g����܂��B
            sendTimer.start();
        }
    } while (!score.IsValid || !IsEndIterate(ptree, timer));

    if (score.IsValid) {
        LOG(Notification) << "  my move: " << ToString(score.Move);
        LOG(Notification) << "real move: " << ToString(score.PVSeq[0]);

        *value = score.Value;

        const auto &tmpseq = score.PVSeq;
        pvseq.insert(pvseq.end(), tmpseq.begin(), tmpseq.end());
        return 0;
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
        return IsThinkEnd(ptree, ms);
    }

    return false;
}

} // namespace server
} // namespace godwhale
