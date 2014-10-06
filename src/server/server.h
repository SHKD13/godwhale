
#ifndef GODWHALE_SERVER_SERVER_H
#define GODWHALE_SERVER_SERVER_H

#include "position.h"
#include "client.h"

namespace godwhale {
namespace server {

/**
 * @brief �]���l��m�[�h���Ȃǂ�ێ����܂��B
 */
struct Score
{
public:
    explicit Score()
        : m_move(MOVE_NA), m_nodes(0), m_value(0) {
    }

    /**
     * @brief �w�����]���l��ݒ肵�܂��B
     */
    void Set(const shared_ptr<Client> &client) {
        m_move = (client->HasPlayedMove() ?
            client->GetPlayedMove() : client->GetMove());
        m_nodes = client->GetNodeCount();
        m_value = client->GetValue();
        m_pvseq = client->GetPVSeq();
    }

    /**
     * @brief �w���肪�ݒ肳�ꂽ���擾���܂��B
     */
    bool IsValid() const {
        return (m_move != MOVE_NA);
    }

    /**
     * @brief �w������擾���܂��B
     */
    Move GetMove() const {
        return m_move;
    }

    /**
     * @brief �T���m�[�h�����擾���܂��B
     */
    long GetNodes() const {
        return m_nodes;
    }

    /**
     * @brief �]���l���擾���܂��B
     */
    int GetValue() const {
        return m_value;
    }

    /**
     * @brief �w����ɕt������PV���擾���܂��B
     */
    const std::vector<Move> GetPVSeq() const {
        return m_pvseq;
    }

public:
    Move m_move;
    long m_nodes;
    int m_value;
    std::vector<Move> m_pvseq;
};


/**
 * @brief �升�_�N�W�������̃T�[�o�[�N���X�ł��B
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

public:
    ~Server();

    /**
     * @brief ���ǖʂ��擾���܂��B
     */
    const Position &GetBoard() const {
        ScopedLock locker(m_guard);
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

    void ClientLogined(shared_ptr<Client> client);
    std::vector<shared_ptr<Client> > GetClientList();
    
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

    void StartThread();
    void ServiceThreadMain();
    void UpdateInfo();

    void BeginAccept();
    void HandleAccept(shared_ptr<tcp::socket> socket,
                      const boost::system::error_code &error);

    void SendCurrentInfo(std::vector<shared_ptr<Client> > &clientList,
                         long nps);
    void SendPV(std::vector<shared_ptr<Client> > &clientList,
                int value, long nodes, const std::vector<Move> &pvseq);

private:
    mutable Mutex m_guard;
    boost::asio::io_service m_service;
    shared_ptr<boost::thread> m_thread;

    volatile bool m_isAlive;
    boost::asio::ip::tcp::acceptor m_acceptor;

    std::list<weak_ptr<Client> > m_clientList;
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
