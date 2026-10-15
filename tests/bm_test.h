/*
 * BLOCK MAYHEM - minimal host-side test framework (no external dependencies).
 */
#ifndef BM_TEST_H
#define BM_TEST_H

#include <cstdio>
#include <cstring>
#include <string>

namespace bmtest
{
    using test_fn = void (*)();

    struct test_case
    {
        const char* suite;
        const char* name;
        test_fn fn;
    };

    void register_test(const char* suite, const char* name, test_fn fn);
    void fail(const char* file, int line, const char* expr);
    void note(const char* fmt, ...);

    struct registrar
    {
        registrar(const char* suite, const char* name, test_fn fn)
        {
            register_test(suite, name, fn);
        }
    };
}

#define BM_TEST(suite, name) \
    static void bmtest_##suite##_##name(); \
    static bmtest::registrar bmtest_reg_##suite##_##name(#suite, #name, bmtest_##suite##_##name); \
    static void bmtest_##suite##_##name()

#define CHECK(expr) \
    do { if(! (expr)) { bmtest::fail(__FILE__, __LINE__, #expr); } } while(0)

#define CHECK_EQ(a, b) \
    do { const auto bm_va = (a); const auto bm_vb = (b); if(! (bm_va == bm_vb)) { \
        char bm_msg[256]; std::snprintf(bm_msg, sizeof(bm_msg), "%s == %s  (%lld vs %lld)", #a, #b, \
        (long long) bm_va, (long long) bm_vb); bmtest::fail(__FILE__, __LINE__, bm_msg); } } while(0)

#define CHECK_STR(a, b) \
    do { const std::string bm_sa = (a); const std::string bm_sb = (b); if(bm_sa != bm_sb) { \
        std::string bm_msg = std::string(#a) + "\n      got:      " + bm_sa + "\n      expected: " + bm_sb; \
        bmtest::fail(__FILE__, __LINE__, bm_msg.c_str()); } } while(0)

#endif
