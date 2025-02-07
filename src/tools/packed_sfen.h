/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2025 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PACKED_SFEN_H
#define PACKED_SFEN_H

#include <vector>
#include <cstdint>

namespace Stockfish::Tools {

// packed sfen
struct PackedSfen {
    std::uint8_t data[32];
};

// Structure in which PackedSfen and evaluation value are integrated
// If you write different contents for each option, it will be a problem when reusing the teacher game
// For the time being, write all the following members regardless of the options.
struct PackedSfenValue {
    // phase
    PackedSfen sfen;

    // Evaluation value returned from Tools::search()
    std::int16_t score;

    // PV first move
    // Used when finding the match rate with the teacher
    std::uint16_t move;

    // Trouble of the phase from the initial phase.
    std::uint16_t gamePly;

    // 1 if the player on this side ultimately wins the game. -1 if you are losing.
    // 0 if a draw is reached.
    // The draw is in the teacher position generation command gensfen,
    // Only write if LEARN_GENSFEN_DRAW_RESULT is enabled.
    std::int8_t game_result;

    // When exchanging the file that wrote the teacher aspect with other people
    //Because this structure size is not fixed, pad it so that it is 40 bytes in any environment.
    std::uint8_t padding;

    // 32 + 2 + 2 + 2 + 1 + 1 = 40bytes
};

// Phase array: PSVector stands for packed sfen vector.
using PSVector = std::vector<PackedSfenValue>;
}

#endif