
#include "../lib/cutest/CuTest.h"

#include <stdio.h>
#include "../lib/xprender/Platform.h"


#if defined(XPR_VC)
#	define RDTSC(low, high)	\
	__asm rdtsc				\
	__asm mov low, eax		\
	__asm mov high, edx
#elif defined(XPR_GCC)
#	define RDTSC(low, high)	\
	__asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
#endif

// mftbu in PowerPC: http://lists.apple.com/archives/mac-games-dev/2002/May/msg00244.html
uint64_t rdtsc()
{
	uint32_t l, h;
	RDTSC(l, h);
	return (((uint64_t)h) << 32) + l;
}

#include "../lib/xprender/NvpParser.h"

void TestNvpParser(CuTest *tc)
{
	const char* str = "name1=value1;name2=\"value 2\";";
	const char* name;
	const char* value;
	XprBool hasNext;
	XprNvpParser nvp;
	XprNvpParser_init(&nvp, str);

	hasNext = XprNvpParser_next(&nvp, &name, &value);

	CuAssertIntEquals(tc, XprTrue, hasNext);
	CuAssertStrEquals(tc, "name1", name);
	CuAssertStrEquals(tc, "value1", value);

	hasNext = XprNvpParser_next(&nvp, &name, &value);

	CuAssertIntEquals(tc, XprTrue, hasNext);
	CuAssertStrEquals(tc, "name2", name);
	CuAssertStrEquals(tc, "value 2", value);

	hasNext = XprNvpParser_next(&nvp, &name, &value);

	CuAssertIntEquals(tc, XprFalse, hasNext);
}

#include "../lib/xprender/StrHash.h"

void TestStrHash(CuTest *tc)
{
	const char* s1 = "u_worldViewMtx";
	const char* s2 = "u_worldViewProjMtx";

	CuAssertTrue(tc, XPR_HASH("u_worldViewMtx") != XPR_HASH("u_worldViewProjMtx"));
	CuAssertTrue(tc, XPR_HASH(s1) != XPR_HASH(s2));

	CuAssertTrue(tc, XPR_HASH("u_worldViewMtx") == XPR_HASH(s1));
	CuAssertTrue(tc, XPR_HASH("u_worldViewProjMtx") == XPR_HASH(s2));
}

void TestStrHashPerformance(CuTest *tc)
{
	uint64_t startTime;
	int cnt = 100000;
	int itCnt = 100;
	int i;
	int it;

	uint64_t t1 = 0;
	uint64_t t2 = 0;

	for(it = 0 ;it < itCnt; ++it) {
		startTime = rdtsc();
		for(i=0; i<cnt; ++i) {
			XprHashCode c1 = XPR_HASH("u_worldViewMtx");
			XprHashCode c2 = XPR_HASH("u_worldViewProjMtx");
			int result = c1 > c2 ? 1 : -1;
		}
		t1 += rdtsc() - startTime;
	}

	for(it = 0 ;it < itCnt; ++it) {
		startTime = rdtsc();
		for(i=0; i<cnt; ++i) {
			int result = strcmp("u_worldViewMtx", "u_worldViewProjMtx");
		}

		t2 += rdtsc() - startTime;
	}

	printf("XprStrHash done in %d ticks\n", t1 / itCnt);
	printf("strcmp     done in %d ticks\n", t2 / itCnt);
}

CuSuite* XprGetSuite()
{
	CuSuite* suite = CuSuiteNew();
	//SUITE_ADD_TEST(suite, TestNvpParser);
	SUITE_ADD_TEST(suite, TestStrHash);
	SUITE_ADD_TEST(suite, TestStrHashPerformance);
	return suite;
}

void RunAllTests()
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, XprGetSuite());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}

int main(int argc, char** argv)
{
	RunAllTests();
	
	return 0;
}