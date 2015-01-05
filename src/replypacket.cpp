#include "precomp.h"
#include "stdinc.h"
#include "replypacket.h"

namespace godwhale {

const ReplyPacket::CharSeparator ReplyPacket::ms_separator(" ", "");

ReplyPacket::ReplyPacket(ReplyType type)
    : m_type(type), m_positionId(-1), m_iterationDepth(-1), m_plyDepth(-1)
{
}

/**
 * @brief �R�}���h�̎��s�D�揇�ʂ��擾���܂��B
 *
 * �l���傫�������A�D�揇�ʂ͍����ł��B
 */
int ReplyPacket::getPriority() const
{
    switch (m_type) {
    // �I���n�̃R�}���h�͂����Ɏ��s
    //case COMMAND_QUIT:
    //    return 100;

    // �ʏ�̃R�}���h�͂��̂܂܂̏���
    case REPLY_LOGIN:
        return 50;

    // �G���[�݂����Ȃ���
    case REPLY_NONE:
        return 0;
    }

    unreachable();
    return -1;
}

/**
 * @brief str��target�������g�[�N���ł��邩���ׂ܂��B
 */
bool ReplyPacket::isToken(std::string const & str, std::string const & target)
{
    return (str.compare(target) == 0);
}

/**
 * @brief RSI(remote shogi interface)���p�[�X���A�R�}���h�ɒ����܂��B
 */
shared_ptr<ReplyPacket> ReplyPacket::parse(std::string const & rsi)
{
    if (rsi.empty()) {
        throw new std::invalid_argument("rsi");
    }

    Tokenizer tokens(rsi, ms_separator);
    std::string token = *tokens.begin();

    if (isToken(token, "login")) {
        return parse_Login(rsi, tokens);
    }

    return shared_ptr<ReplyPacket>();
}

/**
 * @brief �R�}���h��RSI(remote shogi interface)�ɕϊ����܂��B
 */
std::string ReplyPacket::toRSI() const
{
    assert(m_type != REPLY_NONE);

    switch (m_type) {
    case REPLY_LOGIN:
        return toRSI_Login();
    }

    unreachable();
    return std::string();
}


#pragma region Login
/**
 * @brief login�R�}���h���p�[�X���܂��B
 *
 * login <address> <port> <login_id> <nthreads>
 */
shared_ptr<ReplyPacket> ReplyPacket::parse_Login(std::string const & rsi,
                                                     Tokenizer & tokens)
{
    shared_ptr<ReplyPacket> result(new ReplyPacket(REPLY_LOGIN));
    Tokenizer::iterator begin = ++tokens.begin();

    result->m_loginName = *begin++;
    result->m_threadSize = lexical_cast<int>(*begin++);
    return result;
}

/**
 * @brief login�R�}���h��RSI�ɕϊ����܂��B
 */
std::string ReplyPacket::toRSI_Login() const
{
    return (F("login %1% %2%")
        % m_loginName % m_threadSize)
        .str();
}
#pragma endregion

} // namespace godwhale
