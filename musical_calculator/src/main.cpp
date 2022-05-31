/*
    Copyright 2022 r-neal-kelly
*/

#include <climits>
#include <cstdint>
#include <thread>
#include <vector>

#include "musical_calculator.h"

namespace musical_calculator {

    template <std::size_t CHROMATIC_NOTE_COUNT_p = 12>
    class chromatic_t
    {
    public:
        static_assert(CHROMATIC_NOTE_COUNT_p <= MAX_CHROMATIC_NOTE_COUNT);

    public:
        class mode_tier_t
        {
        public:
            const note_t*   notes;

        public:
            mode_tier_t() :
                notes(nullptr)
            {
            }

            mode_tier_t(note_t*             notes,
                        const std::size_t   mode_note_count) :
                notes(notes)
            {
                assert(mode_note_count > 0);
                assert(mode_note_count <= CHROMATIC_NOTE_COUNT_p);

                std::vector<note_t> mode_cache;
                mode_cache.reserve(mode_note_count);

                // the first mode is simply all the possible notes that can be taken
                // from the chromatic scale without any skips, up to the mode_note_count.
                for (std::size_t note = 1, last_note = mode_note_count; note <= last_note; note += 1) {
                    mode_cache.push_back(note);
                }
                for (std::size_t idx = 0, end = mode_note_count; idx < end; idx += 1) {
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
                            for (std::size_t idx = 0, end = mode_note_count; idx < end; idx += 1) {
                                *notes = mode_cache[idx];
                                notes += 1;
                            }
                        }

                        // we need to find a greater place that can be incremented.
                        // if it doesn't exist, then we are finished. we don't consider
                        // idx 0 because we're only working with the one key, and so it
                        // can never be more than 1, and is thus never incrementable.
                        std::size_t next_idx = 0;
                        bool found_next_idx = false;
                        for (std::size_t idx = mode_note_count - 2; idx > 0; idx -= 1) {
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
                            for (std::size_t idx = next_idx + 1, end = mode_cache.size(); idx < end; idx += 1) {
                                mode_cache[idx] = mode_cache[idx - 1] + 1;
                            }
                            for (std::size_t idx = 0, end = mode_note_count; idx < end; idx += 1) {
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

        public:
            void
                Print_Modes(std::size_t mode_count,
                            std::size_t mode_note_count)
            {
                const note_t* note = this->notes;
                const note_t* notes_end = this->notes + (mode_count * mode_note_count);
                for (; note < notes_end; note += mode_note_count) {
                    mode_t::Print(note, mode_note_count);
                }
            }
        };

        class scale_tier_t
        {
        public:
            static void
                Scale_Modes(const note_t*       scale,
                            const std::size_t   scale_note_count,
                            note_t* const       results)
            {
                for (std::size_t idx = 0, end = scale_note_count; idx < end; idx += 1) {
                    results[idx] = scale[idx];
                }

                for (std::size_t previous_mode_idx = 0, previous_mode_end = scale_note_count * scale_note_count - scale_note_count;
                     previous_mode_idx < previous_mode_end;
                     previous_mode_idx += scale_note_count) {
                    const std::size_t this_mode_idx = previous_mode_idx + scale_note_count;

                    // first, we revolve the actual notes as they appear from the previous mode.
                    // then we set the last new note to the previous's first + the chromatic_note_count.
                    for (std::size_t note_idx = 0, note_end = scale_note_count - 1;
                         note_idx < note_end;
                         note_idx += 1) {
                        results[this_mode_idx + note_idx] = results[previous_mode_idx + note_idx + 1];
                    }
                    results[this_mode_idx + scale_note_count - 1] = results[previous_mode_idx + 0] + CHROMATIC_NOTE_COUNT_p;

                    // then for each subsequent note after the new first note,
                    // we add the total note count and subtract by new_first_note - 1.
                    const std::size_t first_note_diff = results[this_mode_idx + 0] - 1;
                    for (std::size_t note_idx = 0, note_end = scale_note_count;
                         note_idx < note_end;
                         note_idx += 1) {
                        results[this_mode_idx + note_idx] = results[this_mode_idx + note_idx] - first_note_diff;
                    }
                }
            }

            static bool
                Has_Mode_Scale(const std::vector<const note_t*>&    scales,
                               const note_t*                        mode,
                               const std::size_t                    mode_note_count,
                               note_t* const                        note_cache)
            {
                // we cache all the possible revolutions of the mode so that
                // we can quickly search them per scale.
                Scale_Modes(mode, mode_note_count, note_cache);

                // instead of looking in the vector, what we do is see if the first mode is a smaller number.
                // if it's smaller than the others, it's a new scale, else the vector should already have it.
                for (std::size_t mode_idx = mode_note_count, mode_end = mode_note_count * mode_note_count;
                     mode_idx < mode_end;
                     mode_idx += mode_note_count) {
                    for (std::size_t note_idx = 1, note_end = mode_note_count;
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
            }

        public:
            std::vector<const note_t*>  scales;

        public:
            scale_tier_t()
            {
            }

            scale_tier_t(const mode_tier_t& mode_tier,
                         const std::size_t  mode_count,
                         const std::size_t  mode_note_count)
            {
                // we use this to generate all the modes from a scale
                note_t* note_cache = static_cast<note_t*>(malloc(sizeof(note_t) * mode_note_count * mode_note_count));
                assert(note_cache != nullptr);

                //this->scales.reserve(0); // once we know all the static values for each chromatic, we can do this up front.

                // just add the first instance of each unique set of modes, which is its scale.
                for (std::size_t modes_idx = 0, modes_end = mode_count * mode_note_count;
                     modes_idx < modes_end;
                     modes_idx += mode_note_count) {
                    const note_t* const mode = mode_tier.notes + modes_idx;
                    if (!Has_Mode_Scale(this->scales, mode, mode_note_count, note_cache)) {
                        scales.push_back(mode);
                    }
                }

                free(note_cache);
            }

        public:
            void
                Print_Scales(std::size_t scale_note_count)
            {
                for (std::size_t idx = 0, end = this->scales.size(); idx < end; idx += 1) {
                    const note_t* const scale = this->scales[idx];
                    std::string scale_string = "";
                    for (std::size_t idx = 0, end = scale_note_count; idx < end; idx += 1) {
                        scale_string.push_back('0' + static_cast<char>(scale[idx]));
                    }
                    std::cout << scale_string << std::endl;
                }
            }
        };

    public:
        note_t*         notes;
        mode_tier_t     mode_tiers[CHROMATIC_NOTE_COUNT_p];
        scale_tier_t    scale_tiers[CHROMATIC_NOTE_COUNT_p];

    public:
        chromatic_t()
        {
            this->notes = static_cast<note_t*>(malloc(sizeof(note_t) * CHROMATIC_MODE_NOTE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1]));
            assert(this->notes != nullptr);

            note_t* notes = this->notes;
            std::vector<std::jthread> threads;
            threads.reserve(CHROMATIC_NOTE_COUNT_p);
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                threads.push_back(std::jthread(
                    [this, notes, idx]() -> void
                    {
                        this->mode_tiers[idx] = mode_tier_t(notes, idx + 1);

                        this->scale_tiers[idx] = scale_tier_t(
                            this->mode_tiers[idx],
                            CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx],
                            idx + 1
                        );
                    }
                ));

                notes += CHROMATIC_TIER_MODE_NOTE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx];
            }
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                threads[idx].join();
            }
        }

        ~chromatic_t()
        {
            free(this->notes);
        }

    public:
        constexpr std::size_t
            Chromatic_Note_Count()
        {
            return CHROMATIC_NOTE_COUNT_p;
        }

        constexpr std::size_t
            Mode_Count()
        {
            return CHROMATIC_MODE_COUNTS[Chromatic_Note_Count() - 1];
        }

        std::size_t
            Scale_Count()
        {
            std::size_t count = 0;
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                count += this->scale_tiers[idx].scales.size();
            }

            return count;
        }

        std::vector<mode_t>
            Modes()
        {
            std::vector<mode_t> modes;
            modes.reserve(Mode_Count());

            for (std::size_t tier_idx = 0, tier_end = CHROMATIC_NOTE_COUNT_p;
                 tier_idx < tier_end;
                 tier_idx += 1) {
                const std::size_t tier_mode_count = CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][tier_idx];
                const std::size_t mode_note_count = tier_idx + 1;
                for (std::size_t mode_idx = 0, mode_end = tier_mode_count * mode_note_count;
                     mode_idx < mode_end;
                     mode_idx += mode_note_count) {
                    modes.push_back(mode_t(this->mode_tiers[tier_idx].notes + mode_idx, mode_note_count));
                }
            }

            return modes;
        }

        std::vector<scale_t>
            Scales()
        {
            std::vector<scale_t> scales;
            scales.reserve(Scale_Count());

            for (std::size_t tier_idx = 0, tier_end = CHROMATIC_NOTE_COUNT_p;
                 tier_idx < tier_end;
                 tier_idx += 1) {
                const scale_tier_t& scale_tier = this->scale_tiers[tier_idx];
                const std::size_t scale_note_count = tier_idx + 1;
                for (std::size_t scale_idx = 0, scale_end = scale_tier.scales.size();
                     scale_idx < scale_end;
                     scale_idx += 1) {
                    scales.push_back(scale_t(scale_tier.scales[scale_idx], scale_note_count));
                }
            }

            return scales;
        }

    public:
        void
            Print_Modes()
        {
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                this->mode_tiers[idx].Print_Modes(CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx], idx + 1);
            }
        }

        void
            Print_Scales()
        {
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                this->scale_tiers[idx].Print_Scales(idx + 1);
            }
        }
    };

    template <std::size_t idx = 0>
    void
        Print_Tests()
    {
        if constexpr (idx < MAX_CHROMATIC_NOTE_COUNT) {
            chromatic_t<idx + 1> chromatic;

            std::cout << "chromatic_note_count: " << idx + 1 << std::endl;
            std::cout << "chromatic_mode_count: " << chromatic.Mode_Count() << std::endl;
            //chromatic.Print_Modes();
            std::cout << "chromatic_scale_count: " << chromatic.Scale_Count() << std::endl;
            //chromatic.Print_Scales();
            std::cout << std::endl;

            Print_Tests<idx + 1>();
        }
    }

}

int
    main(void)
{
    musical_calculator::Print_Tests();

    return 0;
}
