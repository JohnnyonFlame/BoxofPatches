// Signature scan utilities

#ifndef __SIG_H__
#define __SIG_H__

#include <cstdint>
#include <algorithm>
#include <vector>
#include <type_traits>

namespace sig {
    // Wrapper to transform string literals into something that can be passed as
    // a template parameter.
    // String literals can't be used as template parameters, but arrays with chars can,
    // this wrapper explodes the literals into arrays.
    template<int N>
    struct _str {
        char value[N];
        const int size = N;

        constexpr _str(const char (&str)[N]): value() {
            std::copy_n(str, N, value);
        }
    };

    // isxdigit isn't constexpr-able...
    constexpr static int ishex(char c) {
        return 
            ((c) >= 'a' && (c) <= 'f') ||
            ((c) >= 'A' && (c) <= 'F') ||
            ((c) >= '0' && (c) <= '9');
    }

    // get decoder index
    constexpr static int decno(char c) {
        return 
            ((c) >= 'x' && (c) <= 'z') ? c - 'x' + 2 :
            ((c) >= 'X' && (c) <= 'Z') ? c - 'X' + 2 : 0;
    }

    // same as ishex
    constexpr static uint8_t hex_to_int(char a, char b)
    {
        #define doit(c) ( \
            ((c) >= 'a' && (c) <= 'f') ? (c) - 'a' + 10: \
            ((c) >= 'A' && (c) <= 'F') ? (c) - 'A' + 10: \
                                         (c) - '0' \
        )

        return doit(a) << 4 | doit(b);
    }

    // Wrapper struct to copy back results from the lambda in a constexpr form.
    // This is necessary to ensure that the final state of our calculation is all available
    // in a neatly constexpr'd form, because otherwise, arguments and variables can't be considered 
    // constant expressions and won't be usable on template arguments and static_assert arguments.
    struct result_t {
        int state;
        int size;
        int col;
        
        using storage_type = uint8_t[2048];
        storage_type bytes;
        storage_type mask;

        constexpr result_t(int state, int size, int col, const storage_type &bytes, const storage_type &mask) 
        : state(state), size(size), col(col), bytes(), mask()
        {
            std::copy_n(bytes, sizeof(bytes), this->bytes);
            std::copy_n(mask, sizeof(mask), this->mask);
        }
    };

    // The actual compile-time built/guaranteed signature bytes and mask.
    template <int N>
    struct sig_t {
        const int size = N;
        uint8_t bytes[N];
        uint8_t mask[N]; //0 = match, 1 = any, 2 = decode

        template <typename T>
        constexpr sig_t(const T &t) {
            std::copy_n(t.bytes, N, bytes);
            std::copy_n(t.mask, N, mask);
        }

        uint8_t *scan(uint8_t *start, uint8_t *end) const {
            for (uint8_t *ptr = start; ptr < end - N + 1; ptr++) {
                int pos = 0;

                for (uint8_t *cont = ptr; cont < end; cont++) {
                    if (mask[pos] == 1 || bytes[pos] == *cont) {
                        if (++pos >= size)
                            return (uint8_t*)ptr;
                    } else {
                        break;
                    }
                }
            }

            return NULL;
        };
    };

    // Format and pass error around to display
    template <int COLUMN, int LAST_STATE = 0, _str pattern>
    constexpr static void SIG_PARSE_CHECK_ERROR()
    {
        static_assert(LAST_STATE != -1, "SIG Syntax: Expected hexadecimal or '?'.");
        static_assert(LAST_STATE != -2, "SIG Syntax: Expected hexadecimal.");
        static_assert(LAST_STATE != -3, "SIG Syntax: Expected '?'.");
        static_assert(LAST_STATE != -4, "SIG Syntax: Expected hexadecimal, decoder or '?'.");
        static_assert(LAST_STATE != -5, "SIG Syntax: Expected another of the same decoder.");
        static_assert(LAST_STATE != -100, "SIG Syntax: Unknown error.");
    }

    enum sig_type {
        SIG_AOB,
        SIG_DECODE
    };

    template <_str pattern, sig_type type>
    constexpr static auto sig_parse()
    {
        // The lambda is used in order to build the "result_t" wrapper, this guarantees
        // that all variables inside the struct are "constant expressions" compatible, and thus
        // we can peek at the final state and pattern size during compile time.
        constexpr auto ret = [&]() -> auto {
            #define head pattern.value[cur]
            char last = '\0';

            int state = 0;
            int size = 0;
            result_t::storage_type bytes = {};
            result_t::storage_type mask = {};
            
            int cur;
            for (cur = 0; head != '\0' && state >= 0; cur++) {
                switch(state) {
                //State 0: ..
                case 0:
                    if (ishex(head)) 
                        state = 1;             //X.
                    else if (head == '?') 
                        state = 2;             //?.
                    else if ((type == SIG_DECODE) && decno(head)) 
                        state = 3;             //D.
                    else if (head == ' ') 
                        state = 0;             //..
                    else state = (type == SIG_AOB) ? -1 : -4;
                    break;

                //State 1: X.
                case 1:
                    bytes[size++] = hex_to_int(last, head);
                    if (ishex(head)) 
                        state = 0; //XX
                    else state = -2;
                    break;

                //State 2: ?.
                case 2:
                    mask[size++] = 1;
                    if (head == '?') 
                        state = 0; //??
                    else state = -3;
                    break;
                
                //State 3: D.
                case 3:
                    mask[size++] = decno(head);
                    if (decno(head) == decno(last))
                        state = 0; //DD
                    else state = -5;
                    break;
                    
                // Unknown State
                default: 
                    state = -100;
                }

                last = head;
                if (state < 0)
                    break;
            }

            // This cannot compile yet due to it not "being a constant expression.":
            // static_assert(state == 0, "Invalid SIG Syntax");
            // - <source>: error: static_assert expression is not an integral constant expression
            // So, from here on out, state, size, bytes and mask will be wrapped in 'result_t' and
            // available for usage on "constant expressions" such as static_assert and etc on sig_parse.
            return result_t(state, size, cur, bytes, mask);
            #undef head
        }();

        // Warn users about malformed SIG syntaxes during compile time, now possible due to the 'result_t' wrapper.
        SIG_PARSE_CHECK_ERROR<ret.col, ret.state, pattern>();
        
        // Can also use the wrapper members as template parameters.
        return sig_t<ret.size>(ret);
    }

    // pattern: Pattern to be scanned, e.g.: sig::scan<"DE AD ?? EF">(...) will match both "DE AD BE EF" and "DE AD B0 EF"
    template <_str pattern, typename T>
    uint8_t *scan(T start, T end) {
        static constexpr auto sig = sig_parse<pattern, SIG_AOB>();
        return sig.scan((uint8_t *)start, (uint8_t*)end);
    }

    // pattern: Pattern to be scanned, e.g.: sig::scan<"DE AD ?? EF">(...) will match both "DE AD BE EF" and "DE AD B0 EF"
    template <_str pattern, typename T, typename U>
    uint8_t *scan(T &mmaps, U **map_out = NULL) {
        static constexpr auto sig = sig_parse<pattern, SIG_AOB>();
        for (auto &map: mmaps) {
            auto ret = sig.scan((uint8_t*)map.start, (uint8_t*)map.end);
            if (ret) {
                if (map_out)
                    *map_out = &map;
                return ret;
            }
        }

        return NULL;
    }

    template <_str pattern, typename X = int*, typename Y = int*, typename Z = int*> 
    int decode(uint8_t *ptr, X x = NULL, Y y = NULL, Z z = NULL)
    {
        uint64_t _x = 0;
        uint64_t _y = 0;
        uint64_t _z = 0;

        static constexpr auto sig = sig_parse<pattern, SIG_DECODE>();
        for (int i = sig.size - 1; i >= 0; i--) {
            if (sig.mask[i] == 1) {
                // ignore byte
                continue;
            }
            else if (sig.mask[i] > 1) {
                // decode byte
                switch (sig.mask[i]) {
                    case 2: _x = _x << 8 | ptr[i];
                    case 3: _y = _y << 8 | ptr[i];
                    case 4: _z = _z << 8 | ptr[i];
                }
            }
            else if (ptr[i] != sig.bytes[i]) {
                // check byte
                return 0;
            }
        }

        // return the decoded data
        if (x) *x = _x;
        if (y) *y = _y;
        if (z) *z = _z;

        return 1;
    }
}
#endif /* __SIG_H__ */