#include "precomp.h"
#include "stdinc.h"
#include "commandpacket.h"
#include "replypacket.h"
#include "server.h"
#include "serverclient.h"

namespace godwhale {
namespace server {

ServerClient::ServerClient(shared_ptr<Server> server)
    : m_server(server), m_isAlive(false), m_logined(false)
    , m_threadCount(0), m_sendPV(false)
{
}

ServerClient::~ServerClient()
{
    close();

    LOG_DEBUG() << "ServerClient[" << m_loginId << "]�͕����܂����B";
}

/**
 * @brief �ԓ��R�����g�̎�M�Ȃǂ��J�n���܂��B
 */
void ServerClient::initialize(shared_ptr<tcp::socket> socket)
{
    if (m_rsiService != nullptr) {
        LOG_ERROR() << "RSIService�͂��łɏ���������Ă��܂��B";
        return;
    }

    // �R�}���h�̑���M�T�[�r�X���J�n���܂��B
    m_rsiService.reset(new RSIService(shared_from_this()));
    m_rsiService->startReceive(socket);

    m_isAlive = true;
}

/**
 * @brief �R�l�N�V������ؒf���܂��B
 */
void ServerClient::close()
{
    if (m_rsiService != nullptr) {
        m_rsiService->close();
        m_rsiService = nullptr;
    }

    m_isAlive = false;
}

/**
 * @brief �R�l�N�V�����̐ؒf���ɌĂ΂�܂��B
 */
void ServerClient::onDisconnected()
{
    LOG_NOTIFICATION() << "ServerClient[" << m_loginId << "] is disconnected.";

    m_isAlive = false;
}

/**
 * @brief �R�}���h�𑗐M���܂��B
 */
void ServerClient::sendCommand(shared_ptr<CommandPacket> command, bool isOutLog/*= true*/)
{
    if (command == nullptr) {
        throw std::invalid_argument("command");
    }

    std::string rsi = command->toRSI();
    if (rsi.empty()) {
        LOG_ERROR() << F("command type %1%: toRSI�Ɏ��s���܂����B")
            % command->getType();
    }

    m_rsiService->sendRSI(rsi, isOutLog);
}

/**
 * @brief �����R�}���h��RSI��M���ɌĂ΂�܂��B
 */
void ServerClient::onRSIReceived(std::string const & rsi)
{
    auto reply = ReplyPacket::parse(rsi);
    if (reply == nullptr) {
        LOG_ERROR() << rsi << ": RSI�R�}���h�̉��߂Ɏ��s���܂����B";
        return;
    }

    m_server->addReply(reply);
}

} // namespace server
} // namespace godwhale
