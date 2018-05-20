#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/*
 * Helper functions for cpuset_in-set
 */
static char *cpuset_nexttok(const char *c)
{
	char *r = strchr(c+1, ',');
	if (r)
		return r+1;
	return NULL;
}

static int cpuset_getrange(const char *c, int *a, int *b)
{
	int ret;

	ret = sscanf(c, "%d-%d", a, b);
	return ret;
}

/*
 * cpusets are in format "1,2-3,4"
 * iow, comma-delimited ranges
 */
bool cpu_in_cpuset(int cpu, const char *cpuset)
{
	const char *c;

	for (c = cpuset; c; c = cpuset_nexttok(c)) {
		int a, b, ret;

		ret = cpuset_getrange(c, &a, &b);
		if (ret == 1 && cpu == a) // "1" or "1,6"
			return true;
		else if (ret == 2 && cpu >= a && cpu <= b) // range match
			return true;
	}

	return false;
}

void list2map(const char *cpuset, char *buf)
{
	int i, j;
	int max = 0;
	const char *c;
	char tmp[1024];

	const char HEX[] = "0123456789abcdef";

	for (c = cpuset; c; c = cpuset_nexttok(c)) {
		int a, b, ret;

		ret = cpuset_getrange(c, &a, &b);
		if (ret == 1) {
			tmp[a / 8] |= 1 << (a % 8);
			if (a > max)
				max = a;
		} else if (ret == 2) {
			for (i = a; i <= b; i++)
				tmp[i / 8] |= 1 << (i % 8);
			if (b > max)
				max = b;
		}
	}

	for (i = max / 8, j = 0; i >= 0; i--, j += 2) {
		buf[j] = HEX[tmp[i] & 0xf];
		buf[j+1] = HEX[(tmp[i] >> 4) & 0xf];
	}
	buf[max/4 + 1] = '\n';
	buf[max/4 + 2] = '\0';
}
