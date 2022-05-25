#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace musical_calculator {

    enum class note_e :
        std::uint8_t
    {
        NONE_lb,

        A,
        A$,
        B,
        C,
        C$,
        D,
        D$,
        E,
        F,
        F$,
        G,
        G$,

        MIN_lb = A,
        MAX_lb = G$,
    };

    /*
        We can cache this for the 12 note scale, because it's always going to be the same.
        However if we paramtize these functions, we'll need to calculate anything we don't know.
        It does help to keep this constexpr so that the ones we do know don't take any runtime.
    */
    constexpr std::size_t
        Mode_Count()
    {
        return 2048;
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
        This gets all the possible modes of the musical scale.
        It only gathers modes for one key, the first note in the scale.
        We may paramtize this to take the origin note (the key) and how many
        total notes are in the scale (default of 12).

        Keep in mind that these are not scales but modes. In order to get scales
        we would need to remove all the modal repeats, e.g. we would keep Ionian
        as the Diatonic scale and remove Dorian, Lydian, Aeolion, etc.

        It's important to understand that this does not return all the possible
        permutations of the notes involved, but only combinations.
        However it doesn not return all the total possible combinations.
        It would no longer be relevant as modes or scales. It would rather be
        melodies without repeats, in which order does not matter. Not very useful.
    */
    std::vector<std::vector<std::size_t>>
        Modes()
    {
        // we always include the octave up front, which we consider a mode.
        std::vector<std::vector<std::size_t>> modes;
        modes.reserve(Mode_Count());
        modes.push_back({ 1 });

        // we work through all the mode iterations up to and including the
        // chromatic scale, which like the octave is also considered a mode.
        for (std::size_t take_count = 2, take_total = 12; take_count <= take_total; take_count += 1) {
            // we need to cache the previous iterations of the notes for the sake of the algorithm.
            // it's relatively cheap to just copy these small vectors into the results anyway.
            std::vector<std::size_t> notes;
            notes.reserve(take_count);
            notes.push_back(1);

            // the first iteration is simply all the possible notes that can be taken
            // from the chromatic scale, without any skips.
            while (notes.size() < take_count) {
                notes.push_back(notes[notes.size() - 1] + 1);
            }
            modes.push_back(notes);

            // we then proceed to mutate the notes cache by incrementing each place
            // from the least significant digit to the most significant digit.
            // we do not increment a place past the value of the lesser place to its
            // right, thus we end up with combinations instead of permutations.
            while (true) {
                // get the least significant digit iterations first,
                // because we can't check a lesser place. Therefore it
                // can allow upto the total number of notes possible.
                while (notes[take_count - 1] + 1 <= 12) {
                    notes[take_count - 1] += 1;
                    modes.push_back(notes);
                }

                // we need to find a greater place that can be incremented.
                // if it doesn't exist, then we can proceed with the next batch
                // of a different take count. we don't consider idx 0 because
                // we're only working with the one key, and so it can never
                // be more than 1.
                std::size_t prev_idx = 0;
                bool found_prev_idx = false;
                for (std::size_t idx = notes.size() - 2; idx > 0; idx -= 1) {
                    if (notes[idx] + 1 < notes[idx + 1]) {
                        prev_idx = idx;
                        found_prev_idx = true;

                        break;
                    }
                }

                // if we did find it, we increment the place by one and reset
                // the lesser places for another iteration.
                if (found_prev_idx) {
                    notes[prev_idx] += 1;
                    for (std::size_t idx = prev_idx + 1, end = notes.size(); idx < end; idx += 1) {
                        notes[idx] = notes[idx - 1] + 1;
                    }
                    modes.push_back(notes);

                    continue;
                } else {
                    break;
                }
            }
        }

        return modes;
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
    }

}

int
    main(void)
{
    musical_calculator::Print_Tests();

    return 0;
}
