#include "precomp.h"
#include "stdinc.h"
#include "rsiservice.h"

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
        m_command = other.m_command;
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
RSIService::RSIService(weak_ptr<IRSIListener> listener)
    : m_listener(listener), m_isShutdown(false), m_lineIndex(0)
{
    memset(m_line, 0, sizeof(m_line));
}

RSIService::~RSIService()
{
    close();
}
/**
 * @brief �ڑ��p�̃\�P�b�g��ݒ肵�܂��B
 */
void RSIService::startReceive(shared_ptr<tcp::socket> socket)
{
    close();

    // shared_ptr�̓X���b�h�Z�[�t
    m_socket = socket;

    // ��M�������J�n���܂��B
    if (socket != nullptr && socket->is_open()) {
        startReceive();
    }
}

/**
 * @brief �ڑ�����܂��B
 */
void RSIService::close()
{
    LOCK (m_guard) {
        m_sendList.clear();
    }

    auto socket = m_socket;
    if (socket != nullptr && socket->is_open()) {
        system::error_code error;
        socket->shutdown(tcp::socket::shutdown_both, error);

        m_isShutdown = true;
    }
}

/**
 * @brief �R�l�N�V�����̐ؒf���ɌĂ΂�܂��B
 */
void RSIService::onDisconnected()
{
    shared_ptr<IRSIListener> listener = m_listener.lock();
    if (listener != nullptr) {
        listener->onDisconnected();
    }

    LOCK (m_guard) {
        m_sendList.clear();
    }

    auto socket = m_socket;
    if (socket != nullptr && socket->is_open()) {
        system::error_code error;
        socket->close(error);

        m_socket = nullptr;
    }
}

/**
 * @brief �R�}���h�̎�M�������J�n���܂��B
 */
void RSIService::startReceive()
{
    auto socket = m_socket;
    if (socket == nullptr || !socket->is_open()) {
        LOG_ERROR() << "RSI�̎�M�J�n�Ɏ��s���܂����B";
        return;
    }

    asio::async_read_until(
        *socket, m_streamBuf, "\n",
        bind(&RSIService::handleAsyncReceive, shared_from_this(),
             asio::placeholders::error));
}

/**
 * @brief �R�}���h��M��ɌĂ΂�܂��B
 */
void RSIService::handleAsyncReceive(system::error_code const & error)
{
    if (error) {
        if (m_isShutdown) {
            LOG_WARNING() << "�\�P�b�g�͂��łɕ����Ă��܂��B";
        }
        else {
            LOG_ERROR() << error << ": �ʐM�G���[���������܂����B"
                        << "(" << error.message() << ")";
        }

        onDisconnected();
        return;
    }

    std::list<std::string> rsiList;

    LOCK (m_guard) {
        while (m_streamBuf.in_avail()) {
            char ch = m_streamBuf.sbumpc();

            // '\n'���ƂɃR�}���h����؂�܂��B
            if (ch == '\n') {
                std::string line(&m_line[0], &m_line[m_lineIndex]);

                LOG_DEBUG() << "command recv: " << line;

                rsiList.push_back(line);
                m_lineIndex = 0;
            }
            else {
                m_line[m_lineIndex++] = ch;
            }
        }
    }

    // lock�O�ŊeRSI���������܂��B
    shared_ptr<IRSIListener> listener = m_listener.lock();
    if (listener != nullptr) {
        BOOST_FOREACH (auto rsi, rsiList) {
            listener->onRSIReceived(rsi);
        }
    }

    startReceive();
}


/**
 * @brief �R�}���h���X�g�ɑ��M�p�̃R�}���h��ǉ����܂��B
 */
void RSIService::putSendPacket(SendData const & packet)
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
SendData RSIService::getSendPacket()
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
void RSIService::sendRSI(std::string const & rsi, bool isOutLog/*=true*/)
{
    putSendPacket(SendData(rsi, isOutLog));

    // ���̑��M�����Ɉڂ�܂��B
    beginAsyncSend();
}

/**
 * @brief �񓯊��̑��M�������J�n���܂��B
 */
void RSIService::beginAsyncSend()
{
    auto socket = m_socket;

    LOCK (m_guard) {
        if (socket == nullptr || !socket->is_open()) {
            throw std::logic_error("�\�P�b�g���ڑ�����Ă��܂���B");
        }

        if (m_isShutdown) {
            throw std::logic_error("�\�P�b�g�̐ؒf���ł��B");
        }

        // �f�[�^���M���̏ꍇ
        if (!m_sendingBuf.isEmpty()) {
            return;
        }

        m_sendingBuf = getSendPacket();
        if (m_sendingBuf.isEmpty()) {
            return;
        }

        if (m_sendingBuf.isOutLog()) {
            LOG_DEBUG() << "send: " << m_sendingBuf.getCommand();
        }

        // �f���~�^('\n')��t�����܂��B
        m_sendingBuf.appendDelimiter();
    }

    // �R�[���o�b�N�������ɌĂ΂�邱�Ƃ����邽�߁A
    // ���b�N������������ԂŌĂт܂��B
    asio::async_write(
        *socket, asio::buffer(m_sendingBuf.getCommand()),
        bind(&RSIService::handleAsyncSend, shared_from_this(),
             asio::placeholders::error));
}

/**
 * @brief �񓯊����M�̊�����ɌĂ΂�܂��B
 */
void RSIService::handleAsyncSend(system::error_code const & error)
{
    if (error) {
        LOG_ERROR() << "command���M�Ɏ��s���܂����B(" << error.message() << ")";
        onDisconnected();
        return;
    }

    LOCK (m_guard) {
        m_sendingBuf.clear();
    }

    // ���̑��M�����Ɉڂ�܂��B
    beginAsyncSend();
}

} // namespace godwhale
