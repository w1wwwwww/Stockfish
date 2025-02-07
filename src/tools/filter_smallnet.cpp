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

#include "filter_smallnet.h"

#include <bit>
#include <future>
#include <mutex>

#include "../uci.h"
#include "../evaluate.h"
#include "../position.h"
#include "sfen_stream.h"

namespace Stockfish::Tools {

std::atomic_uint64_t             read    = 0;
std::atomic_uint64_t             written = 0;
std::mutex                       mut;
std::unique_ptr<SfenInputStream> istream;

std::unique_ptr<PSVector> Worker();

std::unique_ptr<PSVector> Worker() {
    std::unique_ptr<PSVector> sfens = std::make_unique<PSVector>();
    mut.lock();
    while (!istream->eof())
    {
        std::optional<PackedSfenValue> val = istream->next();
        mut.unlock();

        if (!val.has_value())
        {
            mut.lock();
            break;
        }

        read++;

        chess::Position pos = binpack::nodchip::pos_from_packed_sfen(
          std::bit_cast<binpack::nodchip::PackedSfen, PackedSfen>(val->sfen));

        Stockfish::Position spos;
        StateInfo*          info = static_cast<StateInfo*>(malloc(sizeof(StateInfo)));
        spos.set(pos.fen(), false, info);

        if (Stockfish::Eval::use_smallnet(spos))
            sfens->push_back(val.value());

        free(info);

        written++;

        std::cout << "Read: " << read << " Written: " << written << std::endl;

        mut.lock();
    }
    mut.unlock();

    return sfens;
}

void FilterSmallnet(std::string in, std::string out, int threads) {
    istream = open_sfen_input_file(in);

    std::vector<std::future<std::unique_ptr<PSVector>>> fsfens;

    for (int i = 0; i < threads; i++)
        fsfens.push_back(std::async(std::launch::async, Worker));

    PSVector sfens;
    for (int i = 0; i < threads; i++)
    {
        fsfens[i].wait();
        std::unique_ptr<PSVector> fsfen = fsfens[i].get();
        sfens.insert(sfens.end(), fsfen->begin(), fsfen->end());
    }

    create_new_sfen_output(out)->write(sfens);
}

}