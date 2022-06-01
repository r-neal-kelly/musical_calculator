/*
    Copyright 2022 r-neal-kelly
*/

#pragma once

#include "musical_calculator.h"

namespace musical_calculator {

    void
        mode_t::Print(const note_t* const notes, const count_t note_count)
        noexcept
    {
        std::string note_string = "";
        for (index_t idx = 0, end = note_count; idx < end; idx += 1) {
            note_string.push_back('0' + static_cast<char>(notes[idx]));
        }
        std::cout << note_string << std::endl;
    }

    void
        mode_t::Print(const mode_t& mode)
        noexcept
    {
        return Print(mode.notes, mode.note_count);
    }

    void
        mode_t::Print(mode_t&& mode)
        noexcept
    {
        return Print(mode.notes, mode.note_count);
    }

    mode_t::mode_t(const note_t* const notes, const count_t note_count) noexcept :
        notes(notes),
        note_count(note_count)
    {
        assert(this->notes);
        assert(this->note_count > 0);
    }

    count_t
        mode_t::Note_Count()
        noexcept
    {
        return this->note_count;
    }

    const note_t*
        mode_t::Notes()
        noexcept
    {
        return this->notes;
    }

    note_t
        mode_t::Note(index_t index)
        noexcept
    {
        assert(index < Note_Count());

        return this->notes[index];
    }

    void
        mode_t::Print()
        noexcept
    {
        return Print(this->notes, this->note_count);
    }

    note_t
        mode_t::operator [](index_t index)
        noexcept
    {
        return Note(index);
    }

}

namespace musical_calculator {

    scale_t::scale_t(const note_t* const notes, const count_t note_count) noexcept :
        mode_t(notes, note_count)
    {
    }

}

namespace musical_calculator {

    template <count_t CHROMATIC_NOTE_COUNT_p>
    mode_tier_t<CHROMATIC_NOTE_COUNT_p>::mode_tier_t() noexcept :
        notes(nullptr)
    {
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    mode_tier_t<CHROMATIC_NOTE_COUNT_p>::mode_tier_t(note_t* notes, const count_t mode_note_count) noexcept :
        notes(notes)
    {
        assert(this->notes);
        assert(mode_note_count > 0);
        assert(mode_note_count <= CHROMATIC_NOTE_COUNT_p);

        std::vector<note_t> mode_cache;
        mode_cache.reserve(mode_note_count);

        // the first mode is simply all the possible notes that can be taken
        // from the chromatic scale without any skips, up to the mode_note_count.
        for (note_t note = 1, last_note = mode_note_count; note <= last_note; note += 1) {
            mode_cache.push_back(note);
        }
        for (index_t idx = 0, end = mode_note_count; idx < end; idx += 1) {
            *notes = mode_cache[idx];
            notes += 1;
        }

        // we never change the first place, so if that's all there is, we go ahead and return.
        if (mode_note_count > 1) {
            // we can now proceed to mutate the mode by incrementing each place
            // from the least significant digit to the most significant digit.
            // we do not increment a place past the value of the lesser place to its
            // right, thus we end up with combinations instead of permutations.
            while (true) {
                // get the least significant digit iterations first,
                // because we can't check a lesser place. Therefore it
                // can allow upto the total number of notes possible.
                while (mode_cache[mode_note_count - 1] + 1 <= CHROMATIC_NOTE_COUNT_p) {
                    mode_cache[mode_note_count - 1] += 1;
                    for (index_t idx = 0, end = mode_note_count; idx < end; idx += 1) {
                        *notes = mode_cache[idx];
                        notes += 1;
                    }
                }

                // we need to find a greater place that can be incremented.
                // if it doesn't exist, then we are finished. we don't consider
                // idx 0 because we're only working with the one key, and so it
                // can never be more than 1, and is thus never incrementable.
                index_t next_idx = 0;
                bool found_next_idx = false;
                for (index_t idx = mode_note_count - 2; idx > 0; idx -= 1) {
                    if (mode_cache[idx] + 1 < mode_cache[idx + 1]) {
                        next_idx = idx;
                        found_next_idx = true;

                        break;
                    }
                }

                // if we did find it, we increment the place by one and reset
                // the lesser places for another iteration of while(true).
                if (found_next_idx) {
                    mode_cache[next_idx] += 1;
                    for (index_t idx = next_idx + 1, end = mode_cache.size(); idx < end; idx += 1) {
                        mode_cache[idx] = mode_cache[idx - 1] + 1;
                    }
                    for (index_t idx = 0, end = mode_note_count; idx < end; idx += 1) {
                        *notes = mode_cache[idx];
                        notes += 1;
                    }

                    continue;
                } else {
                    break;
                }
            }
        }
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    void
        mode_tier_t<CHROMATIC_NOTE_COUNT_p>::Print_Modes(count_t mode_count, count_t mode_note_count)
        noexcept
    {
        const note_t* note = this->notes;
        const note_t* notes_end = this->notes + (mode_count * mode_note_count);
        for (; note < notes_end; note += mode_note_count) {
            mode_t::Print(note, mode_note_count);
        }
    }

}

namespace musical_calculator {

    template <count_t CHROMATIC_NOTE_COUNT_p>
    void
        scale_tier_t<CHROMATIC_NOTE_COUNT_p>::Scale_Modes(const note_t* scale,
                                                          const count_t scale_note_count,
                                                          note_t* const results)
        noexcept
    {
        // the first mode is always going to be the scale (or first mode) that is passed in.
        for (index_t idx = 0, end = scale_note_count; idx < end; idx += 1) {
            results[idx] = scale[idx];
        }

        for (index_t previous_mode_idx = 0, previous_mode_end = scale_note_count * scale_note_count - scale_note_count;
             previous_mode_idx < previous_mode_end;
             previous_mode_idx += scale_note_count) {
            const index_t this_mode_idx = previous_mode_idx + scale_note_count;

            // first, we revolve the actual notes as they appear from the previous mode.
            // then we set the last new note to the previous's first + the chromatic_note_count.
            for (index_t note_idx = 0, note_end = scale_note_count - 1;
                 note_idx < note_end;
                 note_idx += 1) {
                results[this_mode_idx + note_idx] = results[previous_mode_idx + note_idx + 1];
            }
            results[this_mode_idx + scale_note_count - 1] = results[previous_mode_idx + 0] + CHROMATIC_NOTE_COUNT_p;

            // then for each subsequent note we add the total note count and subtract by new_first_note - 1.
            const count_t first_note_diff = results[this_mode_idx + 0] - 1;
            for (index_t note_idx = 0, note_end = scale_note_count;
                 note_idx < note_end;
                 note_idx += 1) {
                results[this_mode_idx + note_idx] = results[this_mode_idx + note_idx] - first_note_diff;
            }
        }
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    scale_tier_t<CHROMATIC_NOTE_COUNT_p>::scale_tier_t() noexcept
    {
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    scale_tier_t<CHROMATIC_NOTE_COUNT_p>::scale_tier_t(const mode_tier_t<CHROMATIC_NOTE_COUNT_p>&   mode_tier,
                                                       const count_t                                mode_count,
                                                       const count_t                                mode_note_count)
    {
        // we use this to successively generate all of mode's deriviations performantly
        note_t* note_cache = static_cast<note_t*>(malloc(sizeof(note_t) * mode_note_count * mode_note_count));
        assert(note_cache != nullptr);

        // once we know all the static values for each chromatic, we can do this up front for performance.
        //this->scales.reserve(0);

        // We iterate over the modes to generate an array of pointers to the first occurence of unique scales.
        // We can do this extremely cheaply by utilizing the fact that we generate the modes in numerical order.
        // It is always the case then that the first occurence of a unique scale will be the mode in a set that equates
        // to the lowest numerical number. Therefore if we see a mode that is numerically bigger than it deriviations
        // we know we have already added that mode's scale to the array. We completely avoid lookups doing this,
        // and thus achieve a high level of efficiency and performance while working with the massive number of
        // modes that exist in the larger chromatic scales and their tiers of modes.
        auto Has_Mode_Scale = [](const std::vector<const note_t*>&  scales,
                                 const note_t*                      mode,
                                 const count_t                      mode_note_count,
                                 note_t* const                      note_cache) -> bool
        {
            // we cache all the possible deriviations or revolutions of the mode.
            // notice that we do not allocate and deallocate memory, which would be very non-performant
            Scale_Modes(mode, mode_note_count, note_cache);

            // if the first mode is numerically smaller than its deriviations, it's the first occurence of a unique scale
            for (index_t mode_idx = mode_note_count, mode_end = mode_note_count * mode_note_count;
                 mode_idx < mode_end;
                 mode_idx += mode_note_count) {
                for (index_t note_idx = 1, note_end = mode_note_count;
                     note_idx < note_end;
                     note_idx += 1) {
                    if (note_cache[note_idx] > note_cache[mode_idx + note_idx]) {
                        return true;
                    } else if (note_cache[note_idx] < note_cache[mode_idx + note_idx]) {
                        break;
                    }
                }
            }

            return false;
        };

        for (index_t modes_idx = 0, modes_end = mode_count * mode_note_count;
             modes_idx < modes_end;
             modes_idx += mode_note_count) {
            const note_t* const mode = mode_tier.notes + modes_idx;
            if (!Has_Mode_Scale(this->scales, mode, mode_note_count, note_cache)) {
                scales.push_back(mode);
            }
        }

        free(note_cache);
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    void
        scale_tier_t<CHROMATIC_NOTE_COUNT_p>::Print_Scales(const count_t scale_note_count)
        noexcept
    {
        for (index_t idx = 0, end = this->scales.size(); idx < end; idx += 1) {
            const note_t* const scale = this->scales[idx];
            std::string scale_string = "";
            for (index_t idx = 0, end = scale_note_count; idx < end; idx += 1) {
                scale_string.push_back('0' + static_cast<char>(scale[idx]));
            }
            std::cout << scale_string << std::endl;
        }
    }

}

namespace musical_calculator {

    template <count_t CHROMATIC_NOTE_COUNT_p>
    chromatic_t<CHROMATIC_NOTE_COUNT_p>::chromatic_t()
    {
        // We allocate enough memory to store all modes in the chromatic scale in one place,
        // primary for performance purposes and to avoid using more memory than necessary when dissecting the modes.
        this->notes = static_cast<note_t*>(malloc(sizeof(note_t) * CHROMATIC_MODE_NOTE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1]));
        assert(this->notes != nullptr);

        // We concurrently computate modes and subsequently their scales.
        // Each mode tier and thus scale tier does not rely on any other tier.
        // Some tiers take way longer to calcuate than others. Some shouldn't
        // even be done concurrently because it would be faster to just do them
        // in this thread. We currently do all tiers concurrently just because it's
        // simpler to write and read, and the time saved is currently not significant.
        note_t* notes = this->notes;
        std::vector<std::jthread> threads;
        threads.reserve(CHROMATIC_NOTE_COUNT_p);
        for (index_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
            threads.push_back(std::jthread(
                [this, notes, idx]() -> void
                {
                    // we always have to calcuate each tier's modes before each tier's scales.
                    this->mode_tiers[idx] = mode_tier_t<CHROMATIC_NOTE_COUNT_p>(notes, idx + 1);

                    this->scale_tiers[idx] = scale_tier_t<CHROMATIC_NOTE_COUNT_p>(
                        this->mode_tiers[idx],
                        CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx],
                        idx + 1);
                }
            ));

            notes += CHROMATIC_TIER_MODE_NOTE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx];
        }
        for (index_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
            threads[idx].join();
        }
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    chromatic_t<CHROMATIC_NOTE_COUNT_p>::~chromatic_t()
        noexcept
    {
        free(this->notes);
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    constexpr count_t
        chromatic_t<CHROMATIC_NOTE_COUNT_p>::Chromatic_Note_Count()
        noexcept
    {
        return CHROMATIC_NOTE_COUNT_p;
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    constexpr count_t
        chromatic_t<CHROMATIC_NOTE_COUNT_p>::Mode_Count()
        noexcept
    {
        return CHROMATIC_MODE_COUNTS[Chromatic_Note_Count() - 1];
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    count_t
        chromatic_t<CHROMATIC_NOTE_COUNT_p>::Scale_Count()
        noexcept
    {
        count_t count = 0;
        for (index_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
            count += this->scale_tiers[idx].scales.size();
        }

        return count;
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    std::vector<mode_t>
        chromatic_t<CHROMATIC_NOTE_COUNT_p>::Modes()
    {
        std::vector<mode_t> modes;
        modes.reserve(Mode_Count());

        for (index_t tier_idx = 0, tier_end = CHROMATIC_NOTE_COUNT_p;
             tier_idx < tier_end;
             tier_idx += 1) {
            const count_t tier_mode_count = CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][tier_idx];
            const count_t mode_note_count = tier_idx + 1;
            for (index_t mode_idx = 0, mode_end = tier_mode_count * mode_note_count;
                 mode_idx < mode_end;
                 mode_idx += mode_note_count) {
                modes.push_back(mode_t(this->mode_tiers[tier_idx].notes + mode_idx, mode_note_count));
            }
        }

        return modes;
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    std::vector<scale_t>
        chromatic_t<CHROMATIC_NOTE_COUNT_p>::Scales()
    {
        std::vector<scale_t> scales;
        scales.reserve(Scale_Count());

        for (index_t tier_idx = 0, tier_end = CHROMATIC_NOTE_COUNT_p;
             tier_idx < tier_end;
             tier_idx += 1) {
            const scale_tier_t& scale_tier = this->scale_tiers[tier_idx];
            const count_t scale_note_count = tier_idx + 1;
            for (index_t scale_idx = 0, scale_end = scale_tier.scales.size();
                 scale_idx < scale_end;
                 scale_idx += 1) {
                scales.push_back(scale_t(scale_tier.scales[scale_idx], scale_note_count));
            }
        }

        return scales;
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    void
        chromatic_t<CHROMATIC_NOTE_COUNT_p>::Print_Modes()
        noexcept
    {
        for (index_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
            this->mode_tiers[idx].Print_Modes(CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx], idx + 1);
        }
    }

    template <count_t CHROMATIC_NOTE_COUNT_p>
    void
        chromatic_t<CHROMATIC_NOTE_COUNT_p>::Print_Scales()
        noexcept
    {
        for (index_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
            this->scale_tiers[idx].Print_Scales(idx + 1);
        }
    }

}
