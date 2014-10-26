
#ifndef GODWHALE_IOSFEN_H
#define GODWHALE_IOSFEN_H

#include "exceptions.h"
#include "position.h"
#include "move.h"

namespace godwhale {

extern std::string moveToSfen(Move const & move);
extern Move sfenToMove(Position const & position, std::string const & sfen);

extern std::string positionToSfen(Position const & position);
extern Position sfenToPosition(std::string const & sfen);

/**
 * @brief SFEN�`���̘A�������w���蕶������A���ۂ̎w����ɕϊ����܂��B
 */
template<typename Sequence>
inline std::vector<Move> sfenToMoveList(Position && position,
                                        Sequence & list)
{
    auto begin = boost::begin(list);
    auto end = boost::end(list);

    std::vector<Move> result;
    for (; begin != end; ++begin) {
        Move move = sfenToMove(position, *begin);
        if (move.isEmpty()) {
            return result;
        }
            
        if (!position.makeMove(move)) {
            return result;
        }

        result.push_back(move);
    }

    return result;
}

/**
 * @brief SFEN�`���̘A�������w���蕶������A���ۂ̎w����ɕϊ����܂��B
 */
template<typename Sequence>
inline std::vector<Move> sfenToMoveList(Position const & position,
                                        Sequence & list)
{
    Position tmp(position);

    return sfenToMoveList(std::move(tmp), list);
}

} // namespace godwhale

#endif
