/*
    Copyright 2022 r-neal-kelly
*/

#pragma once

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace musical_calculator {

    using count_t   = std::size_t;
    using index_t   = std::size_t;

    using note_t    = std::size_t;

    class mode_t;
    class scale_t;

    template <count_t CHROMATIC_NOTE_COUNT_p>
    class mode_tier_t;
    template <count_t CHROMATIC_NOTE_COUNT_p>
    class scale_tier_t;

    template <count_t CHROMATIC_NOTE_COUNT_p>
    class chromatic_t;

}

namespace musical_calculator {

    // It becomes quite memory intensive to work with chromatic scales that have more than 24 notes.
    // Also placing a static limit allows us to more easily store static information that massively speeds
    // up the generation of all the patterns we're interested in, such as modes and scales.
    constexpr count_t MAX_CHROMATIC_NOTE_COUNT  = 24;

    // if (mode_note_count > 1)
    //     return (chromatic_note_count - 1) choose (mode_note_count - 1)
    // else
    //     return 1
    constexpr count_t CHROMATIC_TIER_MODE_COUNTS[MAX_CHROMATIC_NOTE_COUNT][MAX_CHROMATIC_NOTE_COUNT] = {
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
    constexpr count_t CHROMATIC_TIER_MODE_NOTE_COUNTS[MAX_CHROMATIC_NOTE_COUNT][MAX_CHROMATIC_NOTE_COUNT] = {
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

    // count_t(1) << (chromatic_note_count - 1)
    constexpr count_t CHROMATIC_MODE_COUNTS[MAX_CHROMATIC_NOTE_COUNT] = {
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
    constexpr count_t CHROMATIC_MODE_NOTE_COUNTS[MAX_CHROMATIC_NOTE_COUNT] = {
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

}

namespace musical_calculator {

    /*
        A mode is a unique combination of chromatic notes.
        It's a subset of all possible notes without regard to order or repetition.

        It's represented numerically by the chromatic notes, e.g.:
            (1 3 5)     is a tier 3 mode in any chromatic scale of 5 or more notes.
            (1 6 8 12)  is a tier 4 mode in any chromatic scale of 12 or more notes.
            (1 24)      is a tier 2 mode in any chromatic scale of 24 or more notes.

        Notice that each modes starts with 1. This is because we only need to calcuate
        one key, the patterns of which all other keys share in common.
    */
    class mode_t
    {
    public:
        static void Print(const note_t* const notes, const count_t note_count) noexcept;
        static void Print(const mode_t& mode) noexcept;
        static void Print(mode_t&& mode) noexcept;

    public:
        const note_t*   notes;
        count_t         note_count;

    public:
        mode_t(const note_t* const notes, const count_t note_count) noexcept;

    public:
        count_t         Note_Count() noexcept;
        const note_t*   Notes() noexcept;
        note_t          Note(index_t index) noexcept;

        void            Print() noexcept;

    public:
        note_t  operator [](index_t index) noexcept;
    };

}

namespace musical_calculator {

    /*
        A scale is any mode of a set of modes which can be derived from its other modes.
        For example the mode (1 3 5) has derived modes (3 5 1) and (5 1 3). Any one of
        these can represent its other modes. The set itself is considered the scale,
        and is programmatically representable by any one of its individual modes.

        Keep in mind that all modes are normalized to one key. Therefore the modes of (1 3 5)
        in a 12 note chromatic scale are:
        (1 3 5) -> (1 3 5)
        (3 5 1) -> (1 3 11)
        (5 1 3) -> (1 9 11)
    */
    class scale_t :
        public mode_t
    {
    public:
        scale_t(const note_t* const notes, const count_t note_count) noexcept;
    };

}

namespace musical_calculator {

    /*
        A mode tier contains all the modes of the same note-count found in a chromatic scale.
    */
    template <count_t CHROMATIC_NOTE_COUNT_p>
    class mode_tier_t
    {
    public:
        const note_t*   notes;

    public:
        mode_tier_t() noexcept;
        mode_tier_t(note_t* notes, const count_t mode_note_count) noexcept;

    public:
        void    Print_Modes(const count_t mode_count, const count_t mode_note_count) noexcept;
    };

}

namespace musical_calculator {

    /*
        A scale tier contains all scales of the same note-count found in a chromatic scale.
    */
    template <count_t CHROMATIC_NOTE_COUNT_p>
    class scale_tier_t
    {
    public:
        static void Scale_Modes(const note_t*   scale,
                                const count_t   scale_note_count,
                                note_t* const   results) noexcept;

    public:
        std::vector<const note_t*>  scales;

    public:
        scale_tier_t() noexcept;
        scale_tier_t(const mode_tier_t<CHROMATIC_NOTE_COUNT_p>& mode_tier,
                     const count_t                              mode_count,
                     const count_t                              mode_note_count);

    public:
        void    Print_Scales(const count_t scale_note_count) noexcept;
    };

}

namespace musical_calculator {

    /*
        A chromatic is a set of a possible notes,
        a set of all possible modes, or combination of the notes in one key,
        and a set of all possible unique scales, or unique sets of modes which have members that derive each other.

        For example, the standard Diatonic scale (C D E F G A B) is derived from a 12 note chromatic scale.
        It can be represented with the numbers (1 3 5 6 8 10 12)
        and it has seven modes: Ionian, Dorian, Phrygian, Lydian, Mixolydian, Aeolian, and Locrian,
        all of which can be represented numerically and can derive one another.

        One of the most interesting questions this program answers is, how many unique scales
        can be derived from the chromatic? In the future I will be expanding this to answer even more
        interesting questions that I had when I began developing my music theory about a decade ago.
        Eventually it will be useful in generating patterns that can be learned to assist with naturally
        good sounding ways to change key, mode, scale, and even chromatics.
    */
    template <count_t CHROMATIC_NOTE_COUNT_p>
    class chromatic_t
    {
    public:
        static_assert(CHROMATIC_NOTE_COUNT_p <= MAX_CHROMATIC_NOTE_COUNT);

    public:
        note_t*                                 notes;
        mode_tier_t<CHROMATIC_NOTE_COUNT_p>     mode_tiers[CHROMATIC_NOTE_COUNT_p];
        scale_tier_t<CHROMATIC_NOTE_COUNT_p>    scale_tiers[CHROMATIC_NOTE_COUNT_p];

    public:
        chromatic_t();
        ~chromatic_t() noexcept;

    public:
        constexpr count_t       Chromatic_Note_Count() noexcept;
        constexpr count_t       Mode_Count() noexcept;
        count_t                 Scale_Count() noexcept;

        std::vector<mode_t>     Modes();
        std::vector<scale_t>    Scales();

    public:
        void    Print_Modes() noexcept;
        void    Print_Scales() noexcept;
    };

}

#include "musical_calculator.inl"
