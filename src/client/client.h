#ifndef GODWHALE_CLIENT_CLIENT_H
#define GODWHALE_CLIENT_CLIENT_H

#include "rsiservice.h" // IRSIListener
#include "replypacket.h"
#include "position.h"

namespace godwhale {

class CommandPacket;

namespace client {

/**
 * @brief �N���C�A���g�Ɋ��蓖�Ă�ꂽ�T���^�X�N���Ǘ����܂��B
 */
struct SearchTask
{
    explicit SearchTask()
        : iterationDepth(-1), plyDepth(0), move(0)
    {
    }

    explicit SearchTask(int itd, int pld, Move mv)
        : iterationDepth(itd), plyDepth(pld), move(mv)
    {
    }

    bool isIdle() const
    {
        return (iterationDepth < 0);
    }

    int getSearchDepth() const
    {
        return 1;
    }

    int iterationDepth;
    int plyDepth;
    Move move;
};


/**
 * @brief �N���C�A���g�̎��s�E�Ǘ����s���܂��B
 *
 * �V���O���g���N���X�ł��B
 */
class Client : public enable_shared_from_this<Client>,
               public IRSIListener,
               private boost::noncopyable
{
public:
    /**
     * @brief �������������s���܂��B
     */
    static int initializeInstance(int argc, char *argv[]);

    /**
     * @brief �V���O���g���C���X�^���X���擾���܂��B
     */
    static shared_ptr<Client> get()
    {
        return ms_instance;
    }

public:
    ~Client();

    /**
     * @brief ����Ƀ��O�C���������s��ꂽ���ǂ������擾���܂��B
     */
    bool isLogined() const
    {
        return m_logined;
    }

    /**
     * @brief �N���C�A���g�̃��O�C�������擾���܂��B
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
        return m_nthreads;
    }

    /**
     * @brief �v�l���̋ǖ�ID���擾���܂��B
     */
    int getPositionId() const
    {
        return m_positionId;
    }

    /**
     * @brief �T���m�[�h�����擾���܂��B
     */
    long getNodeCount() const
    {
        return m_nodes;
    }

    Position getPositionFromId(int id) const;
    void setPositionWithId(Position const & position, int id);

    void close();
    void sendReply(shared_ptr<ReplyPacket> reply, bool isOutLog = true);

    void addCommand(shared_ptr<CommandPacket> command);
    void addCommandFromRSI(std::string const & rsi);

    int mainloop();

private:
    static shared_ptr<Client> ms_instance;

    explicit Client();
    void initialize();
    void serviceThreadMain();

    virtual void onDisconnected();
    virtual void onRSIReceived(std::string const & command);

    void removeCommand(shared_ptr<CommandPacket> command);
    shared_ptr<CommandPacket> getNextCommand() const;

private:
    /* client_mainloop.cpp */

private:
    /* client_proce.cpp */
    void connect(std::string const & address, std::string const & port);
    void login(std::string const & loginName);

    int proce(bool searching);
    int proce_Login(shared_ptr<CommandPacket> command);
    int proce_SetPosition(shared_ptr<CommandPacket> command);
    int proce_MakeMoveRoot(shared_ptr<CommandPacket> command);
    int proce_SetMoveList(shared_ptr<CommandPacket> command);

private:
    boost::asio::io_service m_service;
    shared_ptr<RSIService> m_rsiService;

    shared_ptr<boost::thread> m_serviceThread;
    volatile bool m_isAlive;

    mutable Mutex m_positionGuard;
    std::map<int, Position> m_positionMap;

    mutable Mutex m_commandGuard;
    std::list<shared_ptr<CommandPacket>> m_commandList;

    bool m_logined;
    std::string m_loginName;
    int m_nthreads;

    int m_positionId; // ���݂̋ǖ�ID�ł��B
    long m_nodes;
};

} // namespace client
} // namespace godwhale

#endif
