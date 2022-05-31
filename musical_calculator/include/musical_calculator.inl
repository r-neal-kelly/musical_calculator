/*
    Copyright 2022 r-neal-kelly
*/

#pragma once

#include "musical_calculator.h"

namespace musical_calculator {

    void
        mode_t::Print(const note_t* const notes, const std::size_t note_count)
    {
        std::string note_string = "";
        for (std::size_t idx = 0, end = note_count; idx < end; idx += 1) {
            note_string.push_back('0' + static_cast<char>(notes[idx]));
        }
        std::cout << note_string << std::endl;
    }

    mode_t::mode_t(const note_t* const notes, const std::size_t note_count) :
        notes(notes),
        note_count(note_count)
    {
    }

    std::size_t
        mode_t::Note_Count()
    {
        return this->note_count;
    }

    const note_t*
        mode_t::Notes()
    {
        return this->notes;
    }

    note_t
        mode_t::Note(std::size_t index)
    {
        assert(index < Note_Count());

        return this->notes[index];
    }

    void
        mode_t::Print()
    {
        return Print(this->notes, this->note_count);
    }

    note_t
        mode_t::operator [](std::size_t index)
    {
        return Note(index);
    }

}

namespace musical_calculator {

    scale_t::scale_t(const note_t* const notes, const std::size_t note_count) :
        mode_t(notes, note_count)
    {
    }

}
