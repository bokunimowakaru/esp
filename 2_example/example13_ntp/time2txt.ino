/*********************************************************************
本ソースリストおよびソフトウェアは、
下記のソフトウェアを改変したものです。

出力フォーマット 19バイト+Null (20バイト)
0123456789012345678
2014/01/01,12:34:56

                               Copyright (c) 2014-2019 Wataru KUNINO
                               https://bokunimo.net/bokunimowakaru/
*********************************************************************/

/*
time.c - low level time and date functions
Copyright (c) Michael Margolis 2009

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

6  Jan 2010 - initial release 
12 Feb 2010 - fixed leap year calculation error
1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
*/

/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time serivces and are not normally needed in a sketch */

	// leap year calulator expects year argument as years offset from 1970
//	#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

//	static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

//	void breakTime(time_t time, tmElements_t &tm){
	// break the given time_t into time components
	// this is a more compact version of the C library localtime function
	// note that year is offset from 1970 !!!

#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )

void time2txt(char *date,unsigned long local){
	
	int Year,year;
	int Month,month, monthLength;
	int Day;
	int Second,Minute,Hour,Wday;  // Sunday is day 1 
	unsigned long days;
	static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};
	
	Second = local % 60;
	local /= 60; // now it is minutes
	Minute = local % 60;
	local /= 60; // now it is hours
	Hour = local % 24;
	local /= 24; // now it is days
	Wday = ((local + 4) % 7) + 1;  // Sunday is day 1 

	year = 0;  
	days = 0;
	while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= local) {
		year++;
	}
//	Year = year; // year is offset from 1970 

	days -= LEAP_YEAR(year) ? 366 : 365;
	local  -= days; // now it is days in this year, starting at 0

	days=0;
	month=0;
	monthLength=0;
	for (month=0; month<12; month++) {
		if (month==1) { // february
			if (LEAP_YEAR(year)) {
				monthLength=29;
			} else {
				monthLength=28;
			}
		} else {
			monthLength = monthDays[month];
		}

		if (local >= monthLength) {
			local -= monthLength;
		} else {
		    break;
		}
	}
	Year = year + 1970;
	Month = month + 1;  // jan is month 1  
	Day = local + 1;     // day of month
	
	sprintf(date,"%4d/%02d/%02d,%02d:%02d:%02d",Year,Month,Day,Hour,Minute,Second);
}
