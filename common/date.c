/*
  计算指定日期加上固定天数对应的日期
*/
#include <stdio.h>

typedef struct DateTime;
{
	int year;
	int month;
	int day;
}DateTime;

void funcPrintDate(DateTime *date)
{
	fprint(stdout, "%d-%d-%d", date->year, date->month, date->day);
}

int
date2j(int y, int m, int d)
{
	int			julian;
	int			century;

	if (m > 2)
	{
		m += 1;
		y += 4800;
	}
	else
	{
		m += 13;
		y += 4799;
	}

	century = y / 100;
	julian = y * 365 - 32167;
	julian += y / 4 - century + century / 4;
	julian += 7834 * m / 256 + d;

	return julian;
}								/* date2j() */

void
j2date(int jd, int *year, int *month, int *day)
{
	unsigned int julian;
	unsigned int quad;
	unsigned int extra;
	int			y;

	julian = jd;
	julian += 32044;
	quad = julian / 146097;
	extra = (julian - quad * 146097) * 4 + 3;
	julian += 60 + quad * 3 + extra / 146097;
	quad = julian / 1461;
	julian -= quad * 1461;
	y = julian * 4 / 1461;
	julian = ((y != 0) ? (julian + 305) % 365 : (julian + 306) % 366) + 123;
	y += quad * 4;
	*year = y - 4800;
	quad = julian * 2141 / 65536;
	*day = julian - 7834 * quad / 256;
	*month = (quad + 10) % 12 + 1;

	return;
}	


int main()
{
	DateTime date1;
	DateTime date2;

	date1.year = 2019;
	date1.month = 9;
	date1.month = 21;

    int interval = 123;
	int len = date2j(data1.year, date1.month, date1.day) + interval;
	j2date(len, &date2.year, &date2.month, &date2.day);
	funcPrintDate(&date1);
	funcPrintDate(&date2);


}
