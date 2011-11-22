/*---------------------------------------------------------------------------
 * ADOM Sage - frontend for ADOM
 *
 * Copyright (c) 2002 Joshua Kelley; see LICENSE for details
 *
 * Header file for ADOM Sage containing STL-related stuff.  STL stuff is
 * somewhat involved, and STL details (such as use of hash_map) differ between
 * g++ and MSVC, so they're stuffed into here.
 */

#include <vector>
#include <string>
#include <ext/hash_map>
#include <list>
#include <map>
#include <deque>
#include <cstring>

using namespace std;
using namespace __gnu_cxx;

/*---------------------------------------------------------------------------
 * STL helpers - common
 */
struct ltstr
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) < 0;
    }
};

/*---------------------------------------------------------------------------
 * STL helpers - Linux g++
 */
#ifndef _WIN32

struct eqstr
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) == 0;
    }
};

struct eqcasestr
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcasecmp(s1, s2) == 0;
    }
};

#define STRING_HASH(t) hash_map<const char *, t, hash<const char *>, eqstr>
#define STRING_CASE_HASH(t) hash_map<const char *, t, hash<const char*>, \
    eqcasestr>


/*---------------------------------------------------------------------------
 * STL helpers - Microsoft Visual C++
 */
#else

// Specialized hash_compares modified from Andre Bremer's, off of Google Groups
class string_hash_compare
{
    public:
        enum
        {
            bucket_size = 4,
            min_buckets = 8
        };

        // hash algorithm taken from SGI's STL
        size_t operator()(const char *_Keyval) const
        {
            const char *__s = _Keyval;
            unsigned long __h = 0;

            for ( ; *__s; ++__s)
            {
                __h = 5 * __h + *__s;
            }

            return size_t(__h);
        }

        bool operator()(const char *_Keyval1, const char *_Keyval2) const
        {
            return strcmp(_Keyval1, _Keyval2) < 0;
        }
};

class string_case_hash_compare
{
    public:
        enum
        {
            bucket_size = 4,
            min_buckets = 8
        };

        // hash algorithm taken from SGI's STL
        size_t operator()(const char *_Keyval) const
        {
            const char *__s = _Keyval;
            unsigned long __h = 0;

            for ( ; *__s; ++__s)
            {
                __h = 5 * __h + *__s;
            }

            return size_t(__h);
        }

        bool operator()(const char *_Keyval1, const char *_Keyval2) const
        {
            return _stricmp(_Keyval1, _Keyval2) < 0;
        }
};

#define STRING_HASH(t) hash_map<const char *, t, string_hash_compare>
#define STRING_CASE_HASH(t) hash_map<const char *, t, string_case_hash_compare>

using namespace std;

#endif

