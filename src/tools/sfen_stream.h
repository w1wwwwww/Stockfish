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

#ifndef SFEN_STREAM_H
#define SFEN_STREAM_H

#include "nnue_data_binpack_format.h"
#include "packed_sfen.h"

namespace Stockfish::Tools {
static bool ends_with(const std::string& lhs, const std::string& end) {
    if (end.size() > lhs.size())
        return false;

    return std::equal(end.rbegin(), end.rend(), lhs.rbegin());
}

static std::string filename_with_extension(const std::string& filename, const std::string& ext) {
    if (ends_with(filename, ext))
    {
        return filename;
    }
    else
    {
        return filename + "." + ext;
    }
}

struct SfenInputStream {
    static constexpr auto           openmode  = std::ios::in | std::ios::binary;
    static inline const std::string extension = "binpack";

    SfenInputStream(std::string filename) :
        m_stream(filename, openmode),
        m_eof(!m_stream.hasNext()) {}

    std::optional<PackedSfenValue> next() {
        static_assert(sizeof(binpack::nodchip::PackedSfenValue) == sizeof(PackedSfenValue));

        if (!m_stream.hasNext())
        {
            m_eof = true;
            return std::nullopt;
        }

        auto            training_data_entry = m_stream.next();
        auto            v = binpack::trainingDataEntryToPackedSfenValue(training_data_entry);
        PackedSfenValue psv;
        // same layout, different types. One is from generic library.
        std::memcpy(&psv, &v, sizeof(PackedSfenValue));

        return psv;
    }

    bool eof() const { return m_eof; }

   private:
    binpack::CompressedTrainingDataEntryReader m_stream;
    bool                                       m_eof;
};

struct SfenOutputStream {
    static constexpr auto           openmode  = std::ios::out | std::ios::binary | std::ios::app;
    static inline const std::string extension = "binpack";

    SfenOutputStream(std::string filename) :
        m_stream(filename_with_extension(filename, extension), openmode) {}

    void write(const PSVector& sfens) {
        static_assert(sizeof(binpack::nodchip::PackedSfenValue) == sizeof(PackedSfenValue));

        for (auto& sfen : sfens)
        {
            // The library uses a type that's different but layout-compatibile.
            binpack::nodchip::PackedSfenValue e;
            std::memcpy(&e, &sfen, sizeof(binpack::nodchip::PackedSfenValue));
            m_stream.addTrainingDataEntry(binpack::packedSfenValueToTrainingDataEntry(e));
        }
    }

   private:
    binpack::CompressedTrainingDataEntryWriter m_stream;
};

inline std::unique_ptr<SfenInputStream> open_sfen_input_file(const std::string& filename) {
    return std::make_unique<SfenInputStream>(filename);
}

inline std::unique_ptr<SfenOutputStream> create_new_sfen_output(const std::string& filename) {
    return std::make_unique<SfenOutputStream>(filename);
}
}

#endif