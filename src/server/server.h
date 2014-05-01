
#ifndef GODWHALE_SERVER_SERVER_H
#define GODWHALE_SERVER_SERVER_H

#include "board.h"

namespace godwhale {
namespace server {

class Client;

/**
 *
 */
class Server : public enable_shared_from_this<Server>
{
public:
    /**
     * @brief �������������s���܂��B
     */
    static void Initialize();

    /**
     * @brief �V���O���g���C���X�^���X���擾���܂��B
     */
    static shared_ptr<Server> GetInstance()
    {
        return ms_instance;
    }

private:
    static shared_ptr<Server> ms_instance;

    explicit Server();

    void StartThread();
    void ServiceThreadMain();

    void BeginAccept();
    void HandleAccept(shared_ptr<tcp::socket> socket,
                      const system::error_code &error);

public:
    ~Server();

    /**
     * @brief ���ǖʂ��擾���܂��B
     */
    const Board &GetBoard() const {
        ScopedLock locker(m_guard);
        return m_board;
    }

    /**
     * @brief ���݂̋ǖ�ID���擾���܂��B
     */
    int GetGid() const {
        return m_gid;
    }

    void ClientLogined(shared_ptr<Client> client);
    std::vector<shared_ptr<Client> > GetClientList();
    
    void InitGame(const min_posi_t *posi);
    void MakeRootMove(move_t move);
    void UnmakeRootMove();

    int Iterate(tree_t *restrict ptree, int *value, std::vector<move_t> &pvseq);
    bool IsEndIterate(tree_t *restrict ptree, timer::cpu_timer &timer);

private:
    mutable Mutex m_guard;
    asio::io_service m_service;
    shared_ptr<thread> m_thread;

    volatile bool m_isAlive;
    tcp::acceptor m_acceptor;

    std::list<weak_ptr<Client> > m_clientList;
    Board m_board;
    atomic<int> m_gid;
};

}
}

#endif
