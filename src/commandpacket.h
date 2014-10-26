#ifndef GODWHALE_COMMANDPACKET_H
#define GODWHALE_COMMANDPACKET_H

#include <boost/tokenizer.hpp>
#include "position.h"

namespace godwhale {

/**
 * @brief �R�}���h���ʎq�ł��B
 *
 * �R�}���h�Ƃ�
 * 1, �T�[�o�[����N���C�A���g�ɑ���ꂽ
 * 2, stdin�����M���ꂽ
 * ���߃C���X�g���N�V�����ł��B
 */
enum CommandType
{
    /**
     * @brief ���ɂȂ�
     */
    COMMAND_NONE,
    /**
     * @brief �T�[�o�[�Ƀ��O�C���������˗����܂��B
     */
    COMMAND_LOGIN,
    /**
     * @brief ���O�C�������̌��ʂ����炢�܂��B
     */
    COMMAND_LOGINRESULT,
    /**
     * @brief ���[�g�ǖʂ�ݒ肵�܂��B
     */
    COMMAND_SETPOSITION,
    /**
     * @brief ���[�g�ǖʂ���w�������i�߂܂��B
     */
    COMMAND_MAKEROOTMOVE,
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
     * @brief �X�V�O�̋ǖ�ID���擾���܂��B
     */
    int getOldPositionId() const
    {
        return m_oldPositionId;
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
     * @brief �w������擾���܂��B
     */
    Move getMove() const
    {
        return m_move;
    }

    /**
     * @brief �w����̃��X�g(��̈ꗗ��o�u�Ȃ�)���擾���܂��B
     */
    std::vector<Move> const &getMoveList() const
    {
        return m_moveList;
    }

public:
    /* for Login */

    /**
     * @brief �ڑ���̃T�[�o�[�A�h���X���擾���܂��B
     */
    std::string getServerAddress() const
    {
        return m_serverAddress;
    }

    /**
     * @brief �ڑ���̃T�[�o�[�|�[�g���擾���܂��B
     */
    std::string getServerPort() const
    {
        return m_serverPort;
    }

    /**
     * @brief ���O�C���h�c���擾���܂��B
     */
    std::string getLoginId() const
    {
        return m_loginId;
    }

    /**
     * @brief ���O�C�����ʂ��擾���܂��B
     */
    /*int getLoginResult() const
    {
        return m_loginResult;
    }*/

public:
    int getPriority() const;

    static shared_ptr<CommandPacket> parse(std::string const & rsi);
    std::string toRSI() const;

private:
    static bool isToken(std::string const & str, std::string const & target);

    // login
    static shared_ptr<CommandPacket> parse_Login(std::string const & rsi,
                                                 Tokenizer & tokens);
    std::string toRSI_Login() const;

    // loginresult
    /*static shared_ptr<CommandPacket> parse_Login(std::string const & rsi,
                                                 Tokenizer & tokens);
    std::string toRSI_Login() const;*/

    // setposition
    static shared_ptr<CommandPacket> parse_SetPosition(std::string const & rsi,
                                                       Tokenizer & tokens);
    std::string toRSI_SetPosition() const;

    // makerootmove
    static shared_ptr<CommandPacket> parse_MakeRootMove(std::string const & rsi,
                                                        Tokenizer & tokens);
    std::string toRSI_MakeRootMove() const;

    // setmovelist
    static shared_ptr<CommandPacket> parse_SetMoveList(std::string const & rsi,
                                                       Tokenizer & tokens);
    std::string toRSI_SetMoveList() const;

    // stop
    static shared_ptr<CommandPacket> parse_Stop(std::string const & rsi,
                                                Tokenizer & tokens);
    std::string toRSI_Stop() const;

    // quit
    static shared_ptr<CommandPacket> parse_Quit(std::string const & rsi,
                                                Tokenizer & tokens);
    std::string toRSI_Quit() const;

private:
    CommandType m_type;

    int m_positionId;
    int m_oldPositionId;
    int m_iterationDepth;
    int m_plyDepth;
    Position m_position;

    std::string m_serverAddress;
    std::string m_serverPort;
    std::string m_loginId;

    Move m_move;
    std::vector<Move> m_moveList;
};

} // namespace godwhale

#endif