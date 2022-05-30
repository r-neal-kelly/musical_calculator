/*
    Copyright 2022 r-neal-kelly
*/

#include <cassert>
#include <climits>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace musical_calculator {

    // It becomes computationally expensive to calculate with pretty much any chromatic after 24.
    constexpr std::size_t MAX_CHROMATIC_NOTE_COUNT = 24;

    // if (mode_note_count > 1)
    //     return (chromatic_note_count - 1) choose (mode_note_count - 1)
    // else
    //     return 1
    constexpr std::size_t CHROMATIC_TIER_MODE_COUNTS[MAX_CHROMATIC_NOTE_COUNT][MAX_CHROMATIC_NOTE_COUNT] = {
        { 1 },
        { 1, 1 },
        { 1, 2, 1 },
        { 1, 3, 3, 1 },
        { 1, 4, 6, 4, 1 },
        { 1, 5, 10, 10, 5, 1 },
        { 1, 6, 15, 20, 15, 6, 1 },
        { 1, 7, 21, 35, 35, 21, 7, 1 },
        { 1, 8, 28, 56, 70, 56, 28, 8, 1 },
        { 1, 9, 36, 84, 126, 126, 84, 36, 9, 1 },
        { 1, 10, 45, 120, 210, 252, 210, 120, 45, 10, 1 },
        { 1, 11, 55, 165, 330, 462, 462, 330, 165, 55, 11, 1 },
        { 1, 12, 66, 220, 495, 792, 924, 792, 495, 220, 66, 12, 1 },
        { 1, 13, 78, 286, 715, 1287, 1716, 1716, 1287, 715, 286, 78, 13, 1 },
        { 1, 14, 91, 364, 1001, 2002, 3003, 3432, 3003, 2002, 1001, 364, 91, 14, 1 },
        { 1, 15, 105, 455, 1365, 3003, 5005, 6435, 6435, 5005, 3003, 1365, 455, 105, 15, 1 },
        { 1, 16, 120, 560, 1820, 4368, 8008, 11440, 12870, 11440, 8008, 4368, 1820, 560, 120, 16, 1 },
        { 1, 17, 136, 680, 2380, 6188, 12376, 19448, 24310, 24310, 19448, 12376, 6188, 2380, 680, 136, 17, 1 },
        { 1, 18, 153, 816, 3060, 8568, 18564, 31824, 43758, 48620, 43758, 31824, 18564, 8568, 3060, 816, 153, 18, 1 },
        { 1, 19, 171, 969, 3876, 11628, 27132, 50388, 75582, 92378, 92378, 75582, 50388, 27132, 11628, 3876, 969, 171, 19, 1 },
        { 1, 20, 190, 1140, 4845, 15504, 38760, 77520, 125970, 167960, 184756, 167960, 125970, 77520, 38760, 15504, 4845, 1140, 190, 20, 1 },
        { 1, 21, 210, 1330, 5985, 20349, 54264, 116280, 203490, 293930, 352716, 352716, 293930, 203490, 116280, 54264, 20349, 5985, 1330, 210, 21, 1 },
        { 1, 22, 231, 1540, 7315, 26334, 74613, 170544, 319770, 497420, 646646, 705432, 646646, 497420, 319770, 170544, 74613, 26334, 7315, 1540, 231, 22, 1 },
        { 1, 23, 253, 1771, 8855, 33649, 100947, 245157, 490314, 817190, 1144066, 1352078, 1352078, 1144066, 817190, 490314, 245157, 100947, 33649, 8855, 1771, 253, 23, 1 }
    };

    // map chromatic_tier_mode_counts[chromatic_note_count - 1][chromatic_tier_idx] *= (chromatic_tier_idx + 1)
    constexpr std::size_t CHROMATIC_TIER_MODE_NOTE_COUNTS[MAX_CHROMATIC_NOTE_COUNT][MAX_CHROMATIC_NOTE_COUNT] = {
        { 1 },
        { 1, 2 },
        { 1, 4, 3 },
        { 1, 6, 9, 4 },
        { 1, 8, 18, 16, 5 },
        { 1, 10, 30, 40, 25, 6 },
        { 1, 12, 45, 80, 75, 36, 7 },
        { 1, 14, 63, 140, 175, 126, 49, 8 },
        { 1, 16, 84, 224, 350, 336, 196, 64, 9 },
        { 1, 18, 108, 336, 630, 756, 588, 288, 81, 10 },
        { 1, 20, 135, 480, 1050, 1512, 1470, 960, 405, 100, 11 },
        { 1, 22, 165, 660, 1650, 2772, 3234, 2640, 1485, 550, 121, 12 },
        { 1, 24, 198, 880, 2475, 4752, 6468, 6336, 4455, 2200, 726, 144, 13 },
        { 1, 26, 234, 1144, 3575, 7722, 12012, 13728, 11583, 7150, 3146, 936, 169, 14 },
        { 1, 28, 273, 1456, 5005, 12012, 21021, 27456, 27027, 20020, 11011, 4368, 1183, 196, 15 },
        { 1, 30, 315, 1820, 6825, 18018, 35035, 51480, 57915, 50050, 33033, 16380, 5915, 1470, 225, 16 },
        { 1, 32, 360, 2240, 9100, 26208, 56056, 91520, 115830, 114400, 88088, 52416, 23660, 7840, 1800, 256, 17 },
        { 1, 34, 408, 2720, 11900, 37128, 86632, 155584, 218790, 243100, 213928, 148512, 80444, 33320, 10200, 2176, 289, 18 },
        { 1, 36, 459, 3264, 15300, 51408, 129948, 254592, 393822, 486200, 481338, 381888, 241332, 119952, 45900, 13056, 2601, 324, 19 },
        { 1, 38, 513, 3876, 19380, 69768, 189924, 403104, 680238, 923780, 1016158, 906984, 655044, 379848, 174420, 62016, 16473, 3078, 361, 20 },
        { 1, 40, 570, 4560, 24225, 93024, 271320, 620160, 1133730, 1679600, 2032316, 2015520, 1637610, 1085280, 581400, 248064, 82365, 20520, 3610, 400, 21 },
        { 1, 42, 630, 5320, 29925, 122094, 379848, 930240, 1831410, 2939300, 3879876, 4232592, 3821090, 2848860, 1744200, 868224, 345933, 107730, 25270, 4200, 441, 22 },
        { 1, 44, 693, 6160, 36575, 158004, 522291, 1364352, 2877930, 4974200, 7113106, 8465184, 8406398, 6963880, 4796550, 2728704, 1268421, 474012, 138985, 30800, 4851, 484, 23 },
        { 1, 46, 759, 7084, 44275, 201894, 706629, 1961256, 4412826, 8171900, 12584726, 16224936, 17577014, 16016924, 12257850, 7845024, 4167669, 1817046, 639331, 177100, 37191, 5566, 529, 24 }
    };

    // std::size_t(1) << (chromatic_note_count - 1)
    constexpr std::size_t CHROMATIC_MODE_COUNTS[MAX_CHROMATIC_NOTE_COUNT] = {
        1,
        2,
        4,
        8,
        16,
        32,
        64,
        128,
        256,
        512,
        1024,
        2048,
        4096,
        8192,
        16384,
        32768,
        65536,
        131072,
        262144,
        524288,
        1048576,
        2097152,
        4194304,
        8388608
    };

    // fold + chromatic_tier_mode_note_counts[chromatic_note_count - 1]
    constexpr std::size_t CHROMATIC_MODE_NOTE_COUNTS[MAX_CHROMATIC_NOTE_COUNT] = {
        1,
        3,
        8,
        20,
        48,
        112,
        256,
        576,
        1280,
        2816,
        6144,
        13312,
        28672,
        61440,
        131072,
        278528,
        589824,
        1245184,
        2621440,
        5505024,
        11534336,
        24117248,
        50331648,
        104857600
    };

    using note_t =
        std::size_t;

    class mode_t
    {
    public:
        const note_t*   notes;
        std::size_t     note_count;

    public:
        mode_t(const note_t* const  notes,
               const std::size_t    note_count) :
            notes(notes),
            note_count(note_count)
        {
        }

    public:
        std::size_t
            Note_Count()
        {
            return this->note_count;
        }

        const note_t*
            Notes()
        {
            return this->notes;
        }

        note_t
            Note(std::size_t index)
        {
            assert(index < Note_Count());

            return this->notes[index];
        }

    public:
        note_t
            operator [](std::size_t index)
        {
            return Note(index);
        }
    };

    class scale_t :
        public mode_t
    {
    public:
        scale_t(const note_t* const notes,
                const std::size_t   note_count) :
            mode_t(notes, note_count)
        {
        }
    };

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
                    std::string mode = "";
                    for (std::size_t idx = 0, end = mode_note_count; idx < end; idx += 1) {
                        mode.push_back('0' + static_cast<char>(note[idx]));
                    }
                    std::cout << mode << std::endl;
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
