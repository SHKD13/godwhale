#ifndef GODWHALE_REPLYPACKET_H
#define GODWHALE_REPLYPACKET_H

#include <boost/tokenizer.hpp>
#include "position.h"

namespace godwhale {

/**
 * @brief �ԓ��R�}���h�̎��ʎq�ł��B
 *
 * �ԓ��R�}���h�Ƃ̓N���C�A���g����T�[�o�[�ɑ�����
 * �C���X�g���N�V�����ł��B
 */
enum ReplyType
{
    /**
     * @brief ���ɂȂ��B
     */
    REPLY_NONE,
    /**
     * @brief ���O�C���p�R�}���h
     *
     * login <name> <thread-num>
     */
    REPLY_LOGIN,
    /**
     * @brief ���l�̍X�V�̉\��������Ƃ��A�T�����Ԃ̒����ȂǂɎg���܂��B
     *
     * retryed <position_id> <itd> <pld> <move>
     */
    REPLY_RETRYED,
    /**
     * @brief ���l�̍X�V���s��ꂽ�Ƃ��Ɏg���܂��B
     *
     * valueupdated <position_id> <itd> <pld> <move> <value> <ule>
     */
    REPLY_VALUEUPDATED,
};

/**
 * @brief �ԓ��R�}���h�f�[�^�������܂��B
 */
class ReplyPacket : private boost::noncopyable
{
private:
    typedef boost::char_separator<char> CharSeparator;
    typedef boost::tokenizer<CharSeparator> Tokenizer;

    static const CharSeparator ms_separator;

public:
    explicit ReplyPacket(ReplyType type);

    /**
     * @brief �R�}���h�^�C�v���擾���܂��B
     */
    ReplyType getType() const
    {
        return m_type;
    }

    /**
     * @brief ���O�C���h�c���擾���܂��B
     */
    std::string getLoginId() const
    {
        return m_loginId;
    }

    int getThreadSize() const
    {
        return m_threadSize;
    }

    /**
     * @brief �ǖ�ID���擾���܂��B
     */
    int getPositionId() const
    {
        return m_positionId;
    }

    /**
     * @brief �ǖ�ID��ݒ肵�܂��B
     */
    void setPositionId(int id)
    {
        m_positionId = id;
    }

    /**
     * @brief �����[���̒T���[�����擾���܂��B
     */
    int getIterationDepth() const
    {
        return m_iterationDepth;
    }

    /**
     * @brief ���[�g�ǖʂ���v�l�ǖʂ܂ł̎�̐[�����擾���܂��B
     */
    int getPlyDepth() const
    {
        return m_plyDepth;
    }

    /**
     * @brief �w������擾���܂��B
     */
    Move getMove() const
    {
        return m_move;
    }

    /**
     * @brief �w����̃��X�g(��̈ꗗ��o�u�Ȃ�)���擾���܂��B
     */
    std::vector<Move> const &getPVList() const
    {
        return m_moveList;
    }    

public:
    int getPriority() const;

    static shared_ptr<ReplyPacket> parse(std::string const & rsi);
    std::string toRSI() const;

private:
    static bool isToken(std::string const & str, std::string const & target);

    // login
    static shared_ptr<ReplyPacket> parse_Login(std::string const & rsi,
                                               Tokenizer & tokens);
    std::string toRSI_Login() const;

private:
    ReplyType m_type;

    std::string m_loginId;
    int m_threadSize;

    int m_positionId;
    int m_iterationDepth;
    int m_plyDepth;

    Move m_move;
    std::vector<Move> m_moveList;
};

} // namespace godwhale

#endif
