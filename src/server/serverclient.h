#ifndef GODWHALE_SERVER_CLIENT_H
#define GODWHALE_SERVER_CLIENT_H

#include "commandpacket.h"
#include "rsiservice.h"

namespace godwhale {
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
    bool isLogined() const {
        return m_logined;
    }

    /**
     * @brief �e�N���C�A���g�̎���ID���擾���܂��B
     */
    std::string const &getLoginId() const {
        return m_loginId;
    }

    /**
     * @brief �e�N���C�A���g�̎g�p�X���b�h�����擾���܂��B
     */
    int getThreadCount() const {
        return m_threadCount;
    }

    /**
     * @brief �N���C�A���g��PV�𑗂邩�ǂ������擾���܂��B
     */
    bool isSendPV() const {
        return m_sendPV;
    }    

    void close();
    void initialize(shared_ptr<tcp::socket> socket);
    void sendCommand(shared_ptr<CommandPacket> command, bool isOutLog = true);

private:
    virtual void onRSIReceived(std::string const & rsi);
    virtual void onDisconnected();

    int handleCommand(const std::string &command);

private:
    shared_ptr<Server> m_server;
    shared_ptr<RSIService> m_rsiService;

    bool m_isAlive;
    bool m_logined;
    std::string m_loginId;
    int m_threadCount;
    bool m_sendPV;
};

} // namespace server
} // namespace godwhale

#endif
