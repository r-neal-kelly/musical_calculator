/*
    Copyright 2022 r-neal-kelly
*/

#include <cassert>
#include <climits>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace musical_calculator {

    /*
        Gets the total number of modes found within a chromatic scale.
    */
    constexpr std::size_t
        Mode_Count(std::size_t chromatic_note_count)
    {
        assert(chromatic_note_count > 0);
        assert(chromatic_note_count <= sizeof(std::size_t) * CHAR_BIT);

        return std::size_t(1) << (chromatic_note_count - 1);
    }

    constexpr std::size_t
        Mode_Count()
    {
        return Mode_Count(12);
    }

    /*
        This gets all the possible modes of the musical scale.
        It only gathers modes for one key, the first note in the scale.

        Keep in mind that these are not scales but modes. Scales have more
        than one mode, e.g. the Diatonic scale has Ionian, Lydian, Aeolion, etc.

        mode_note_count:
            the size of each resultant mode.
        chromatic_note_count:
            the size of the chromatic mode/scale, i.e. the max number of notes.
        mode_cache:
            an optimzation to prevent the reallocation of a vector for repeated calls.
        modes_cache:
            the resultant modes, your return value. an optimization to prevent the reallocation of a vector for repeated calls.
    */
    void
        Modes(std::size_t mode_note_count,
              std::size_t chromatic_note_count,
              std::vector<std::size_t>& mode_cache,
              std::vector<std::vector<std::size_t>>& modes_cache)
    {
        assert(mode_note_count > 0);
        assert(mode_note_count <= chromatic_note_count);

        mode_cache.clear();
        mode_cache.reserve(mode_note_count);

        // the first mode is simply all the possible notes that can be taken
        // from the chromatic scale without any skips, up to the mode_note_count.
        for (std::size_t note = 1, last_note = mode_note_count; note <= last_note; note += 1) {
            mode_cache.push_back(note);
        }
        modes_cache.push_back(mode_cache);

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
                while (mode_cache[mode_note_count - 1] + 1 <= chromatic_note_count) {
                    mode_cache[mode_note_count - 1] += 1;
                    modes_cache.push_back(mode_cache);
                }

                // we need to find a greater place that can be incremented.
                // if it doesn't exist, then we are finished. we don't consider
                // idx 0 because we're only working with the one key, and so it
                // can never be more than 1, and is thus never incrementable.
                std::size_t next_idx = 0;
                bool found_next_idx = false;
                for (std::size_t idx = mode_cache.size() - 2; idx > 0; idx -= 1) {
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
                    modes_cache.push_back(mode_cache);

                    continue;
                } else {
                    break;
                }
            }
        }
    }

    void
        Modes(std::size_t chromatic_note_count,
              std::vector<std::size_t>& mode_cache,
              std::vector<std::vector<std::size_t>>& modes_cache)
    {
        mode_cache.reserve(chromatic_note_count);
        modes_cache.clear();
        modes_cache.reserve(Mode_Count(chromatic_note_count));

        for (std::size_t mode_note_count = 1, last_mode_note_count = chromatic_note_count;
             mode_note_count <= last_mode_note_count;
             mode_note_count += 1) {
            Modes(mode_note_count, chromatic_note_count, mode_cache, modes_cache);
        }
    }

    std::vector<std::vector<std::size_t>>
        Modes(std::size_t chromatic_note_count)
    {
        std::vector<std::size_t> mode;
        std::vector<std::vector<std::size_t>> modes;

        Modes(chromatic_note_count, mode, modes);

        return modes;
    }

    std::vector<std::vector<std::size_t>>
        Modes()
    {
        return Modes(12);
    }

    constexpr std::size_t
        Scale_Count()
    {
        return 351;
    }

    /*
        Prints out a vector of note sets, primarly for testing.
    */
    void
        Print_Note_Sets(const std::vector<std::vector<std::size_t>>& note_sets, std::string unit_name = "note_set")
    {
        std::cout << "total " + unit_name + " count: " << note_sets.size() << std::endl;

        for (std::size_t idx = 0, end = note_sets.size(); idx < end; idx += 1) {
            const std::vector<std::size_t>& note_set = note_sets.at(idx);
            std::string string;
            for (std::size_t idx = 0, end = note_set.size(); idx < end; idx += 1) {
                string += static_cast<char>(note_set.at(idx)) + '0';
            }
            std::cout << string << std::endl;
        }
        std::cout << std::endl;
    }

    /*
        Takes a mode and results array. Generates all revolutions of a single mode,
        thus achieving all modes of the same scale.
    */
    void
        To_Modes(const std::vector<std::size_t>& scale, std::vector<std::vector<std::size_t>>& results)
    {
        results.reserve(scale.size());
        results.push_back(scale);

        // it's easier if we track the state of the scale, and then we can just keep revolving it
        // to build up our revolutions.
        std::vector<std::size_t> revolution = scale;
        for (std::size_t idx = 1, end = scale.size(); idx < end; idx += 1) {
            // first, we revolve the actual notes as they appear.
            const std::size_t first_note = revolution[0];
            for (std::size_t idx = 0, end = revolution.size() - 1; idx < end; idx += 1) {
                revolution[idx] = revolution[idx + 1];
            }
            revolution[revolution.size() - 1] = first_note;

            // then for each subsequent note after the new first note, we add the total note count
            // and subtract by new_first_note - 1. we can then set the new first note to 1
            const std::size_t first_note_diff = revolution[0] - 1;
            revolution[revolution.size() - 1] += 12;
            for (std::size_t idx = 0, end = revolution.size(); idx < end; idx += 1) {
                revolution[idx] = revolution[idx] - first_note_diff;
            }

            results.push_back(revolution);
        }
    }

    /*
        Optionally creates the revolutions vector for you.
    */
    std::vector<std::vector<std::size_t>>
        To_Modes(const std::vector<std::size_t>& scale)
    {
        std::vector<std::vector<std::size_t>> modes;
        To_Modes(scale, modes);

        return modes;
    }

    /*
        This removes modal revolutions, e.g. [1, 2, 3], [1, 11, 12], and [1, 2, 12]
        which are all counted as the same scale. This has the effect of converting modes
        into a set of unique scales. In the above example, the latter two arrays are
        removed and [1, 2, 3] is kept because it proceeds either of the others in the
        passed in array. If that changes then the first occuring mode of a scale is kept.
    */
    std::vector<std::vector<std::size_t>>
        To_Scales(const std::vector<std::vector<std::size_t>>& modes)
    {
        auto Has_Scale = [](const auto& scales, const auto& mode, const std::size_t search_start_idx) -> bool
        {
            // we cache all the possible revolutions of the mode so that
            // we can quickly search them per scale in the scales vector.
            std::vector<std::vector<std::size_t>> revolutions;
            To_Modes(mode, revolutions);

            // now we can compare each scale starting from the first search index to see if it matches one of our revolutions.
            for (std::size_t scales_idx = search_start_idx, scales_end = scales.size(); scales_idx < scales_end; scales_idx += 1) {
                const std::vector<std::size_t>& scale = scales[scales_idx];
                for (std::size_t revolutions_idx = 0, revolutions_end = revolutions.size(); revolutions_idx < revolutions_end; revolutions_idx += 1) {
                    const std::vector<std::size_t>& revolution = revolutions[revolutions_idx];
                    assert(scale.size() == revolution.size());

                    bool scale_is_revolution = true;
                    for (std::size_t idx = 0, end = scale.size(); idx < end; idx += 1) {
                        if (scale[idx] != revolution[idx]) {
                            scale_is_revolution = false;
                            break;
                        }
                    }

                    if (scale_is_revolution) {
                        return true;
                    }
                }
            }

            return false;
        };

        std::vector<std::vector<std::size_t>> scales;
        scales.reserve(Scale_Count());

        std::size_t curr_take_count = 0;
        std::size_t curr_start_idx = 0;

        for (std::size_t idx = 0, end = modes.size(); idx < end; idx += 1) {
            const std::vector<std::size_t>& mode = modes[idx];

            // if this is the first mode of this take_count, then we simply add it
            // and track its position in the scales array. That way we don't have
            // to search the entire array when we compare it with its modes in Has_Scale.
            if (curr_take_count < mode.size()) {
                curr_take_count = mode.size();
                curr_start_idx = scales.size();
                scales.push_back(mode);
            } else {
                // we have to determine if one of the revolutions already exists in the scales vector.
                // if it doesn't then add this mode, else skip adding it.
                if (!Has_Scale(scales, mode, curr_start_idx)) {
                    scales.push_back(mode);
                }
            }
        }

        return scales;
    }

    /*
        A simple convenience. But if you need modes, it's more efficient to use To_Scales
    */
    std::vector<std::vector<std::size_t>>
        Scales()
    {
        return To_Scales(Modes());
    }

    void
        Print_Tests()
    {
        std::vector<std::vector<std::size_t>> modes = Modes();
        std::vector<std::vector<std::size_t>> scales = To_Scales(modes);

        Print_Note_Sets(modes, "mode");
        Print_Note_Sets(scales, "scale");

        std::vector<std::size_t> diatonic_scale = { 1, 3, 5, 6, 8, 10, 12 };
        std::vector<std::vector<std::size_t>> diatonic_modes = To_Modes(diatonic_scale);

        Print_Note_Sets(diatonic_modes, "diatonic mode");

        {
            for (std::size_t chromatic_note_count = 1, last_chromatic_note_count = 16;
                 chromatic_note_count <= last_chromatic_note_count;
                 chromatic_note_count += 1) {
                std::size_t mode_count = Modes(chromatic_note_count).size();
                assert(mode_count == Mode_Count(chromatic_note_count));

                std::string message =
                    "there are " +
                    std::to_string(mode_count) +
                    " modes in a " +
                    std::to_string(chromatic_note_count) +
                    " note chromatic scale.";
                std::cout << message << std::endl;
            }
        }
    }

}

int
    main(void)
{
    musical_calculator::Print_Tests();

    return 0;
}
