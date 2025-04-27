# nobuild

I hate `Make` and `Cmake`, so I built my own C/C++ build system.

## What is capable of?
There’s still a lot to do, but we’ve reached a point where we can build simple projects that are as easy to build as running a command like `$CC -Wall -Wextra <...flags...> -o <exec name> main.c foo.c bar.c`, which is what `nobuild` is designed for.
Hovewer, `nobuild` can be further improved to include some form of dependency management. The necessary (I guess) data structures for this are already defined in `nobuild.h`; I just need to find some spare time to implement it.