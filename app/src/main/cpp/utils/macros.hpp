#ifndef CROSSWORD_HELPER_MACROS_HPP
#define CROSSWORD_HELPER_MACROS_HPP

#define LIKELY(X) __builtin_expect((X), 1)
#define UNLIKELY(X) __builtin_expect((X), 0)

#endif // CROSSWORD_HELPER_MACROS_HPP