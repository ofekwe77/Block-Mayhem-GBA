#include "bm_test.h"

#include <cstdarg>
#include <cstdlib>
#include <vector>

namespace bmtest
{
    namespace
    {
        std::vector<test_case>& registry()
        {
            static std::vector<test_case> tests;
            return tests;
        }

        int current_failures = 0;
        int total_checks_failed = 0;
    }

    void register_test(const char* suite, const char* name, test_fn fn)
    {
        registry().push_back({ suite, name, fn });
    }

    void fail(const char* file, int line, const char* expr)
    {
        ++current_failures;
        ++total_checks_failed;
        std::printf("    FAILED %s:%d: %s\n", file, line, expr);
    }

    void note(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        std::printf("    ");
        std::vprintf(fmt, args);
        std::printf("\n");
        va_end(args);
    }
}

int main(int argc, char** argv)
{
    const char* filter = argc > 1 ? argv[1] : nullptr;
    int passed = 0;
    int failed = 0;
    for(const bmtest::test_case& t : bmtest::registry())
    {
        if(filter && ! std::strstr(t.suite, filter) && ! std::strstr(t.name, filter))
        {
            continue;
        }
        bmtest::current_failures = 0;
        std::printf("[ RUN  ] %s.%s\n", t.suite, t.name);
        std::fflush(stdout);
        t.fn();
        if(bmtest::current_failures)
        {
            ++failed;
            std::printf("[ FAIL ] %s.%s\n", t.suite, t.name);
        }
        else
        {
            ++passed;
        }
    }
    std::printf("\n%d passed, %d failed, %d total\n", passed, failed, passed + failed);
    return failed ? 1 : 0;
}
