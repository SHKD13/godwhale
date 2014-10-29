#include "precomp.h"
#include "stdinc.h"
#include "io_sfen.h"
#include "commandpacket.h"

namespace godwhale {

const CommandPacket::CharSeparator CommandPacket::ms_separator(" ", "");

CommandPacket::CommandPacket(CommandType type)
    : m_type(type), m_positionId(-1), m_position(false)
{
}

CommandPacket::CommandPacket(CommandPacket const & other)
    : m_positionId(-1), m_position(false)
{
}

CommandPacket::CommandPacket(CommandPacket && other)
    : m_positionId(-1), m_position(false)
{
}

/**
 * @brief �R�}���h�̎��s�D�揇�ʂ��擾���܂��B
 *
 * �l���傫�������A�D�揇�ʂ͍����ł��B
 */
int CommandPacket::getPriority() const
{
    switch (m_type) {
    // �I���n�̃R�}���h�͂����Ɏ��s
    case COMMAND_QUIT:
    case COMMAND_STOP:
        return 100;

    // �ʏ�̃R�}���h�͂��̂܂܂̏���
    case COMMAND_LOGIN:
    case COMMAND_SETPOSITION:
    case COMMAND_MAKEMOVEROOT:
    case COMMAND_SETPV:
    case COMMAND_SETMOVELIST:
    case COMMAND_VERIFY:
        return 50;

    // �G���[�݂����Ȃ���
    case COMMAND_NONE:
        return 0;
    }

    unreachable();
    return -1;
}

/**
 * @brief str��target�������g�[�N���ł��邩���ׂ܂��B
 */
bool CommandPacket::isToken(std::string const & str, std::string const & target)
{
    return (str.compare(target) == 0);
}

/**
 * @brief RSI(remote shogi interface)���p�[�X���A�R�}���h�ɒ����܂��B
 */
shared_ptr<CommandPacket> CommandPacket::parse(std::string const & rsi)
{
    if (rsi.empty()) {
        throw new std::invalid_argument("rsi");
    }

    Tokenizer tokens(rsi, ms_separator);
    std::string token = *tokens.begin();

    if (isToken(token, "login")) {
        return parse_Login(rsi, tokens);
    }
    else if (isToken(token, "setposition")) {
        return parse_SetPosition(rsi, tokens);
    }
    else if (isToken(token, "makemoveroot")) {
        return parse_MakeMoveRoot(rsi, tokens);
    }
    else if (isToken(token, "setmovelist")) {
        return parse_SetMoveList(rsi, tokens);
    }
    else if (isToken(token, "stop")) {
        return parse_Stop(rsi, tokens);
    }
    else if (isToken(token, "quit")) {
        return parse_Quit(rsi, tokens);
    }

    return shared_ptr<CommandPacket>();
}

/**
 * @brief �R�}���h��RSI(remote shogi interface)�ɕϊ����܂��B
 */
std::string CommandPacket::toRSI() const
{
    assert(m_type != COMMAND_NONE);

    switch (m_type) {
    case COMMAND_LOGIN:
        return toRSI_Login();
    case COMMAND_SETPOSITION:
        return toRSI_SetPosition();
    case COMMAND_MAKEMOVEROOT:
        return toRSI_MakeMoveRoot();
    case COMMAND_SETMOVELIST:
        return toRSI_SetMoveList();
    case COMMAND_STOP:
        return toRSI_Stop();
    case COMMAND_QUIT:
        return toRSI_Quit();
    }

    unreachable();
    return std::string();
}


#pragma region Login
/**
 * @brief login�R�}���h���p�[�X���܂��B
 *
 * login <address> <port> <login_name> <nthreads>
 */
shared_ptr<CommandPacket> CommandPacket::parse_Login(std::string const & rsi,
                                                     Tokenizer & tokens)
{
    shared_ptr<CommandPacket> result(new CommandPacket(COMMAND_LOGIN));
    Tokenizer::iterator begin = ++tokens.begin();

    result->m_serverAddress = *begin++;
    result->m_serverPort = *begin++;
    result->m_loginName = *begin++;
    return result;
}

/**
 * @brief login�R�}���h��RSI�ɕϊ����܂��B
 */
std::string CommandPacket::toRSI_Login() const
{
    return (F("login %1% %2% %3%")
        % m_serverAddress % m_serverPort % m_loginName)
        .str();
}
#pragma endregion


#pragma region SetPosition
/**
 * @brief setposition�R�}���h���p�[�X���܂��B
 *
 * setposition <position_id> [sfen <sfen> | startpos] moves <move1> ... <moven>
 * <sfen> = <board_sfen> <turn_sfen> <hand_sfen> <nmoves>
 */
shared_ptr<CommandPacket> CommandPacket::parse_SetPosition(std::string const & rsi,
                                                           Tokenizer & tokens)
{
    shared_ptr<CommandPacket> result(new CommandPacket(COMMAND_SETPOSITION));
    Tokenizer::iterator begin = ++tokens.begin();

    result->m_positionId = lexical_cast<int>(*begin++);

    std::string token = *begin++;
    if (token == "sfen") {
        std::string sfen;
        sfen += *begin++ + " "; // board
        sfen += *begin++ + " "; // turn
        sfen += *begin++ + " "; // hand
        sfen += *begin++;       // nmoves
        result->m_position = sfenToPosition(sfen);
    }
    else if (token == "startpos") {
        result->m_position = Position();
    }

    // moves�͂Ȃ����Ƃ�����܂��B
    if (begin == tokens.end()) {
        return result;
    }

    if (*begin++ != "moves") {
        throw new ParseException(F("%1%: �w���肪����������܂���B") % rsi);
    }

    for (; begin != tokens.end(); ++begin) {
        Move move = sfenToMove(result->m_position, *begin);

        if (!result->m_position.makeMove(move)) {
            LOG_ERROR() << result->m_position;
            LOG_ERROR() << *begin << ": �w���肪����������܂���B";
            throw new ParseException(F("%1%: �w���肪����������܂���B") % *begin);
        }
    }

    return result;
}

/**
 * @brief setposition�R�}���h��RSI�ɕϊ����܂��B
 */
std::string CommandPacket::toRSI_SetPosition() const
{
    Position position = m_position;
    std::vector<Move> moves = position.getMoveList();

    // �ǖʂ����ׂĖ߂��܂��B
    while (!position.getMoveList().empty()) {
        position.unmakeMove();
    }

    std::string posStr;
    if (position.isInitial()) {
        posStr = " startpos";
    }
    else {
        posStr  = " sfen ";
        posStr += positionToSfen(position);
    }

    std::string movesStr;
    if (!moves.empty()) {
        movesStr += " moves";
        BOOST_FOREACH(Move move, moves) {
            movesStr += " ";
            movesStr += moveToSfen(move);
        }
    }

    return (F("setposition %1%%2%%3%")
        % m_positionId % posStr % movesStr)
        .str();
}
#pragma endregion


#pragma region MakeRootMove
/**
 * @brief makemoveroot�R�}���h���p�[�X���܂��B
 *
 * makemoveroot <position_id> <old_position_id> <move>
 */
shared_ptr<CommandPacket> CommandPacket::parse_MakeMoveRoot(std::string const & rsi,
                                                            Tokenizer & tokens)
{
    shared_ptr<CommandPacket> result(new CommandPacket(COMMAND_MAKEMOVEROOT));
    Tokenizer::iterator begin = ++tokens.begin(); // �ŏ��̃g�[�N���͔�΂��܂��B

    result->m_positionId = lexical_cast<int>(*begin++);
    result->m_oldPositionId = lexical_cast<int>(*begin++);

    // �^����ꂽpositionId�Ȃǂ���ǖʂ��������܂��B
    Position position;

    result->m_move = sfenToMove(position, *begin++);
    return result;
}

/**
 * @brief makerootmove�R�}���h��RSI�ɕϊ����܂��B
 */
std::string CommandPacket::toRSI_MakeMoveRoot() const
{
    return (F("makemoveroot %1% %2% %3%")
        % m_positionId % m_oldPositionId % moveToSfen(m_move))
        .str();
}
#pragma endregion


#pragma region SetMoveList
/**
 * @brief setmovelist�R�}���h���쐬���܂��B
 */
shared_ptr<CommandPacket> CommandPacket::createSetMoveList(int positionId,
                                                           int iterationDepth,
                                                           int plyDepth,
                                                           std::vector<Move> const & moves)
{
    shared_ptr<CommandPacket> result(new CommandPacket(COMMAND_SETMOVELIST));

    result->m_positionId = positionId;
    result->m_iterationDepth = iterationDepth;
    result->m_plyDepth = plyDepth;
    result->m_moveList = moves;

    return result;
}

/**
 * @brief setmovelist�R�}���h���p�[�X���܂��B
 *
 * setmovelist <position_id> <itd> <pld> <move1> ... <moven>
 */
shared_ptr<CommandPacket> CommandPacket::parse_SetMoveList(std::string const & rsi,
                                                           Tokenizer & tokens)
{
    Tokenizer::iterator begin = ++tokens.begin(); // �ŏ��̃g�[�N���͔�΂��܂��B

    int positionId = lexical_cast<int>(*begin++);
    int iterationDepth = lexical_cast<int>(*begin++);
    int plyDepth = lexical_cast<int>(*begin++);

    // �^����ꂽpositionId�Ȃǂ���ǖʂ��������܂��B
    Position position;

    std::vector<Move> moves;
    Tokenizer::iterator end = tokens.end();
    for (; begin != end; ++begin) {
        Move move = sfenToMove(position, *begin);

        if (move.isEmpty()) {
            throw ParseException(F("%1%: �������w����ł͂���܂���B") % *begin);
        }

        moves.push_back(move);
    }

    return createSetMoveList(positionId, iterationDepth, plyDepth, moves);
}

/**
 * @brief setmovelist�R�}���h��RSI�ɕϊ����܂��B
 */
std::string CommandPacket::toRSI_SetMoveList() const
{
    std::string movesStr;
    BOOST_FOREACH(Move move, m_moveList) {
        movesStr += " ";
        movesStr += moveToSfen(move);
    }

    return (F("setmovelist %1% %2% %3% %4%")
        % m_positionId %m_iterationDepth % m_plyDepth % movesStr)
        .str();
}
#pragma endregion


#pragma region Start
/**
 * @brief start�R�}���h���쐬���܂��B
 */
shared_ptr<CommandPacket> CommandPacket::createStart(int positionId,
                                                     int iterationDepth,
                                                     int plyDepth,
                                                     int alpha, int beta)
{
    shared_ptr<CommandPacket> result(new CommandPacket(COMMAND_START));

    result->m_positionId = positionId;
    result->m_iterationDepth = iterationDepth;
    result->m_plyDepth = plyDepth;
    result->m_alpha = alpha;
    result->m_beta = beta;

    return result;
}

/**
 * @brief start�R�}���h���p�[�X���܂��B
 *
 * start <position_id> <itd> <pld> <alpha> <beta>
 */
shared_ptr<CommandPacket> CommandPacket::parse_Start(std::string const & rsi,
                                                     Tokenizer & tokens)
{
    Tokenizer::iterator begin = ++tokens.begin(); // �ŏ��̃g�[�N���͔�΂��܂��B

    int positionId = lexical_cast<int>(*begin++);
    int iterationDepth = lexical_cast<int>(*begin++);
    int plyDepth = lexical_cast<int>(*begin++);
    int alpha = lexical_cast<int>(*begin++);
    int beta = lexical_cast<int>(*begin++);

    return createStart(positionId, iterationDepth, plyDepth, alpha, beta);
}

/**
 * @brief start�R�}���h��RSI�ɕϊ����܂��B
 */
std::string CommandPacket::toRSI_Start() const
{
    return (F("start %1% %2% %3% %4% %5%")
        % m_positionId %m_iterationDepth % m_plyDepth % m_alpha % m_beta)
        .str();
}
#pragma endregion


#pragma region Stop
/**
 * @brief stop�R�}���h���p�[�X���܂��B
 */
shared_ptr<CommandPacket> CommandPacket::parse_Stop(std::string const & rsi,
                                                    Tokenizer & tokens)
{
    return shared_ptr<CommandPacket>(new CommandPacket(COMMAND_STOP));
}

/**
 * @brief stop�R�}���h��RSI�ɕϊ����܂��B
 */
std::string CommandPacket::toRSI_Stop() const
{
    return "stop";
}
#pragma endregion


#pragma region Quit
/**
 * @brief quit�R�}���h���p�[�X���܂��B
 */
shared_ptr<CommandPacket> CommandPacket::parse_Quit(std::string const & rsi,
                                                    Tokenizer & tokens)
{
    return shared_ptr<CommandPacket>(new CommandPacket(COMMAND_QUIT));
}

/**
 * @brief quit�R�}���h��RSI�ɕϊ����܂��B
 */
std::string CommandPacket::toRSI_Quit() const
{
    return "quit";
}
#pragma endregion

} // namespace godwhale
