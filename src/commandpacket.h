#ifndef GODWHALE_COMMANDPACKET_H
#define GODWHALE_COMMANDPACKET_H

#include <boost/tokenizer.hpp>
#include "position.h"

namespace godwhale {

/**
 * @brief �R�}���h���ʎq�ł��B
 *
 * �R�}���h�Ƃ̓T�[�o�[����N���C�A���g�ɑ�����
 * �C���X�g���N�V�����ł��B
 */
enum CommandType
{
    /**
     * @brief ���ɂȂ�
     */
    COMMAND_NONE,
    /**
     * @brief ���[�g�ǖʂ�ݒ肵�܂��B
     */
    COMMAND_SETPOSITION,
    /**
     * @brief PV��ݒ肵�܂��B
     */
    COMMAND_SETPV,
    /**
     * @brief �S������w�����ݒ肵�܂��B
     */
    COMMAND_SETMOVELIST,
    /*COMMAND_SHRINK,
    COMMAND_EXTEND,*/
    /**
     * @brief �T�[�o�[�ƃN���C�A���g�̏�Ԃ���v���Ă��邩�m�F���܂��B
     */
    COMMAND_VERIFY,
    /**
     * �N���C�A���g���~���܂��B
     */
    COMMAND_STOP,
    /**
     * @brief �N���C�A���g���I�������܂��B
     */
    COMMAND_QUIT,
};

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
     * @brief ���l�̍X�V�̉\��������Ƃ��A�T�����Ԃ̒����ȂǂɎg���܂��B
     */
    REPLY_RETRYED,
    /**
     * @brief ���l�̍X�V���s��ꂽ�Ƃ��Ɏg���܂��B
     */
    REPLY_VALUEUPDATED,
};

/**
 * @brief �R�}���h�f�[�^���܂Ƃ߂Ĉ����܂��B
 */
class CommandPacket
{
private:
    typedef boost::char_separator<char> CharSeparator;
    typedef boost::tokenizer<CharSeparator> Tokenizer;

    static const CharSeparator ms_separator;

public:
    explicit CommandPacket(CommandType type);
    explicit CommandPacket(CommandPacket const & other);
    explicit CommandPacket(CommandPacket && other);

    /**
     * @brief �R�}���h�^�C�v���擾���܂��B
     */
    CommandType getType() const
    {
        return m_type;
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
     * @brief �ǖʂ��擾���܂��B
     */
    Position const &getPosition() const
    {
        return m_position;
    }

    /**
     * @brief �ǖʂ�ݒ肵�܂��B
     */
    void setPosition(Position const & position)
    {
        m_position = position;
    }

    /**
     * @brief �w����̃��X�g(��̈ꗗ��o�u�Ȃ�)���擾���܂��B
     */
    std::vector<Move> const &getMoveList() const
    {
        return m_moveList;
    }

    int getPriority() const;

    static shared_ptr<CommandPacket> parse(std::string const & rsi);
    std::string toRsi() const;

private:
    static bool isToken(std::string const & str, std::string const & target);

    // setposition
    static shared_ptr<CommandPacket> parse_SetPosition(std::string const & rsi,
                                                       Tokenizer & tokens);
    std::string toRsi_SetPosition() const;

    // setmovelist
    static shared_ptr<CommandPacket> parse_SetMoveList(std::string const & rsi,
                                                       Tokenizer & tokens);
    std::string toRsi_SetMoveList() const;

    // stop
    static shared_ptr<CommandPacket> parse_Stop(std::string const & rsi,
                                                Tokenizer & tokens);
    std::string toRsi_Stop() const;

    // quit
    static shared_ptr<CommandPacket> parse_Quit(std::string const & rsi,
                                                Tokenizer & tokens);
    std::string toRsi_Quit() const;

private:
    CommandType m_type;

    int m_positionId;
    int m_iterationDepth;
    int m_plyDepth;
    Position m_position;
    std::vector<Move> m_moveList;
};

} // namespace godwhale

#endif
