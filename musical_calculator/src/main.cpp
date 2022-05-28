/*
    Copyright 2022 r-neal-kelly
*/

#include <cassert>
#include <climits>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_set>
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
            mode_tier_t(note_t* notes) :
                notes(notes)
            {
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
            std::vector<const note_t*>  scales;

        public:
            scale_tier_t(const mode_tier_t& mode_tier,
                         const std::size_t  mode_count,
                         const std::size_t  mode_note_count)
            {
                // we do need to statically store the amount of unique scales for each chromatic also
            }
        };

    public:
        static void
            Write_Modes(note_t* notes,
                        const std::size_t mode_note_count)
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
        note_t*                     notes;
        std::vector<mode_tier_t>    mode_tiers;
        std::vector<scale_tier_t>   scale_tiers;

    public:
        chromatic_t()
        {
            this->notes = static_cast<note_t*>(malloc(sizeof(note_t) * CHROMATIC_MODE_NOTE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1]));
            assert(this->notes != nullptr);

            std::vector<std::jthread> threads;
            threads.reserve(CHROMATIC_NOTE_COUNT_p);
            note_t* notes = this->notes;
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                threads.push_back(std::jthread(
                    [notes, idx]() -> void
                    {
                        Write_Modes(notes, idx + 1);
                    }
                ));
                notes += CHROMATIC_TIER_MODE_NOTE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx];
            }
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                threads[idx].join();
            }

            this->mode_tiers.reserve(CHROMATIC_NOTE_COUNT_p);
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                this->mode_tiers.push_back({
                    this->notes + (CHROMATIC_TIER_MODE_NOTE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx] - 1) });
            }

            this->scale_tiers.reserve(CHROMATIC_NOTE_COUNT_p);
            for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                this->scale_tiers.push_back({
                    this->mode_tiers[idx],
                    CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx],
                    CHROMATIC_TIER_MODE_NOTE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx] });
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

    public:
        void
            Print_Modes()
        {
            /*for (std::size_t idx = 0, end = CHROMATIC_NOTE_COUNT_p; idx < end; idx += 1) {
                this->mode_tiers[idx].Print_Modes(CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][idx], idx + 1);
            }*/

            const note_t* note = this->notes;
            for (std::size_t tier_idx = 0, tier_end = CHROMATIC_NOTE_COUNT_p; tier_idx < tier_end; tier_idx += 1) {
                for (std::size_t mode_idx = 0, mode_end = CHROMATIC_TIER_MODE_COUNTS[CHROMATIC_NOTE_COUNT_p - 1][tier_idx]; mode_idx < mode_end; mode_idx += 1) {
                    std::string mode = "";
                    for (std::size_t note_idx = 0, note_end = tier_idx + 1; note_idx < note_end; note_idx += 1) {
                        mode.push_back('0' + static_cast<char>(*note));
                        note += 1;
                    }
                    std::cout << mode << std::endl;
                }
            }
        }
    };

    ///*
    //    Gets the number of modes found within a chromatic scale
    //    when the mode must be a certain number of notes long.
    //*/
    //constexpr std::size_t
    //    Mode_Tier_Count(const std::size_t chromatic_note_count,
    //                    const std::size_t mode_note_count)
    //{
    //    

    //    assert(mode_note_count > 0);
    //    assert(mode_note_count <= chromatic_note_count);
    //    assert(chromatic_note_count <= MAX_CHROMATIC_NOTE_COUNT);

    //    return CHROMATIC_MODE_TIER_COUNTS[chromatic_note_count - 1][mode_note_count - 1];
    //}

    ///*
    //    Gets the total number of modes found within a chromatic scale.
    //*/
    //constexpr std::size_t
    //    Mode_Count(const std::size_t chromatic_note_count)
    //{
    //    // I would like to investigate supporting chromatic scales longer than 64
    //    // but I can't help to think that it would rarely if every be useful
    //    // and would be little more than a curiousity at best. 64 is plenty for now.
    //    // However if we did support it, we would need to have a big integer type.
    //    assert(chromatic_note_count > 0);
    //    assert(chromatic_note_count <= MAX_CHROMATIC_NOTE_COUNT);
    //    assert(chromatic_note_count <= sizeof(std::size_t) * CHAR_BIT);

    //    return std::size_t(1) << (chromatic_note_count - 1);
    //}

    ///*
    //    This gets all the possible modes of the musical scale.
    //    It only gathers modes for one key, the first note in the scale.

    //    Keep in mind that these are not scales but modes. Scales have more
    //    than one mode, e.g. the Diatonic scale has Ionian, Lydian, Aeolion, etc.

    //    mode_note_count:
    //        the size of each resultant mode.
    //    chromatic_note_count:
    //        the size of the chromatic mode/scale, i.e. the max number of notes.
    //    mode_cache:
    //        an optimzation to prevent the reallocation of a vector for repeated calls.
    //    modes_cache:
    //        the resultant modes, your return value. an optimization to prevent the reallocation of a vector for repeated calls.
    //*/
    //void
    //    Modes(const std::size_t chromatic_note_count,
    //          const std::size_t mode_note_count,
    //          std::vector<std::size_t>& mode_cache,
    //          std::vector<std::vector<std::size_t>>& modes_cache)
    //{
    //    assert(mode_note_count > 0);
    //    assert(mode_note_count <= chromatic_note_count);
    //    assert(chromatic_note_count <= MAX_CHROMATIC_NOTE_COUNT);

    //    mode_cache.clear();
    //    mode_cache.reserve(mode_note_count);
    //    modes_cache.reserve(Mode_Count(chromatic_note_count, mode_note_count));

    //    // the first mode is simply all the possible notes that can be taken
    //    // from the chromatic scale without any skips, up to the mode_note_count.
    //    for (std::size_t note = 1, last_note = mode_note_count; note <= last_note; note += 1) {
    //        mode_cache.push_back(note);
    //    }
    //    modes_cache.push_back(mode_cache);

    //    // we never change the first place, so if that's all there is, we go ahead and return.
    //    if (mode_note_count > 1) {
    //        // we can now proceed to mutate the mode by incrementing each place
    //        // from the least significant digit to the most significant digit.
    //        // we do not increment a place past the value of the lesser place to its
    //        // right, thus we end up with combinations instead of permutations.
    //        while (true) {
    //            // get the least significant digit iterations first,
    //            // because we can't check a lesser place. Therefore it
    //            // can allow upto the total number of notes possible.
    //            while (mode_cache[mode_note_count - 1] + 1 <= chromatic_note_count) {
    //                mode_cache[mode_note_count - 1] += 1;
    //                modes_cache.push_back(mode_cache);
    //            }

    //            // we need to find a greater place that can be incremented.
    //            // if it doesn't exist, then we are finished. we don't consider
    //            // idx 0 because we're only working with the one key, and so it
    //            // can never be more than 1, and is thus never incrementable.
    //            std::size_t next_idx = 0;
    //            bool found_next_idx = false;
    //            for (std::size_t idx = mode_note_count - 2; idx > 0; idx -= 1) {
    //                if (mode_cache[idx] + 1 < mode_cache[idx + 1]) {
    //                    next_idx = idx;
    //                    found_next_idx = true;

    //                    break;
    //                }
    //            }

    //            // if we did find it, we increment the place by one and reset
    //            // the lesser places for another iteration of while(true).
    //            if (found_next_idx) {
    //                mode_cache[next_idx] += 1;
    //                for (std::size_t idx = next_idx + 1, end = mode_cache.size(); idx < end; idx += 1) {
    //                    mode_cache[idx] = mode_cache[idx - 1] + 1;
    //                }
    //                modes_cache.push_back(mode_cache);

    //                continue;
    //            } else {
    //                break;
    //            }
    //        }
    //    }
    //}

    //void
    //    Modes(const std::size_t chromatic_note_count,
    //          std::vector<std::size_t>& mode_cache,
    //          std::vector<std::vector<std::size_t>>& modes_cache)
    //{
    //    mode_cache.reserve(chromatic_note_count);
    //    modes_cache.clear();
    //    modes_cache.reserve(Mode_Count(chromatic_note_count));

    //    for (std::size_t mode_note_count = 1, last_mode_note_count = chromatic_note_count;
    //         mode_note_count <= last_mode_note_count;
    //         mode_note_count += 1) {
    //        Modes(chromatic_note_count, mode_note_count, mode_cache, modes_cache);
    //    }
    //}

    

    //void
    //    Modes_Parallel(const std::size_t chromatic_note_count,
    //                   std::vector<std::size_t>& mode_cache,
    //                   std::vector<std::vector<std::size_t>>& modes_cache)
    //{
    //    // because we have a limit on the chromatic_note_count,
    //    // we'll just spawn all threads at the same time and let the os
    //    // schedule everything for us automatically. this makes sense because
    //    // some of these threads have to do a lot more work than others, and
    //    // when an easy one finishes before a hard one, we might as well get
    //    // it started on another.

    //    // right now we're doing all of the tiers in their own threads, even ones
    //    // that are more expensive to do in a thread, such as the octave and chromatic modes.
    //    std::vector<std::jthread> threads;
    //    threads.reserve(chromatic_note_count);

    //    mode_cache.reserve(chromatic_note_count);

    //    // we go ahead and allocate everything so that the underlying algorithm merely
    //    // needs to fill in each mode's notes.
    //    const std::size_t mode_count = Mode_Count(chromatic_note_count);
    //    modes_cache.clear();
    //    modes_cache.reserve(mode_count);
    //    for (std::size_t mode = 0; mode < mode_count; mode += 1) {

    //    }

    //    

    //    // if we can calculate easily the amount of modes that in each tier then
    //    // we can make the array and hand out pointers to the threads to fill in for us.
    //    // as it is, we would have to do a lot of copying which may defeat the gains from
    //    // parallelization. another option would be to change the data-structure such
    //    // that it's a vector of vectors of tiers of vectors of notes.

    //    // I think to get this right we'll need to update the underlying Modes algorithm to
    //    // write to pointers for us. that means anything above it will have to make sure that
    //    // it has allocated the necessary memory, which is no longer a problem now that we know
    //    // the exact number of possible modes per tier, statically at that.
    //    // Probably want to keep the old algorithm around because that's how we determined the
    //    // numbers in the first place.
    //    std::vector<std::vector<std::size_t>*> pointers;
    //    pointers.reserve(modes_cache.size());

    //    for (std::size_t mode_note_count = 1, last_mode_note_count = chromatic_note_count;
    //         mode_note_count <= last_mode_note_count;
    //         mode_note_count += 1) {
    //        threads.push_back(std::jthread(
    //            []() -> void
    //            {
    //                //Modes(chromatic_note_count, mode_note_count, mode_cache, modes_cache);
    //            }
    //        ));
    //    }
    //}

    //std::vector<std::vector<std::size_t>>
    //    Modes(const std::size_t chromatic_note_count,
    //          const std::size_t mode_note_count)
    //{
    //    std::vector<std::size_t> mode;
    //    std::vector<std::vector<std::size_t>> modes;

    //    Modes(chromatic_note_count, mode_note_count, mode, modes);

    //    return modes;
    //}

    //std::vector<std::vector<std::size_t>>
    //    Modes(const std::size_t chromatic_note_count)
    //{
    //    std::vector<std::size_t> mode;
    //    std::vector<std::vector<std::size_t>> modes;

    //    Modes(chromatic_note_count, mode, modes);

    //    return modes;
    //}

    //constexpr std::size_t
    //    Scale_Count()
    //{
    //    // this needs to be updated to handle different chromatic scales, in fact the entire scale section does.
    //    return 351;
    //}

    ///*
    //    Prints out a vector of note sets, primarly for testing.
    //*/
    //void
    //    Print_Note_Sets(const std::vector<std::vector<std::size_t>>& note_sets, std::string unit_name = "note_set")
    //{
    //    std::cout << "total " + unit_name + " count: " << note_sets.size() << std::endl;

    //    for (std::size_t idx = 0, end = note_sets.size(); idx < end; idx += 1) {
    //        const std::vector<std::size_t>& note_set = note_sets.at(idx);
    //        std::string string;
    //        for (std::size_t idx = 0, end = note_set.size(); idx < end; idx += 1) {
    //            string += static_cast<char>(note_set.at(idx)) + '0';
    //        }
    //        std::cout << string << std::endl;
    //    }
    //    std::cout << std::endl;
    //}

    ///*
    //    Takes a mode and results array. Generates all revolutions of a single mode,
    //    thus achieving all modes of the same scale.
    //*/
    //void
    //    To_Modes(const std::vector<std::size_t>& scale, std::vector<std::vector<std::size_t>>& results)
    //{
    //    results.reserve(scale.size());
    //    results.push_back(scale);

    //    // it's easier if we track the state of the scale, and then we can just keep revolving it
    //    // to build up our revolutions.
    //    std::vector<std::size_t> revolution = scale;
    //    for (std::size_t idx = 1, end = scale.size(); idx < end; idx += 1) {
    //        // first, we revolve the actual notes as they appear.
    //        const std::size_t first_note = revolution[0];
    //        for (std::size_t idx = 0, end = revolution.size() - 1; idx < end; idx += 1) {
    //            revolution[idx] = revolution[idx + 1];
    //        }
    //        revolution[revolution.size() - 1] = first_note;

    //        // then for each subsequent note after the new first note, we add the total note count
    //        // and subtract by new_first_note - 1. we can then set the new first note to 1
    //        const std::size_t first_note_diff = revolution[0] - 1;
    //        revolution[revolution.size() - 1] += 12;
    //        for (std::size_t idx = 0, end = revolution.size(); idx < end; idx += 1) {
    //            revolution[idx] = revolution[idx] - first_note_diff;
    //        }

    //        results.push_back(revolution);
    //    }
    //}

    ///*
    //    Optionally creates the revolutions vector for you.
    //*/
    //std::vector<std::vector<std::size_t>>
    //    To_Modes(const std::vector<std::size_t>& scale)
    //{
    //    std::vector<std::vector<std::size_t>> modes;
    //    To_Modes(scale, modes);

    //    return modes;
    //}

    ///*
    //    This removes modal revolutions, e.g. [1, 2, 3], [1, 11, 12], and [1, 2, 12]
    //    which are all counted as the same scale. This has the effect of converting modes
    //    into a set of unique scales. In the above example, the latter two arrays are
    //    removed and [1, 2, 3] is kept because it proceeds either of the others in the
    //    passed in array. If that changes then the first occuring mode of a scale is kept.
    //*/
    //std::vector<std::vector<std::size_t>>
    //    To_Scales(const std::vector<std::vector<std::size_t>>& modes)
    //{
    //    auto Has_Scale = [](const auto& scales, const auto& mode, const std::size_t search_start_idx) -> bool
    //    {
    //        // we cache all the possible revolutions of the mode so that
    //        // we can quickly search them per scale in the scales vector.
    //        std::vector<std::vector<std::size_t>> revolutions;
    //        To_Modes(mode, revolutions);

    //        // now we can compare each scale starting from the first search index to see if it matches one of our revolutions.
    //        for (std::size_t scales_idx = search_start_idx, scales_end = scales.size(); scales_idx < scales_end; scales_idx += 1) {
    //            const std::vector<std::size_t>& scale = scales[scales_idx];
    //            for (std::size_t revolutions_idx = 0, revolutions_end = revolutions.size(); revolutions_idx < revolutions_end; revolutions_idx += 1) {
    //                const std::vector<std::size_t>& revolution = revolutions[revolutions_idx];
    //                assert(scale.size() == revolution.size());

    //                bool scale_is_revolution = true;
    //                for (std::size_t idx = 0, end = scale.size(); idx < end; idx += 1) {
    //                    if (scale[idx] != revolution[idx]) {
    //                        scale_is_revolution = false;
    //                        break;
    //                    }
    //                }

    //                if (scale_is_revolution) {
    //                    return true;
    //                }
    //            }
    //        }

    //        return false;
    //    };

    //    std::vector<std::vector<std::size_t>> scales;
    //    scales.reserve(Scale_Count());

    //    std::size_t curr_take_count = 0;
    //    std::size_t curr_start_idx = 0;

    //    for (std::size_t idx = 0, end = modes.size(); idx < end; idx += 1) {
    //        const std::vector<std::size_t>& mode = modes[idx];

    //        // if this is the first mode of this take_count, then we simply add it
    //        // and track its position in the scales array. That way we don't have
    //        // to search the entire array when we compare it with its modes in Has_Scale.
    //        if (curr_take_count < mode.size()) {
    //            curr_take_count = mode.size();
    //            curr_start_idx = scales.size();
    //            scales.push_back(mode);
    //        } else {
    //            // we have to determine if one of the revolutions already exists in the scales vector.
    //            // if it doesn't then add this mode, else skip adding it.
    //            if (!Has_Scale(scales, mode, curr_start_idx)) {
    //                scales.push_back(mode);
    //            }
    //        }
    //    }

    //    return scales;
    //}

    ///*
    //    A simple convenience. But if you need modes, it's more efficient to use To_Scales
    //*/
    //std::vector<std::vector<std::size_t>>
    //    Scales()
    //{
    //    return To_Scales(Modes(12));
    //}

    //void
    //    Print_Tests()
    //{
    //    std::vector<std::vector<std::size_t>> modes = Modes(12);
    //    std::vector<std::vector<std::size_t>> scales = To_Scales(modes);

    //    Print_Note_Sets(modes, "mode");
    //    Print_Note_Sets(scales, "scale");

    //    std::vector<std::size_t> diatonic_scale = { 1, 3, 5, 6, 8, 10, 12 };
    //    std::vector<std::vector<std::size_t>> diatonic_modes = To_Modes(diatonic_scale);

    //    Print_Note_Sets(diatonic_modes, "diatonic mode");

    //    {
    //        for (std::size_t chromatic_note_count = 1, last_chromatic_note_count = 16;
    //             chromatic_note_count <= last_chromatic_note_count;
    //             chromatic_note_count += 1) {
    //            std::size_t mode_count = Modes(chromatic_note_count).size();
    //            assert(mode_count == Mode_Count(chromatic_note_count));

    //            std::string message =
    //                "there are " +
    //                std::to_string(mode_count) +
    //                " modes in a " +
    //                std::to_string(chromatic_note_count) +
    //                " note chromatic scale.";
    //            std::cout << message << std::endl;
    //        }
    //    }

    //    std::cout << std::endl;

    //    {
    //        for (std::size_t chromatic_note_count = 1, last_chromatic_note_count = 16;
    //             chromatic_note_count <= last_chromatic_note_count;
    //             chromatic_note_count += 1) {
    //            for (std::size_t mode_note_count = 1, last_mode_note_count = chromatic_note_count;
    //                 mode_note_count <= last_mode_note_count;
    //                 mode_note_count += 1) {
    //                auto modes = Modes(chromatic_note_count, mode_note_count);
    //                assert(modes.size() == Mode_Count(chromatic_note_count, mode_note_count));
    //            }
    //        }
    //    }

    //    std::cout << std::endl;
    //}

    void
        Print_Tests()
    {
        chromatic_t<12> chromatic;
        chromatic.Print_Modes();

        /*for (std::size_t a = 0, end = MAX_CHROMATIC_NOTE_COUNT; a < end; a += 1) {
            std::cout << "{ ";
            for (std::size_t b = 0, end = a + 1; b < end; b += 1) {
                std::cout << std::to_string(CHROMATIC_TIER_MODE_COUNTS[a][b] * (b + 1)) << ", ";
            }
            std::cout << "}" << std::endl;
        }

        for (std::size_t a = 0, end = MAX_CHROMATIC_NOTE_COUNT; a < end; a += 1) {
            std::size_t result = 0;
            for (std::size_t b = 0, end = a + 1; b < end; b += 1) {
                result += CHROMATIC_TIER_MODE_NOTE_COUNTS[a][b];
            }
            std::cout << std::to_string(result) << "," << std::endl;
        }

        for (std::size_t a = 0, end = MAX_CHROMATIC_NOTE_COUNT; a < end; a += 1) {
            std::size_t result = std::size_t(1) << (a);
            std::cout << std::to_string(result) << "," << std::endl;
        }*/
    }

}

int
    main(void)
{
    musical_calculator::Print_Tests();

    return 0;
}
