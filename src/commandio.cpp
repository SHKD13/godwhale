#include "precomp.h"
#include "stdinc.h"
#include "commandio.h"

namespace godwhale {

using namespace boost;

SendData::SendData()
{
}

SendData::SendData(std::string const & command, bool isOutLog)
{
    m_command = command;
    m_isOutLog = isOutLog;
}

SendData::SendData(SendData const & other)
    : m_command(other.m_command), m_isOutLog(other.m_isOutLog)
{
}

SendData::SendData(SendData && other)
    : m_command(std::move(other.m_command)), m_isOutLog(other.m_isOutLog)
{
}

SendData &SendData::operator =(SendData const & other)
{
    if (this != &other) {
        m_command = std::move(other.m_command);
        m_isOutLog = other.m_isOutLog;
    }

    return *this;
}

SendData &SendData::operator =(SendData && other)
{
    if (this != &other) {
        m_command = std::move(other.m_command);
        m_isOutLog = other.m_isOutLog;
    }

    return *this;
}


/////////////////////////////////////////////////////////////////////
CommandIO::CommandIO(shared_ptr<tcp::socket> socket,
                     shared_ptr<ICommandListener> listener)
    : m_listener(listener), m_socket(socket), m_lineIndex(0)
{
    memset(m_line, 0, sizeof(m_line));
}

CommandIO::~CommandIO()
{
    Close();
}

void CommandIO::Close()
{
    LOCK (m_guard) {
        m_sendList.clear();
    }

    if (m_socket->is_open()) {
        system::error_code error;
        m_socket->shutdown(tcp::socket::shutdown_both, error);
    }
}

/**
 * @brief �R�l�N�V�����̐ؒf���ɌĂ΂�܂��B
 */
void CommandIO::Disconnected()
{
    if (m_listener != nullptr) {
        m_listener->OnDisconnected();
    }

    LOCK (m_guard) {
        m_sendList.clear();

        if (m_socket != nullptr && m_socket->is_open()) {
            m_socket->close();
            m_socket = nullptr;
        }
    }
}


/**
 * @brief �R�}���h�̎�M�������J�n���܂��B
 */
void CommandIO::StartReceive()
{
    asio::async_read_until(
        *m_socket, m_streamBuf, "\n",
        bind(&CommandIO::HandleAsyncReceive, shared_from_this(),
             asio::placeholders::error));
}

/**
 * @brief �R�}���h��M��ɌĂ΂�܂��B
 */
void CommandIO::HandleAsyncReceive(const system::error_code &error)
{
    if (error) {
        LOG_ERROR() << error << ": �ʐM�G���[���������܂����B";
        Disconnected();
        return;
    }

    while (m_streamBuf.in_avail()) {
        char ch = m_streamBuf.sbumpc();

        // '\n'���ƂɃR�}���h����؂�܂��B
        if (ch == '\n') {
            std::string line(&m_line[0], &m_line[m_lineIndex]);
            m_lineIndex = 0;

            if (m_listener != nullptr) {
                m_listener->OnCommandReceived(line);
            }
            break;
        }

        m_line[m_lineIndex++] = ch;
    }

    StartReceive();
}


/**
 * @brief �R�}���h���X�g�ɑ��M�p�̃R�}���h��ǉ����܂��B
 */
void CommandIO::PutSendPacket(SendData const & packet)
{
    if (packet.isEmpty()) {
        return;
    }

    LOCK (m_guard) {
        m_sendList.push_back(packet);
    }
}

/**
 * @brief ���������M�̃R�}���h������΂�������X�g����폜���Ԃ��܂��B
 */
SendData CommandIO::GetSendPacket()
{
    LOCK (m_guard) {
        if (m_sendList.empty()) {
            return SendData();
        }

        auto packet = m_sendList.front();
        m_sendList.pop_front();
        return packet;
    }
}

/**
 * @brief �R�}���h�𑗐M���܂��B
 */
void CommandIO::SendCommand(std::string const & command, bool isOutLog/*=true*/)
{
    PutSendPacket(SendData(command, isOutLog));

    // ���̑��M�����Ɉڂ�܂��B
    BeginAsyncSend();
}

/**
 * @brief �񓯊��̑��M�������J�n���܂��B
 */
void CommandIO::BeginAsyncSend()
{
    LOCK(m_guard) {
        if (!m_sendingBuf.isEmpty()) {
            return;
        }

        m_sendingBuf = GetSendPacket();
        if (m_sendingBuf.isEmpty()) {
            return;
        }

        if (m_sendingBuf.isOutLog()) {
            LOG_DEBUG() << "CommandIO send: " << m_sendingBuf.getCommand();
        }

        m_sendingBuf.appendDelimiter();
        asio::async_write(
            *m_socket, asio::buffer(m_sendingBuf.getCommand()),
            bind(&CommandIO::HandleAsyncSend, shared_from_this(),
                 asio::placeholders::error));
    }
}

/**
 * @brief �񓯊����M�̊�����ɌĂ΂�܂��B
 */
void CommandIO::HandleAsyncSend(const system::error_code &error)
{
    if (error) {
        LOG_ERROR() << "command���M�Ɏ��s���܂����B(" << error.message() << ")";
        Disconnected();
        return;
    }

    LOCK(m_guard) {
        m_sendingBuf.clear();
    }

    // ���̑��M�����Ɉڂ�܂��B
    BeginAsyncSend();
}

} // namespace godwhale
