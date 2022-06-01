/*
    Copyright 2022 r-neal-kelly
*/

#include "musical_calculator.h"

namespace musical_calculator {

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
