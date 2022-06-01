[blank]: blank

# musical_calculator
A handful of helpful algorithms to help study the nature of music.

Back in the early 2010's, before I learned to program, I really wanted to know the answer to a particular question: "How many unique scales can be derived from our western 12 note chromatic scale?" My first guess was "probably in the millions." But to my surprise once I looked up how to calculate the permutations and more importantly the combinations of the 12 note scale I learned that it's significantly less.

With this insight I knew and some basic knowledge of music I knew that the answer to my question would significantly be even less than the combination count. One only needs the combinations of one key, and not all keys. That in itself cuts out a huge chunk, but there is something further. We don't even need all the combinations in one key, but only one combination per unique set of combinations. It's a little difficult to explain without going into the numbers, but essentially each combination can be revolved or rotated to derive a number of other combinations. For example, numerically writing out the diatonic scale allows one to derive 7 total unique modes, which are called Ionian, Dorian, Phrygian, Lydian, Mixolydian, Aeolian, and Locrian in common theory. These 7 modes combined account for one unique scale. However, as it turns out other combinations may derive themselves over and over again, and so it's not a simple matter to calculate how many combinations equate to a unique scale.

But with this knowledge in mind, I knew that the total number of unique scales had to be a significantly smaller number than I originally imagined, and because I had no idea how to write a program for the computer and barely any math skills appropriate to approach such a problem, I decided to do something completely ridiculous to resolve the answer of my original question.

I began writing each combination out by hand and eventually ended up typing them out one after another, after which I removed the repeats manually. Turns out that my work back then came out perfectly, because this program now proves that my original answer of 2048 modes with a tantalizingly memorable 351 unique scales derivable from the western 12 note chromatic scale was correct.

It may have taken several weeks to answer this question manually back then, but now this program can do it in the blink of an eye and it can do it for other chromatic scales besides the standard 12 note western scale.
