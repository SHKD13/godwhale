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
    explicit Client(shared_ptr<Server> server, shared_ptr<tcp::socket> socket);
    ~Client();

    /**
     * @brief ���b�N�p�̃I�u�W�F�N�g���擾���܂��B
     */
    Mutex &GetGuard() const {
        return m_guard;
    }

    /**
     * @brief ����Ƀ��O�C���������s��ꂽ���ǂ������擾���܂��B
     */
    bool IsLogined() const {
        ScopedLock locker(m_guard);
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
     * @brief �v�l���̋ǖ�ID���擾���܂��B
     */
    int GetPid() const {
        ScopedLock locker(m_guard);
        return m_pid;
    }

    /**
     * @brief ���̎w������擾���܂��B
     */
    move_t GetMove() const {
        ScopedLock locker(m_guard);
        return m_move;
    }

    /**
     * @brief ���̃N���C�A���g�̎v�l���J�n���ꂽ����擾���܂��B
     */
    move_t GetPlayedMove() const {
        ScopedLock locker(m_guard);
        return m_playedMove;
    }

    /**
     * @brief �e�N���C�A���g���[���Œ�ŒT�����Ă��邩�擾���܂��B
     */
    bool IsStable() const {
        ScopedLock locker(m_guard);
        return m_stable;
    }

    /**
     * @brief �]���l�̊m�肵����(��ՁA�l�݂Ȃ�)���ǂ������擾���܂��B
     */
    bool IsFinal() const {
        ScopedLock locker(m_guard);
        return m_final;
    }

    /**
     * @brief �T���m�[�h�����擾���܂��B
     */
    long GetNodeCount() const {
        ScopedLock locker(m_guard);
        return m_nodes;
    }

    /**
     * @brief �w����̕]���l���擾���܂��B
     */
    int GetValue() const {
        ScopedLock locker(m_guard);
        return m_value;
    }

    /**
     * @brief PV�m�[�h���擾���܂��B
     */
    const std::vector<move_t> &GetPVSeq() const {
        ScopedLock locker(m_guard);
        return m_pvseq;
    }

    /**
     * @brief ��������w������擾���܂��B
     */
    const std::vector<move_t> &GetIgnoreMoves() const {
        ScopedLock locker(m_guard);
        return m_ignoreMoves;
    }

    void Close();
    void BeginAsyncReceive();
    void SendCommand(const std::string &command);

    void MakeRootMove(move_t move, int pid);

private:
    void Disconnected();
    
    void HandleAsyncReceive(const error_code &error);

    void PutSendCommand(const std::string &command);
    std::string GetSendCommand();

    void BeginAsyncSend();
    void HandleAsyncSend(const error_code &error);

    int ParseCommand(const std::string &command);

private:
    shared_ptr<Server> m_server;
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

    int m_pid;
    move_t m_move;
    move_t m_playedMove;
    bool m_stable;
    bool m_final;
    long m_nodes;
    int m_value;
    std::vector<move_t> m_pvseq;

    std::vector<move_t> m_ignoreMoves;
};

} // namespace server
} // namespace godwhale

#endif
