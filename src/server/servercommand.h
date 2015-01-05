#ifndef GODWHALE_SERVER_SERVERCOMMAND_H
#define GODWHALE_SERVER_SERVERCOMMAND_H

#include "position.h"

namespace godwhale {
namespace server {

/**
 * @brief �T�[�o�[���̃R�}���h�^�C�v�ł��B
 */
enum ServerCommandType
{
    SERVERCOMMAND_SETPOSITION,
    SERVERCOMMAND_BEGINGAME,
    SERVERCOMMAND_ENDGAME,
    SERVERCOMMAND_MAKEMOVEROOT,
    SERVERCOMMAND_UNMAKEMOVEROOT,
};

/**
 * @brief �T�[�o�[���̃R�}���h�ł��B
 */
class ServerCommand
{
public:
    explicit ServerCommand(ServerCommandType type)
        : m_type(type)
    {
    }

    /**
     * @brief �R�}���h�^�C�v���擾���܂��B
     */
    ServerCommandType getType() const
    {
        return m_type;
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
     * @brief �w�����ݒ肵�܂��B
     */
    void setMove(Move move)
    {
        m_move = move;
    }

private:
    ServerCommandType m_type;
    Position m_position;
    Move m_move;
};

} // namespace server
} // namespace godwhale

#endif
