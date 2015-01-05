#ifndef GODWHALE_SERVER_SERVERCLIENT_H
#define GODWHALE_SERVER_SERVERCLIENT_H

#include "rsiservice.h"

namespace godwhale {

class CommandPacket;
class ReplyPacket;

namespace server {

class Server;

/**
 * @brief ���X�i�[�o�b�ɂ��N���C�A���g���Ǘ����܂��B
 */
class ServerClient : public enable_shared_from_this<ServerClient>,
                     public IRSIListener,
                     private boost::noncopyable
{
public:
    explicit ServerClient(shared_ptr<Server> server);
    ~ServerClient();

    /**
     * @brief �N���C�A���g�̐ڑ��������Ă��邩�ǂ������擾���܂��B
     */
    bool isAlive() const
    {
        return m_isAlive;
    }

    /**
     * @brief ����Ƀ��O�C���������s��ꂽ���ǂ������擾���܂��B
     */
    bool isLogined() const
    {
        return m_logined;
    }

    /**
     * @brief �e�N���C�A���g�̃��O�C�������擾���܂��B
     */
    std::string const &getLoginName() const
    {
        return m_loginName;
    }

    /**
     * @brief �e�N���C�A���g�̎g�p�X���b�h�����擾���܂��B
     */
    int getThreadCount() const
    {
        return m_threadCount;
    }

    /**
     * @brief �N���C�A���g��PV�𑗂邩�ǂ������擾���܂��B
     */
    bool isSendPV() const
    {
        return m_sendPV;
    }    

    void initialize(shared_ptr<tcp::socket> socket);
    void close();

    void sendCommand(shared_ptr<CommandPacket> command, bool isOutLog = true);
    int proce();

private:
    virtual void onRSIReceived(std::string const & rsi);
    virtual void onDisconnected();

    void addReply(shared_ptr<ReplyPacket> reply);
    void removeReply(shared_ptr<ReplyPacket> reply);
    shared_ptr<ReplyPacket> getNextReply() const;

    int proce_Login(shared_ptr<ReplyPacket> reply);

private:
    shared_ptr<Server> m_server;
    shared_ptr<RSIService> m_rsiService;

    mutable Mutex m_replyGuard;
    std::list<shared_ptr<ReplyPacket>> m_replyList;

    bool m_isAlive;
    bool m_logined;
    std::string m_loginName;
    int m_threadCount;
    bool m_sendPV;
};

} // namespace server
} // namespace godwhale

#endif
