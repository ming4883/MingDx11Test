
#include "../lib/cutest/CuTest.h"

#include <stdio.h>

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

CuSuite* XprGetSuite()
{
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, TestNvpParser);
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