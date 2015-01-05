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

    LOG_DEBUG() << "ServerClient[" << m_loginName << "]�͕����܂����B";
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
    LOG_NOTIFICATION() << "ServerClient[" << m_loginName << "] is disconnected.";

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

    // IO�Ƃ��̏����𕪗����邽�߁A��x���X�g�ɓ���܂��B
    addReply(reply);
}

/**
 * @brief �����R�}���h��ǉ����܂��B
 */
void ServerClient::addReply(shared_ptr<ReplyPacket> reply)
{
    if (reply == nullptr) {
        throw std::invalid_argument("reply");
    }

    LOCK (m_replyGuard) {
        m_replyList.push_back(reply);
    }
}

/**
 * @brief �����R�}���h���폜���܂��B
 */
void ServerClient::removeReply(shared_ptr<ReplyPacket> reply)
{
    LOCK (m_replyGuard) {
        m_replyList.remove(reply);
    }
}

/**
 * @brief ���ɏ������鉞���R�}���h���擾���܂��B
 */
shared_ptr<ReplyPacket> ServerClient::getNextReply() const
{
    LOCK (m_replyGuard) {
        if (m_replyList.empty()) {
            return shared_ptr<ReplyPacket>();
        }

        return m_replyList.front();
    }
}

/**
 * @brief �����R�}���h���������܂��B
 */
int ServerClient::proce()
{
    auto reply = getNextReply();
    if (reply == nullptr) {
        return 0;
    }

    removeReply(reply);

    switch (reply->getType()) {
    case REPLY_LOGIN:
        proce_Login(reply);
        break;
    }

    return 1;
}

/**
 * @brief login�������������܂��B
 *
 * login <name> <thread-num>
 */
int ServerClient::proce_Login(shared_ptr<ReplyPacket> reply)
{
    LOG_NOTIFICATION() << "handle login";

    m_loginName = reply->getLoginName();
    m_threadCount = reply->getThreadSize();
    return 0;
}

} // namespace server
} // namespace godwhale
