#ifndef GODWHALE_SERVER_CLIENT_H
#define GODWHALE_SERVER_CLIENT_H

#include "position.h"

namespace godwhale {
namespace server {

class Server;

/**
 * @brief �N���C�A���g�ւ̃R�}���h���M�Ɏg���܂��B
 */
class SendPacket
{
public:
    explicit SendPacket()
        : m_isOutLog(false) {
    }

    explicit SendPacket(const std::string &command, bool isOutLog)
        : m_command(command), m_isOutLog(isOutLog) {
    }

    /**
     * @brief �R�}���h���󂩂ǂ������擾���܂��B
     */
    bool IsEmpty() const {
        return m_command.empty();
    }

    /**
     * @brief �R�}���h���N���A���܂��B
     */
    void Clear() {
        m_command.clear();
    }

    /**
     * @brief �R�}���h���擾���܂��B
     */
    const std::string &GetCommand() const {
        return m_command;
    }

    /**
     * @brief �R�}���h���M�����O�ɋL�^���邩�ǂ������擾���܂��B
     */
    bool IsOutLog() const {
        return m_isOutLog;
    }

    /**
     * @brief �R�}���h�̋�؂�L����ǉ����܂��B
     */
    void AppendDelimiter() {
        if (m_command.empty() || m_command.back() != '\n') {
            m_command.append("\n");
        }
    }

private:
    std::string m_command;
    bool m_isOutLog;
};


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
    Move GetMove() const {
        ScopedLock locker(m_guard);
        return m_move;
    }

    /**
     * @brief �w���肪���邩�ǂ������擾���܂��B
     */
    bool HasMove() const {
        ScopedLock locker(m_guard);
        return (!m_move.isEmpty() || !m_playedMove.isEmpty());
    }

    /**
     * @brief ���̃N���C�A���g�̎v�l���J�n���ꂽ����擾���܂��B
     */
    Move GetPlayedMove() const {
        ScopedLock locker(m_guard);
        return m_playedMove;
    }

    /**
     * @brief ���ǖʂƂ͕ʂɁA�N���C�A���g�̎v�l���J�n���ꂽ�肪���邩�擾���܂��B
     */
    bool HasPlayedMove() const {
        ScopedLock locker(m_guard);
        return (m_playedMove != MOVE_NA);
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
    const std::vector<Move> &GetPVSeq() const {
        ScopedLock locker(m_guard);
        return m_pvseq;
    }

    /**
     * @brief ��������w������擾���܂��B
     */
    const std::vector<Move> &GetIgnoreMoves() const {
        ScopedLock locker(m_guard);
        return m_ignoreMoves;
    }

    /**
     * @brief �R�}���h�𑗐M���܂��B
     */
    void SendCommand(const format &fmt, bool isOutLog = true) {
        SendCommand(fmt.str(), isOutLog);
    }

    void Close();
    void BeginAsyncReceive();
    void SendCommand(const std::string &command, bool isOutLog = true);

    void InitGame();
    void ResetPosition(const min_posi_t *posi);
    void MakeRootMove(Move move, int pid, bool isActualMove=true);
    void SetPlayedMove(Move move);
    void AddIgnoreMove(Move move);

private:
    void Disconnected();
    
    void HandleAsyncReceive(const error_code &error);

    void PutSendPacket(const SendPacket &packet);
    SendPacket GetSendPacket();

    void BeginAsyncSend();
    void HandleAsyncSend(const error_code &error);

    int ParseCommand(const std::string &command);
    void SendInitGameInfo();
    std::string GetMoveHistory() const;

private:
    shared_ptr<Server> m_server;
    shared_ptr<tcp::socket> m_socket;
    mutable Mutex m_guard;

    asio::streambuf m_streambuf;
    char m_line[2048];
    int m_lineIndex;

    std::list<SendPacket> m_sendList;
    SendPacket m_sendingbuf;

    Position m_board;
    bool m_logined;
    std::string m_id;
    int m_nthreads;
    bool m_sendpv;

    int m_pid;
    Move m_move;
    Move m_playedMove;
    bool m_stable;
    bool m_final;
    long m_nodes;
    int m_value;
    std::vector<Move> m_pvseq;

    std::vector<Move> m_ignoreMoves;
};

} // namespace server
} // namespace godwhale

#endif
