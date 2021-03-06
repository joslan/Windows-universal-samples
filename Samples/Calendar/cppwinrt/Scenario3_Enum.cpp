//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "Scenario3_Enum.h"
#include "Scenario3_Enum.g.cpp"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Globalization;
using namespace Windows::Globalization::DateTimeFormatting;
using namespace Windows::UI::Xaml;

namespace winrt::SDKTemplate::implementation
{
    Scenario3_Enum::Scenario3_Enum()
    {
        InitializeComponent();
    }

    void Scenario3_Enum::ShowResults_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // This scenario uses the Windows.Globalization.Calendar class to enumerate through a calendar and
        // perform calendar math
        std::wostringstream results;

        results <<
            L"The number of years in each era of the Japanese era calendar is not regular. " <<
            L"It is determined by the length of the given imperial era:\n";

        // Create Japanese calendar.
        Calendar calendar({ L"en-US" }, CalendarIdentifiers::Japanese(), ClockIdentifiers::TwentyFourHour());

        // Enumerate all supported years in all supported Japanese eras.
        for (calendar.Era(calendar.FirstEra()); true; calendar.AddYears(1))
        {
            // Process current era.
            results << L"Era " << std::wstring_view(calendar.EraAsString()) <<
                L" contains " << calendar.NumberOfYearsInThisEra() <<
                L" year(s)\n";

            // Enumerate all years in this era.
            for (calendar.Year(calendar.FirstYearInThisEra()); true; calendar.AddYears(1))
            {
                // Begin sample processing of current year.

                // Move to first day of year. Change of month can affect day so order of assignments is important.
                calendar.Month(calendar.FirstMonthInThisYear());
                calendar.Day(calendar.FirstDayInThisMonth());

                // Set time to midnight (local).
                calendar.Period(calendar.FirstPeriodInThisDay());    // All days have 1 or 2 periods depending on clock type
                calendar.Hour(calendar.FirstHourInThisPeriod());     // Hours start from 12 or 0 depending on clock type
                calendar.Minute(0);
                calendar.Second(0);
                calendar.Nanosecond(0);

                if (calendar.Year() % 1000 == 0)
                {
                    results << L"\n";
                }
                else if (calendar.Year() % 10 == 0)
                {
                    results << L".";
                }

                // End sample processing of current year.

                // Break after processing last year.
                if (calendar.Year() == calendar.LastYearInThisEra())
                {
                    break;
                }
            }
            results << L"\n";

            // Break after processing last era.
            if (calendar.Era() == calendar.LastEra())
            {
                break;
            }
        }
        results << L"\n";

        // This section shows enumeration through the hours in a day to demonstrate that the number of time units in a given period (hours in a day, minutes in an hour, etc.)
        // should not be regarded as fixed. With Daylight Saving Time and other local calendar adjustments, a given day may have not have 24 hours, and
        // a given hour may not have 60 minutes, etc.
        results <<
            L"The number of hours in a day is not constant. " <<
            L"The US calendar transitions from daylight saving time to standard time on 4 November 2012:\n\n";

        // Create a DateTimeFormatter to display dates
        DateTimeFormatter displayDate(L"longdate");

        // Create a Gregorian calendar for the US with 12-hour clock format
        Calendar currentCal({ L"en-US" }, CalendarIdentifiers::Gregorian(), ClockIdentifiers::TwentyFourHour(), L"America/Los_Angeles");

        // Set the calendar to a the date of the Daylight Saving Time-to-Standard Time transition for the US in 2012.
        // An easier way to set the calendar date and time is to set the currentCal.Year, Month, etc. properties.
        // However, we calculate it the complicated way to demonstrate interoperability between Calendar and Windows::Foundation::DateTime.
        // DST ends in the America/Los_Angeles time zone at 4 November 2012 02:00 PDT = 4 November 2012 09:00 UTC.
        SYSTEMTIME dstSystemTime;
        dstSystemTime.wYear = 2012;
        dstSystemTime.wMonth = 11;
        dstSystemTime.wDay = 4;
        dstSystemTime.wHour = 9;
        dstSystemTime.wMinute = 0;
        dstSystemTime.wSecond = 0;
        dstSystemTime.wMilliseconds = 0;
        FILETIME dstFileTime;
        SystemTimeToFileTime(&dstSystemTime, &dstFileTime);
        DateTime dstDateTime = clock::from_file_time(dstFileTime);
        currentCal.SetDateTime(dstDateTime);

        // Set the current calendar to one day before DST change. Create a second calendar for comparision and set it to one day after DST change.
        Calendar endDate = currentCal.Clone();
        currentCal.AddDays(-1);
        endDate.AddDays(1);

        // Enumerate the day before, the day of, and the day after the 2012 DST-to-Standard time transition
        while (currentCal.Day() <= endDate.Day())
        {
            // Process current day.
            DateTime date = currentCal.GetDateTime();
            results << std::wstring_view(displayDate.Format(date)) << L" contains " << currentCal.NumberOfHoursInThisPeriod() << L" hour(s)\n";

            // Enumerate all hours in this day.
            // Create a calendar to represent the following day.
            Calendar nextDay = currentCal.Clone();
            nextDay.AddDays(1);
            for (currentCal.Hour(currentCal.FirstHourInThisPeriod()); true; currentCal.AddHours(1))
            {
                // Display the hour for each hour in the day.             
                results << std::wstring_view(currentCal.HourAsPaddedString(2)) << L" ";

                // Break upon reaching the next period (i.e. the first period in the following day).
                if (currentCal.Day() == nextDay.Day() && currentCal.Period() == nextDay.Period())
                {
                    break;
                }
            }
            results << L"\n";
        }

        // Display results
        OutputTextBlock().Text(results.str());
    }
}
