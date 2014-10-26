
#ifndef GODWHALE_SERVER_SERVER_H
#define GODWHALE_SERVER_SERVER_H

#include "position.h"
#include "serverclient.h"

namespace godwhale {
namespace server {

class Client;

/**
 * @brief �升�_�N�W�������̃T�[�o�[�N���X�ł��B
 */
class Server : public enable_shared_from_this<Server>
{
public:
    /**
     * @brief �������������s���܂��B
     */
    static void initialize();

    /**
     * @brief �V���O���g���C���X�^���X���擾���܂��B
     */
    static shared_ptr<Server> get()
    {
        return ms_instance;
    }

public:
    ~Server();

    /**
     * @brief ���ǖʂ��擾���܂��B
     */
    const Position &GetBoard() const {
        return m_board;
    }

    /**
     * @brief ���݂̋ǖ�ID���擾���܂��B
     */
    int GetGid() const {
        return m_gid;
    }

    /**
     * @brief ���݁A�΋ǒ����ǂ������擾���܂��B
     */
    bool IsPlaying() const {
        return m_isPlaying;
    }

    void clientLogined(shared_ptr<ServerClient> client);
    std::vector<shared_ptr<ServerClient>> getClientList();
    
    void InitGame();
    void QuitGame();
    void ResetPosition(const min_posi_t *posi);
    void MakeRootMove(Move move);
    void UnmakeRootMove();
    void AdjustTimeHook(int turn);

    int Iterate(tree_t *restrict ptree, int *value, std::vector<move_t> &pvseq);
    bool IsEndIterate(tree_t *restrict ptree, boost::timer::cpu_timer &timer);

private:
    static shared_ptr<Server> ms_instance;

    explicit Server();

    void startThread();
    void serviceThreadMain();

    void beginAccept();
    void handleAccept(shared_ptr<tcp::socket> socket,
                      const boost::system::error_code &error);

private:
    mutable Mutex m_guard;
    boost::asio::io_service m_service;
    shared_ptr<boost::thread> m_thread;

    volatile bool m_isAlive;
    boost::asio::ip::tcp::acceptor m_acceptor;

    std::list<shared_ptr<ServerClient>> m_clientList;
    Position m_board;
    boost::atomic<int> m_gid;
    bool m_isPlaying;

    boost::timer::cpu_timer m_turnTimer;
    boost::timer::cpu_timer m_sendTimer;
    int m_currentValue;
};

}
}

#endif
