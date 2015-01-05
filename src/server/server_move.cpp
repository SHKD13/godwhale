#include "precomp.h"
#include "stdinc.h"
#include "commandpacket.h"
#include "replypacket.h"
#include "searchresult.h"
#include "movenodetree.h"
#include "syncposition.h"
#include "server.h"
#include "serverclient.h"

/*
 * Server�N���X�̒��ł����Ɏw���萶���Ɋւ�镔���݂̂������Ă��܂��B
 */

namespace godwhale {
namespace server {
    
using namespace boost;

extern bool IsThinkEnd(tree_t *restrict ptree, unsigned int turnTimeMS);

/**
 * @brief �T�[�o�[�p�R�}���h�̏������s���܂��B
 */
int Server::setPosition(Position const & position)
{
    LOG_NOTIFICATION() << "set position";

    m_positionId += 1;
    m_position = position;
    m_currentValue = 0;

    // �ǖʂ��N���C�A���g�ɒʒm���܂��B
    for (int ci = 0; ci < CLIENT_SIZE; ++ci) {
        auto command = CommandPacket::createSetPosition(m_positionId,
                                                        position);
        Server::get()->sendCommand(ci, command);
    }

    return PROCE_CONTINUE;
}

int Server::beginGame()
{
    LOG_NOTIFICATION() << "begin game";

    m_currentValue = 0;
    m_turnTimer.start();

    return PROCE_CONTINUE;
}

int Server::endGame()
{
    LOG_NOTIFICATION() << "end game";

    m_positionId = 0;
    m_currentValue = 0;

    // �S�N���C�A���g���~���܂��B
    sendCommandAll(CommandPacket::createStop());

    return PROCE_CONTINUE;
}

int Server::makeMoveRoot(Move move)
{
    int oldPositionId = m_positionId;
    m_positionId += 1;

    SyncPosition::get()->rewind();
    if (!SyncPosition::get()->makeMoveRoot(m_positionId, move)) {
        LOG_ERROR() << move << ": makeMoveRoot�Ɏ��s���܂����B";
        LOG_ERROR() << SyncPosition::get()->getPosition();
        return PROCE_ABORT;
    }

    LOG(Notification) << "root move: " << move;
    LOG(Notification) << SyncPosition::get()->getPosition();

    // �ǖʂ��N���C�A���g�ɒʒm���܂��B
    for (int ci = 0; ci < CLIENT_SIZE; ++ci) {
        auto command = CommandPacket::createMakeMoveRoot(m_positionId,
                                                         oldPositionId,
                                                         move);
        Server::get()->sendCommand(ci, command);
    }

    m_turnTimer.start();

    return PROCE_CONTINUE;
}

int Server::unmakeMoveRoot()
{
    LOG_NOTIFICATION() << "root unmove";

    throw std::logic_error("unmakeMoveRoot: ��������Ă��܂���B");
}

/**
 * @brief �N���C�A���g����M���������R�}���h���������܂��B
 */
int Server::proceClientReply()
{
    auto clientList = getClientList();
    int count = 0;
    
    BOOST_FOREACH(auto client, clientList) {
        // ���ׂẲ����R�}���h���������܂��B
        while (client->proce() > 0) {
            count += 1;
        }
    }

    return count;
}

int Server::iterate(tree_t *restrict ptree, int *value, std::vector<move_t> &pvseq)
{
    LOG_NOTIFICATION() << "";
    LOG_NOTIFICATION() << "";
    LOG_NOTIFICATION() << "";
    LOG_NOTIFICATION() << "------------------ Begin Iterate.";
    LOG_NOTIFICATION() << "thinking: " << ((game_status & flag_thinking) != 0);
    LOG_NOTIFICATION() << "puzzling: " << ((game_status & flag_puzzling) != 0);
    LOG_NOTIFICATION() << "pondering: " << ((game_status & flag_pondering) != 0);

    SearchResult result;
    generateRootMove(&result);

    // �ō��m�[�h��ݒ肵�܂��B
    m_ntree->initialize(m_positionId, result.getIterationDepth(), result.getPV());

    // �w���胊�X�g��ݒ肵�܂��B
    generateFirstMoves(result);

    while (m_isAlive) {
        int count = proceClientReply();
        if (count == 0) {
            sleep(200);
            continue;
        }
    }

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
