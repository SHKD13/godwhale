#include "precomp.h"
#include "stdinc.h"
#include "server.h"
#include "client.h"

namespace godwhale {
namespace server {

Client::Client(shared_ptr<Server> server, shared_ptr<tcp::socket> socket)
    : m_server(server), m_socket(socket), m_logined(false), m_nthreads(0)
    , m_sendpv(false), m_move(MOVE_NA), m_playedMove(MOVE_NA), m_stable(false)
    , m_final(false), m_nodes(0), m_value(0), m_pid(0)
{
    LOG(Notification) << m_board;
}

Client::~Client()
{
    Close();
}

void Client::Close()
{
    LOCK(m_guard) {
        m_sendList.clear();
    }

    if (m_socket->is_open()) {
        system::error_code error;
        m_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
    }
}

/**
 * @brief �R�l�N�V�����̐ؒf���ɌĂ΂�܂��B
 */
void Client::Disconnected()
{
    LOCK(m_guard) {
        m_sendList.clear();
    }

    if (m_socket->is_open()) {
        m_socket->close();
    }

    LOG(Notification) << "Client[" << m_id << "] ���ؒf����܂����B";
}

/**
 * @brief �R�}���h�̎�M�������J�n���܂��B
 */
void Client::BeginAsyncReceive()
{
    asio::async_read_until(
        *m_socket, m_streambuf, "\n",
        bind(&Client::HandleAsyncReceive, shared_from_this(),
             asio::placeholders::error));
}

/**
 * @brief �R�}���h��M��ɌĂ΂�܂��B
 */
void Client::HandleAsyncReceive(const system::error_code &error)
{
    if (error) {
        LOG(Error) << "command��M�Ɏ��s���܂����B(" << error.message() << ")";
        Disconnected();
        return;
    }

    std::istream is(&m_streambuf);
    std::string line;

    while (std::getline(is, line, '\n')) {
        LOG(Debug) << "client[" << m_id << "] recv: " << line;
        
        if (ParseCommand(line) < 0) {
            LOG(Error) << "parse error: " << line;
        }
    }

    BeginAsyncReceive();
}

/**
 * @brief �R�}���h���X�g�ɑ��M�p�̃R�}���h��ǉ����܂��B
 */
void Client::PutSendCommand(const std::string &command)
{
    if (command.empty()) {
        return;
    }

    LOCK(m_guard) {
        m_sendList.push_back(command);
    }
}

/**
 * @brief ���������M�̃R�}���h������΂�������X�g����폜���Ԃ��܂��B
 */
std::string Client::GetSendCommand()
{
    LOCK(m_guard) {
        if (m_sendList.empty()) {
            return std::string();
        }

        auto command = m_sendList.front();
        m_sendList.pop_front();
        return command;
    }
}

/**
 * @brief �R�}���h�𑗐M���܂��B
 */
void Client::SendCommand(const std::string &command)
{
    PutSendCommand(command);

    // ���̑��M�����Ɉڂ�܂��B
    BeginAsyncSend();
}

/**
 * @brief �񓯊��̑��M�������J�n���܂��B
 */
void Client::BeginAsyncSend()
{
    LOCK(m_guard) {
        if (!m_sendingbuf.empty()) {
            return;
        }

        m_sendingbuf = GetSendCommand();
        if (m_sendingbuf.empty()) {
            return;
        }

        asio::async_write(
            *m_socket, asio::buffer(m_sendingbuf),
            bind(&Client::HandleAsyncSend, shared_from_this(),
                 asio::placeholders::error));
    }
}

/**
 * @brief �񓯊����M�̊�����ɌĂ΂�܂��B
 */
void Client::HandleAsyncSend(const system::error_code &error)
{
    if (error) {
        LOG(Error) << "command���M�Ɏ��s���܂����B(" << error.message() << ")";
        Disconnected();
        return;
    }
    
    LOCK(m_guard) {
        LOG(Debug) << "client[" << m_id << "] send: " << m_sendingbuf;
        m_sendingbuf.clear();
    }

    // ���̑��M�����Ɉڂ�܂��B
    BeginAsyncSend();
}


const static regex PidRegex("pid=(\\d+)");
const static regex LoginRegex("^login (\\w+) (\\d+) (\\d+)?");

const static regex MoveRegex("move=(\\d\\d\\d\\d[A-Z][A-Z])");
const static regex ToryoRegex("move=%TORYO");
const static regex NodeRegex("n=(\\d+)");
const static regex ValueRegex("v=([+-]?\\d+)");
const static regex PVRegex("pv=\\s*(([+-]?\\d\\d\\d\\d[A-Z][A-Z]\\s*)+)");

int Client::ParseCommand(const std::string &command)
{
    if (command.empty()) {
        return 0;
    }

    smatch m;

    if (regex_search(command, m, LoginRegex)) {
        LOCK(m_guard) {
            m_logined = true;
            m_id = m.str(1);
            m_nthreads = lexical_cast<int>(m.str(2));
            m_sendpv = (lexical_cast<int>(m.str(3)) != 0);
            m_pid = 0;
        }

        SendCommand("init 0\n");
        LOG(Notification) << "client '" << m_id << "' is logined !";
        return 0;
    }

    // PID
    if (!regex_search(command, m, PidRegex)) {
        return -1;
    }

    int pid = lexical_cast<int>(m.str(1));
    if (pid >= 0 && pid != GetPid()) {
        return 0;
    }

    // �w����
    std::string moveStr;
    move_t move = MOVE_NA;
    if (regex_search(command, m, MoveRegex)) {
        moveStr = m.str(1);
        move = m_board.InterpretCsaMove(moveStr);
        assert(m_board.IsValidMove(move));
    }
    else if (regex_search(command, m, ToryoRegex)) {
        move = MOVE_RESIGN;
    }

    // �m�[�h��
    long nodes = -1;
    if (regex_search(command, m, NodeRegex)) {
        nodes = lexical_cast<long>(m.str(1));
    }

    // �]���l
    int value = INT_MAX;
    if (regex_search(command, m, ValueRegex)) {
        value = lexical_cast<int>(m.str(1));
    }

    // PV�m�[�h
    std::vector<move_t> pvseq;
    if (!moveStr.empty() && regex_search(command, m, PVRegex)) {
        std::vector<std::string> result;
        split(result, m.str(1), is_any_of("+- "));
        result.insert(result.begin(), moveStr);

        // �s�v�ȗv�f������
        result.erase(std::remove(result.begin(), result.end(), ""), result.end());
        pvseq = m_board.InterpretCsaMoveList(result.begin(), result.end());

        // ��ԍŏ��͎w����Ɠ����̂��ߍ폜
        pvseq.erase(pvseq.begin());
    }

    bool final = (command.find("final") >= 0);
    bool stable = (command.find("stable") >= 0);

    LOCK(m_guard) {
        // �O�̂��߂�����x�m�F����B
        if (pid != m_pid) return 0;

        if (move != MOVE_NA && nodes > 0 && value != INT_MAX) {
            m_move = move;
            m_nodes = nodes;
            m_value = value;
            m_final = final;
            m_stable = stable;
            m_pvseq = pvseq;
        }
        else if (final) m_final = final;
        else if (nodes > 0) m_nodes = nodes;
        else return -1;
    }

    return 0;
}

void Client::MakeRootMove(move_t move, int pid)
{
    assert(m_board.IsValidMove(move));

    if (move == MOVE_NA) return;

    LOCK(m_guard) {
        LOG(Notification) << m_board;
        m_board.Move(move);
        LOG(Notification) << m_board;

        m_pid = pid;
        m_move = move;
        m_nodes = 0;
        m_value = 0;
        m_final = false;
        m_stable = false;
    }

    auto fmt = format("move %1% %2%\n") % str_CSA_move(move) % pid;
    SendCommand(fmt.str());
}

}
}
