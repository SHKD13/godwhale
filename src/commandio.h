#ifndef GODWHALE_COMMANDIO_H
#define GODWHALE_COMMANDIO_H

namespace godwhale {

/**
 * @brief �R�}���h�̎�M�ȂǂɎg���郊�X�i�[�ł��B
 */
class ICommandListener
{
public:
    /**
     * @brief �R�}���h��M���ɌĂ΂�܂��B
     */
    virtual void OnCommandReceived(std::string const & command)
    {
    }

    /**
     * @brief �ڑ����ؒf���ꂽ�Ƃ��ɌĂ΂�܂��B
     */
    virtual void OnDisconnected()
    {
    }
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
     * @brief �R�}���h�̋�؂�L����ǉ����܂��B
     */
    bool isEmpty() const
    {
        return m_command.empty();
    }

    /**
     * @brief �R�}���h�̋�؂�L����ǉ����܂��B
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
 * @brief ���X�i�[�o�b�ɂ��N���C�A���g���Ǘ����܂��B
 */
class CommandIO : public enable_shared_from_this<CommandIO>
{
public:
    explicit CommandIO(shared_ptr<tcp::socket> socket,
                       shared_ptr<ICommandListener> listener);
    ~CommandIO();

    /**
     * @brief �R�}���h�𑗐M���܂��B
     */
    void SendCommand(boost::format const & fmt, bool isOutLog = true)
    {
        SendCommand(fmt.str(), isOutLog);
    }

    void Close();
    void StartReceive();
    void SendCommand(std::string const & command, bool isOutLog = true);

private:
    void Disconnected();
    
    void HandleAsyncReceive(const boost::system::error_code &error);

    void PutSendPacket(SendData const & packet);
    SendData GetSendPacket();

    void BeginAsyncSend();
    void HandleAsyncSend(const boost::system::error_code &error);

private:
    shared_ptr<ICommandListener> m_listener;
    shared_ptr<tcp::socket> m_socket;
    mutable Mutex m_guard;

    boost::asio::streambuf m_streamBuf;
    char m_line[2048];
    int m_lineIndex;

    std::list<SendData> m_sendList;
    SendData m_sendingBuf;
};

} // namespace godwhale

#endif
