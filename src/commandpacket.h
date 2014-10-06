#ifndef GODWHALE_COMMANDPACKET_H
#define GODWHALE_COMMANDPACKET_H

#include "position.h"

namespace godwhale {

using namespace boost;

/**
 * @brief �R�}���h���ʎq�ł��B
 *
 * �R�}���h�Ƃ̓T�[�o�[����N���C�A���g�ɑ�����
 * �C���X�g���N�V�����ł��B
 */
enum CommandType
{
    COMMAND_NONE,
    COMMAND_SETROOT,
    COMMAND_MAKEMOVE,
    COMMAND_UNMAKEMOVE,

    COMMAND_SETMOVELIST,
    COMMAND_SHRINK,
    COMMAND_EXTEND,
    COMMAND_VERIFY,
    COMMAND_STOP,

    COMMAND_QUIT,
};

/**
 * @brief �R�}���h�f�[�^���܂Ƃ߂Ĉ����܂��B
 */
class CommandPacket
{
public:
    /// �g��USI���p�[�X���A�R�}���h�ɒ����܂��B
    static shared_ptr<CommandPacket> Parse(std::string const & exusi);

public:
    explicit CommandPacket(CommandType type);
    explicit CommandPacket(CommandPacket const & other);
    explicit CommandPacket(CommandPacket && other);

    /// �R�}���h�^�C�v���擾���܂��B
    CommandType getType() const
    {
        return m_type;
    }

    /// �R�}���h�̎��s�D�揇�ʂ��擾���܂��B
    int getPriority() const;

    /// �ǖ�ID���擾���܂��B
    int getPositionId() const
    {
        return m_positionId;
    }

    /// �ǖ�ID��ݒ肵�܂��B
    void setPositionId(int id)
    {
        m_positionId = id;
    }

    /// �ǖʂ��擾���܂��B
    Position const &getPosition() const
    {
        return m_position;
    }

    /// �ǖʂ�ݒ肵�܂��B
    void setPosition(Position const & position)
    {
        m_position = position;
    }

    /// �w����̃��X�g(��̈ꗗ��o�u�Ȃ�)���擾���܂��B
    std::vector<Move> const &getMoveList() const
    {
        return m_moveList;
    }

    /// �R�}���h���g��USI�ɕϊ����܂��B
    std::string ToExUsi();

private:
    CommandType m_type;

    int m_positionId;
    Position m_position;
    std::vector<Move> m_moveList;
};

} // namespace godwhale

#endif
