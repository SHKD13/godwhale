
#ifndef GODWHALE_SERVER_CLIENT_H
#define GODWHALE_SERVER_CLIENT_H

#include "board.h"

namespace godwhale {
namespace server {

class Server;

/**
 * @brief ���X�i�[�o�b�ɂ��N���C�A���g���Ǘ����܂��B
 */
class Client : public enable_shared_from_this<Client>
{
public:
    explicit Client(Server *server, shared_ptr<tcp::socket> socket);
    ~Client();

    /**
     * @brief ����Ƀ��O�C���������s��ꂽ���ǂ������擾���܂��B
     */
    bool IsLogined() const {
        return m_logined;
    }

    /**
     * @brief �e�N���C�A���g�̎���ID���擾���܂��B
     */
    const std::string &GetId() const {
        return m_id;
    }

    /**
     * @brief �e�N���C�A���g�̎g�p�X���b�h�����擾���܂��B
     */
    int GetThreadCount() const {
        return m_nthreads;
    }

    /**
     * @brief �N���C�A���g��PV�𑗂邩�ǂ������擾���܂��B
     */
    bool IsSendPV() const {
        return m_sendpv;
    }

    /**
     * @brief ���̎w������擾���܂��B
     */
    move_t GetMove() const {
        return m_move;
    }

    /**
     * @brief ���̃N���C�A���g�̎v�l���J�n���ꂽ����擾���܂��B
     */
    move_t GetPlayedMove() const {
        return m_playedMove;
    }

    /**
     * @brief �e�N���C�A���g���[���Œ�ŒT�����Ă��邩�擾���܂��B
     */
    bool IsStable() const {
        return m_stable;
    }

    /**
     * @brief �]���l�̊m�肵����(��ՁA�l�݂Ȃ�)���ǂ������擾���܂��B
     */
    bool IsFinal() const {
        return m_final;
    }

    /**
     * @brief �T���m�[�h�����擾���܂��B
     */
    long GetNodeCount() const {
        return m_nodes;
    }

    /**
     * @brief �w����̕]���l���擾���܂��B
     */
    int GetValue() const {
        return m_value;
    }

    /**
     * @brief �v�l���̋ǖ�ID���擾���܂��B
     */
    int GetPid() const {
        return m_pid;
    }

    void BeginAsyncReceive();
    void SendCommand(const std::string &command);

    void MakeRootMove(move_t move, int pid);

private:
    void Disconnected(bool destructor=false);
    
    void HandleAsyncReceive(const system::error_code &error);

    void PutSendCommand(const std::string &command);
    std::string GetSendCommand();

    void BeginAsyncSend();
    void HandleAsyncSend(const system::error_code &error);

    int ParseCommand(const std::string &command);

private:
    Server *m_server;
    shared_ptr<tcp::socket> m_socket;
    mutable Mutex m_guard;
    asio::streambuf m_streambuf;

    std::list<std::string> m_sendList;
    std::string m_sendingbuf;

    Board m_board;
    bool m_logined;
    std::string m_id;
    int m_nthreads;
    bool m_sendpv;

    move_t m_move;
    move_t m_playedMove;
    bool m_stable;
    bool m_final;
    long m_nodes;
    int m_value;
    int m_pid;
};

} // namespace server
} // namespace godwhale

#endif
