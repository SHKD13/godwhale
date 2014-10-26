#ifndef GODWHALE_COMMANDIO_H
#define GODWHALE_COMMANDIO_H

namespace godwhale {

/**
 * @brief �R�}���h�̎�M�ȂǂɎg���郊�X�i�[�ł��B
 */
class IRSIListener
{
public:
    /**
     * @brief �R�}���h��M���ɌĂ΂�܂��B
     */
    virtual void onRSIReceived(std::string const & rsi) = 0;

    /**
     * @brief �ڑ����ؒf���ꂽ�Ƃ��ɌĂ΂�܂��B
     */
    virtual void onDisconnected() = 0;
};


/**
 * @brief �f�[�^���M�p�̃N���X�ł��B(private�N���X)
 */
class SendData
{
public:
    explicit SendData();
    SendData(std::string const & command, bool isOutLog);
    SendData(SendData const & other);
    SendData(SendData && other);

    SendData &operator =(SendData const & other);
    SendData &operator =(SendData && other);

    /**
     * @brief �R�}���h��������擾���܂��B
     */
    std::string const &getCommand() const
    {
        return m_command;
    }

    /**
     * @brief ���O�\�����邩�ǂ����̃t���O���擾���܂��B
     */
    bool isOutLog() const
    {
        return m_isOutLog;
    }

    /**
     * @brief �R�}���h���󂩂ǂ������ׂ܂��B
     */
    bool isEmpty() const
    {
        return m_command.empty();
    }

    /**
     * @brief �R�}���h�����������܂��B
     */
    void clear()
    {
        m_command = "";
    }

    /**
     * @brief �R�}���h�̋�؂�L����ǉ����܂��B
     */
    void appendDelimiter()
    {
        if (m_command.empty() || m_command.back() != '\n') {
            m_command.append("\n");
        }
    }

private:
    std::string m_command;
    bool m_isOutLog;
};


/**
 * @brief RSI(remote shogi protocol)�̑���M���s���N���X�ł��B
 */
class RSIService : public enable_shared_from_this<RSIService>
{
public:
    explicit RSIService(weak_ptr<IRSIListener> listener);
    virtual ~RSIService();

    /**
     * @brief �\�P�b�g���ڑ��������ׂ܂��B
     */
    bool isOpened() const
    {
        auto socket = m_socket;
        return (socket != nullptr && socket->is_open());
    }

    /**
     * @brief �R�}���h�𑗐M���܂��B
     */
    void sendRSI(boost::format const & fmt, bool isOutLog = true)
    {
        sendRSI(fmt.str(), isOutLog);
    }

    void startReceive(shared_ptr<tcp::socket> socket);
    void close();
    void sendRSI(std::string const & rsi, bool isOutLog = true);

private:
    void onDisconnected();

    void startReceive();
    void handleAsyncReceive(boost::system::error_code const & error);

    void putSendPacket(SendData const & packet);
    SendData getSendPacket();

    void beginAsyncSend();
    void handleAsyncSend(boost::system::error_code const & error);

private:
    mutable Mutex m_guard;
    shared_ptr<tcp::socket> m_socket;
    weak_ptr<IRSIListener> m_listener;

    volatile bool m_isShutdown;

    boost::asio::streambuf m_streamBuf;
    char m_line[2048];
    int m_lineIndex;

    std::list<SendData> m_sendList;
    SendData m_sendingBuf;
};

} // namespace godwhale

#endif
