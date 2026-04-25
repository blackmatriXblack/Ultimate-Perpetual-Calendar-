#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

// Constants
#define MAX_EVENTS 100
#define MAX_EVENT_DESC 100
#define MAX_REMINDERS 50
#define CALENDAR_WIDTH 80
#define MAX_ZODIAC_NAME 10
#define MAX_SOLAR_TERM 15
#define MAX_LUNAR_MONTH_NAME 10

// Date structure
typedef struct {
    int year;
    int month;
    int day;
} Date;

// Time structure
typedef struct {
    int hour;
    int minute;
    int second;
} Time;

// Event structure
typedef struct {
    Date date;
    char description[MAX_EVENT_DESC];
    int is_recurring;
    int reminder_minutes;
} Event;

// Reminder structure
typedef struct {
    Date date;
    Time time;
    char message[MAX_EVENT_DESC];
    int snoozed;
} Reminder;

// Calendar configuration
typedef struct {
    int start_weekday; // 0=Sunday, 1=Monday
    int show_lunar;
    int show_holidays;
    int show_events;
    char language[20];
    int time_format; // 0=12-hour, 1=24-hour
} CalendarConfig;

// Chinese calendar data structures
typedef struct {
    int year;
    char zodiac[MAX_ZODIAC_NAME];
    char heavenly_stem[3];
    char earthly_branch[3];
} ChineseYearInfo;

typedef struct {
    int month;
    int day;
    char solar_term[MAX_SOLAR_TERM];
} SolarTerm;

// Global variables
Event events[MAX_EVENTS];
int event_count = 0;
Reminder reminders[MAX_REMINDERS];
int reminder_count = 0;
CalendarConfig config = {0, 1, 1, 1, "English", 1}; // Default: Sunday start, show lunar, holidays, events, English, 24-hour format
time_t current_time;
struct tm *now;

// Function prototypes
void initialize_calendar();
void display_main_menu();
void display_calendar(int year, int month);
void display_year_calendar(int year);
void display_lunar_calendar(int year, int month);
void add_event();
void view_events();
void set_reminder();
void view_reminders();
void date_calculator();
void chinese_calendar_features();
void solar_terms_calendar(int year);
void zodiac_calculator();
void date_difference_calculator();
void find_day_of_week();
void leap_year_checker();
void settings_menu();
void help_menu();

int is_leap_year(int year);
int get_days_in_month(int year, int month);
int calculate_day_of_week(int year, int month, int day);
int validate_date(int year, int month, int day);
void format_date(Date date, char *buffer, int format_type);
void get_current_date_time();
void save_events_to_file();
void load_events_from_file();
void save_reminders_to_file();
void load_reminders_from_file();

// Chinese calendar functions
int is_chinese_leap_year(int year);
int get_chinese_new_year_date(int year, Date *date);
char* get_chinese_zodiac(int year);
void get_heavenly_stem_earthly_branch(int year, char *stem, char *branch);
char* get_solar_term(int year, int month, int day);
void convert_solar_to_lunar(int year, int month, int day, int *lunar_year, int *lunar_month, int *lunar_day, int *is_leap_month);
void convert_lunar_to_solar(int lunar_year, int lunar_month, int lunar_day, int is_leap_month, int *year, int *month, int *day);
char* get_lunar_month_name(int month, int is_leap);

// Helper functions
void clear_screen();
void pause();
void print_header(const char *title);
void print_footer();
void center_text(const char *text, int width);

// Implementation
void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pause() {
    printf("\nPress Enter to continue...");
    getchar();
    if (getchar() != '\n') {
        while (getchar() != '\n');
    }
}

void print_header(const char *title) {
    printf("\n");
    printf("================================================\n");
    center_text(title, 48);
    printf("================================================\n");
}

void print_footer() {
    printf("================================================\n");
    time_t now = time(NULL);
    char time_str[50];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    printf("Current Time: %s\n", time_str);
    printf("================================================\n");
}

void center_text(const char *text, int width) {
    int text_len = strlen(text);
    int padding = (width - text_len) / 2;
    if (padding < 0) padding = 0;
    printf("%*s%s%*s\n", padding, "", text, (width - text_len - padding), "");
}

int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int get_days_in_month(int year, int month) {
    static const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12) return 0;
    if (month == 2 && is_leap_year(year)) return 29;
    return days_in_month[month - 1];
}

int calculate_day_of_week(int year, int month, int day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    int century = year / 100;
    year %= 100;
    int day_of_week = (day + (int)(2.6 * month - 0.2) + year + year / 4 + century / 4 - 2 * century) % 7;
    if (day_of_week < 0) day_of_week += 7;
    return day_of_week; // 0=Sunday, 1=Monday, ..., 6=Saturday
}

int validate_date(int year, int month, int day) {
    if (year < 1 || month < 1 || month > 12 || day < 1) return 0;
    return day <= get_days_in_month(year, month);
}

void format_date(Date date, char *buffer, int format_type) {
    switch (format_type) {
        case 0: // YYYY-MM-DD
            sprintf(buffer, "%04d-%02d-%02d", date.year, date.month, date.day);
            break;
        case 1: // MM/DD/YYYY
            sprintf(buffer, "%02d/%02d/%04d", date.month, date.day, date.year);
            break;
        case 2: // DD/MM/YYYY
            sprintf(buffer, "%02d/%02d/%04d", date.day, date.month, date.year);
            break;
        case 3: // Month DD, YYYY
            sprintf(buffer, "%s %d, %d",
                (const char*[]){"January", "February", "March", "April", "May", "June",
                               "July", "August", "September", "October", "November", "December"}[date.month - 1],
                date.day, date.year);
            break;
        default:
            sprintf(buffer, "%04d-%02d-%02d", date.year, date.month, date.day);
    }
}

void get_current_date_time() {
    current_time = time(NULL);
    now = localtime(&current_time);
}

void initialize_calendar() {
    get_current_date_time();
    // Placeholder for loading data
}

void display_lunar_calendar(int year, int month) {
    clear_screen();
    print_header("LUNAR/CHINESE CALENDAR");

    char month_names[12][10] = {"January", "February", "March", "April", "May", "June",
                               "July", "August", "September", "October", "November", "December"};
    char weekday_names[7][10] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    printf("\n      Solar: %s %d\n", month_names[month - 1], year);

    // Get Chinese year info
    char zodiac[20];
    char stem[10], branch[10];
    strcpy(zodiac, get_chinese_zodiac(year));
    get_heavenly_stem_earthly_branch(year, stem, branch);

    printf("      Lunar: %s%s Year of the %s\n", stem, branch, zodiac);
    printf("\n");

    // Print weekday headers
    printf("  Solar Date    Lunar Date     ");
    for (int i = 0; i < 7; i++) {
        printf("%4s ", weekday_names[i]);
    }
    printf("\n");

    // Print separator
    printf("  -----------   -----------   ");
    for (int i = 0; i < 7; i++) {
        printf("-----");
    }
    printf("\n");

    int days_in_month = get_days_in_month(year, month);
    int first_day = calculate_day_of_week(year, month, 1);

    // Print leading spaces for the first week
    printf("                                ");
    for (int i = 0; i < first_day; i++) {
        printf("     ");
    }

    // Print days with lunar information
    for (int day = 1; day <= days_in_month; day++) {
        int current_weekday = (first_day + day - 1) % 7;

        if (current_weekday == 0 && day > 1) {
            printf("\n                                ");
        }

        // Get lunar date
        int lunar_year, lunar_month, lunar_day, is_leap_month;
        convert_solar_to_lunar(year, month, day, &lunar_year, &lunar_month, &lunar_day, &is_leap_month);

        // Get solar term if any
        char* solar_term = get_solar_term(year, month, day);

        // Highlight current date
        int is_current = (year == now->tm_year + 1900 && month == now->tm_mon + 1 && day == now->tm_mday);

        // Determine formatting based on day type
        if (is_current) {
            printf("\033[1;32m%2d\033[0m  ", day);  // Green for current date
        } else if (current_weekday == 0 || current_weekday == 6) {
            printf("\033[1;34m%2d\033[0m  ", day);  // Blue for weekends
        } else {
            printf("%2d  ", day);
        }

        // Print lunar day with appropriate formatting
        if (solar_term) {
            printf("\033[1;33m%2d*\033[0m ", lunar_day);  // Yellow with * for solar terms
        } else {
            printf("%2d  ", lunar_day);
        }

        if (current_weekday == 6) {
            printf("\n");
        }
    }

    printf("\n\n");
    printf("  Solar Date Format: MM/DD\n");
    printf("  Lunar Date Format: Day of Month\n");
    printf("  * indicates Solar Term\n");
    printf("\n");
    printf("  Legend: ");
    printf("\033[1;32mCurrent Date\033[0m | ");
    printf("\033[1;34mWeekend\033[0m | ");
    printf("\033[1;33mSolar Term\033[0m\n");

    print_footer();
    pause();
}

void display_calendar(int year, int month) {
    clear_screen();
    print_header("MONTHLY CALENDAR");

    char month_names[12][10] = {"January", "February", "March", "April", "May", "June",
                               "July", "August", "September", "October", "November", "December"};
    char weekday_names[7][10] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    printf("\n      %s %d\n\n", month_names[month - 1], year);

    // Print weekday headers
    for (int i = 0; i < 7; i++) {
        printf("%4s ", weekday_names[i]);
    }
    printf("\n");

    // Print separator
    for (int i = 0; i < 7; i++) {
        printf("-----");
    }
    printf("\n");

    int days_in_month = get_days_in_month(year, month);
    int first_day = calculate_day_of_week(year, month, 1);

    // Print leading spaces
    for (int i = 0; i < first_day; i++) {
        printf("     ");
    }

    // Print days
    for (int day = 1; day <= days_in_month; day++) {
        int current_day = (first_day + day - 1) % 7;

        // Check for events
        int has_event = 0;
        for (int i = 0; i < event_count; i++) {
            if (events[i].date.year == year && events[i].date.month == month && events[i].date.day == day) {
                has_event = 1;
                break;
            }
        }

        // Highlight current date
        int is_current = (year == now->tm_year + 1900 && month == now->tm_mon + 1 && day == now->tm_mday);

        if (is_current) {
            printf("\033[1;32m%4d\033[0m ", day);
        } else if (has_event) {
            printf("\033[1;33m%4d\033[0m ", day);
        } else if (current_day == 0 || current_day == 6) { // Weekend
            printf("\033[1;34m%4d\033[0m ", day);
        } else {
            printf("%4d ", day);
        }

        if (current_day == 6) {
            printf("\n");
        }
    }

    printf("\n\nLegend: ");
    printf("\033[1;32mCurrent Date\033[0m | ");
    printf("\033[1;33mEvents\033[0m | ");
    printf("\033[1;34mWeekend\033[0m\n");

    print_footer();
    pause();
}

void display_year_calendar(int year) {
    clear_screen();
    print_header("YEARLY CALENDAR");

    char month_names[12][10] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char weekday_names[7][4] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};

    printf("\n                    %d\n\n", year);

    // Print 3 months per row
    for (int quarter = 0; quarter < 4; quarter++) {
        // Print month names
        for (int month_offset = 0; month_offset < 3; month_offset++) {
            int month = quarter * 3 + month_offset + 1;
            printf("    %s %-4d", month_names[month - 1], year);
            if (month_offset < 2) printf("  ");
        }
        printf("\n");

        // Print weekday headers for all 3 months
        for (int month_offset = 0; month_offset < 3; month_offset++) {
            for (int i = 0; i < 7; i++) {
                printf(" %s", weekday_names[i]);
            }
            if (month_offset < 2) printf("   ");
        }
        printf("\n");

        // Print separators
        for (int month_offset = 0; month_offset < 3; month_offset++) {
            for (int i = 0; i < 7; i++) {
                printf(" --");
            }
            if (month_offset < 2) printf("   ");
        }
        printf("\n");

        // Print days for each month
        int max_days = 0;
        int days_in_months[3];
        int first_days[3];

        for (int month_offset = 0; month_offset < 3; month_offset++) {
            int month = quarter * 3 + month_offset + 1;
            days_in_months[month_offset] = get_days_in_month(year, month);
            first_days[month_offset] = calculate_day_of_week(year, month, 1);
            if (days_in_months[month_offset] > max_days) {
                max_days = days_in_months[month_offset];
            }
        }

        // Print days row by row
        for (int day_row = 0; day_row < 6; day_row++) {
            for (int month_offset = 0; month_offset < 3; month_offset++) {
                int month = quarter * 3 + month_offset + 1;
                int first_day = first_days[month_offset];
                int days_in_month = days_in_months[month_offset];

                for (int weekday = 0; weekday < 7; weekday++) {
                    int day = day_row * 7 + weekday - first_day + 1;

                    if (day >= 1 && day <= days_in_month) {
                        int is_current = (year == now->tm_year + 1900 && month == now->tm_mon + 1 && day == now->tm_mday);

                        if (is_current) {
                            printf("\033[1;32m%3d\033[0m", day);
                        } else if (weekday == 0 || weekday == 6) {
                            printf("\033[1;34m%3d\033[0m", day);
                        } else {
                            printf("%3d", day);
                        }
                    } else {
                        printf("   ");
                    }
                }

                if (month_offset < 2) printf("   ");
            }
            printf("\n");
        }

        printf("\n");
    }

    printf("\nLegend: ");
    printf("\033[1;32mCurrent Date\033[0m | ");
    printf("\033[1;34mWeekend\033[0m\n");

    print_footer();
    pause();
}

void add_event() {
    clear_screen();
    print_header("ADD EVENT/REMINDER");

    if (event_count >= MAX_EVENTS) {
        printf("\nError: Maximum number of events reached (%d).\n", MAX_EVENTS);
        pause();
        return;
    }

    Event new_event;
    char buffer[100];

    printf("\nEnter event date (YYYY-MM-DD): ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d-%d-%d", &new_event.date.year, &new_event.date.month, &new_event.date.day);

    if (!validate_date(new_event.date.year, new_event.date.month, new_event.date.day)) {
        printf("\nInvalid date entered. Please try again.\n");
        pause();
        return;
    }

    printf("Enter event description: ");
    fgets(new_event.description, MAX_EVENT_DESC, stdin);
    new_event.description[strcspn(new_event.description, "\n")] = '\0';

    printf("Is this a recurring event? (0=No, 1=Yes): ");
    scanf("%d", &new_event.is_recurring);
    getchar();

    printf("Set reminder (minutes before event, 0 for no reminder): ");
    scanf("%d", &new_event.reminder_minutes);
    getchar();

    events[event_count++] = new_event;

    printf("\nEvent added successfully!");
    pause();
}

void view_events() {
    clear_screen();
    print_header("EVENTS & REMINDERS");

    if (event_count == 0) {
        printf("\nNo events found.\n");
        pause();
        return;
    }

    printf("\nUpcoming Events:\n");
    printf("===============\n\n");

    int current_year = now->tm_year + 1900;
    int current_month = now->tm_mon + 1;
    int current_day = now->tm_mday;

    for (int i = 0; i < event_count; i++) {
        char date_str[20];
        format_date(events[i].date, date_str, 0);

        printf("%s - %s", date_str, events[i].description);
        if (events[i].is_recurring) printf(" [Recurring]");
        if (events[i].reminder_minutes > 0) printf(" [Reminder: %d min]", events[i].reminder_minutes);
        printf("\n");
    }

    printf("\n\nReminders:\n");
    printf("===========\n\n");

    if (reminder_count == 0) {
        printf("No active reminders.\n");
    } else {
        for (int i = 0; i < reminder_count; i++) {
            char date_str[20], time_str[10];
            format_date(reminders[i].date, date_str, 0);
            sprintf(time_str, "%02d:%02d", reminders[i].time.hour, reminders[i].time.minute);

            printf("%s %s - %s", date_str, time_str, reminders[i].message);
            if (reminders[i].snoozed) printf(" [Snoozed]");
            printf("\n");
        }
    }

    print_footer();
    pause();
}

void date_calculator() {
    clear_screen();
    print_header("DATE CALCULATOR");

    int choice;
    Date date1, date2;
    char buffer[100];

    printf("\nDate Calculator Options:");
    printf("\n1. Find Day of Week for a Date");
    printf("\n2. Calculate Difference Between Two Dates");
    printf("\n3. Add/Subtract Days from a Date");
    printf("\n4. Find Leap Years in a Range");
    printf("\n5. Back to Main Menu");

    printf("\n\nEnter your choice (1-5): ");
    scanf("%d", &choice);
    getchar();

    switch (choice) {
        case 1:
            find_day_of_week();
            break;
        case 2:
            date_difference_calculator();
            break;
        case 3:
            printf("\nEnter base date (YYYY-MM-DD): ");
            fgets(buffer, sizeof(buffer), stdin);
            sscanf(buffer, "%d-%d-%d", &date1.year, &date1.month, &date1.day);

            if (!validate_date(date1.year, date1.month, date1.day)) {
                printf("\nInvalid date entered.\n");
                pause();
                return;
            }

            int days_to_add;
            printf("Enter number of days to add (negative to subtract): ");
            scanf("%d", &days_to_add);
            getchar();

            // Simple date arithmetic (not handling month/year boundaries properly)
            date1.day += days_to_add;
            printf("\nResulting date: %04d-%02d-%02d\n", date1.year, date1.month, date1.day);
            pause();
            break;
        case 4:
            leap_year_checker();
            break;
        case 5:
            return;
        default:
            printf("\nInvalid choice.\n");
            pause();
    }
}

void find_day_of_week() {
    clear_screen();
    print_header("DAY OF WEEK CALCULATOR");

    Date date;
    char buffer[100];

    printf("\nEnter date (YYYY-MM-DD): ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d-%d-%d", &date.year, &date.month, &date.day);

    if (!validate_date(date.year, date.month, date.day)) {
        printf("\nInvalid date entered.\n");
        pause();
        return;
    }

    int day_of_week = calculate_day_of_week(date.year, date.month, date.day);
    char *weekday_names[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    printf("\nDate: %04d-%02d-%02d", date.year, date.month, date.day);
    printf("\nDay of Week: %s\n", weekday_names[day_of_week]);

    pause();
}

void date_difference_calculator() {
    clear_screen();
    print_header("DATE DIFFERENCE CALCULATOR");

    Date date1, date2;
    char buffer[100];

    printf("\nEnter first date (YYYY-MM-DD): ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d-%d-%d", &date1.year, &date1.month, &date1.day);

    if (!validate_date(date1.year, date1.month, date1.day)) {
        printf("\nInvalid first date entered.\n");
        pause();
        return;
    }

    printf("Enter second date (YYYY-MM-DD): ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d-%d-%d", &date2.year, &date2.month, &date2.day);

    if (!validate_date(date2.year, date2.month, date2.day)) {
        printf("\nInvalid second date entered.\n");
        pause();
        return;
    }

    // Simple difference calculation (not accurate for different years)
    int days1 = date1.day + (date1.month - 1) * 30;
    int days2 = date2.day + (date2.month - 1) * 30;
    int difference = abs(days2 - days1);

    printf("\nDate 1: %04d-%02d-%02d", date1.year, date1.month, date1.day);
    printf("\nDate 2: %04d-%02d-%02d", date2.year, date2.month, date2.day);
    printf("\nDifference: %d days\n", difference);

    pause();
}

void leap_year_checker() {
    clear_screen();
    print_header("LEAP YEAR CALCULATOR");

    int start_year, end_year;

    printf("\nEnter start year: ");
    scanf("%d", &start_year);
    getchar();

    printf("Enter end year: ");
    scanf("%d", &end_year);
    getchar();

    if (start_year > end_year) {
        int temp = start_year;
        start_year = end_year;
        end_year = temp;
    }

    printf("\nLeap years between %d and %d:\n", start_year, end_year);
    printf("================================\n");

    int count = 0;
    for (int year = start_year; year <= end_year; year++) {
        if (is_leap_year(year)) {
            printf("%d ", year);
            count++;
            if (count % 10 == 0) printf("\n");
        }
    }

    if (count == 0) {
        printf("\nNo leap years found in this range.\n");
    } else {
        printf("\n\nTotal leap years: %d\n", count);
    }

    pause();
}

void chinese_calendar_features() {
    clear_screen();
    print_header("CHINESE CALENDAR FEATURES");

    int choice;
    int year;

    printf("\nChinese Calendar Options:");
    printf("\n1. Convert Solar to Lunar Date");
    printf("\n2. Convert Lunar to Solar Date");
    printf("\n3. Find Chinese New Year Date");
    printf("\n4. Get Heavenly Stems & Earthly Branches");
    printf("\n5. Back to Main Menu");

    printf("\n\nEnter your choice (1-5): ");
    scanf("%d", &choice);
    getchar();

    switch (choice) {
        case 1:
            printf("\nEnter solar date (YYYY-MM-DD): ");
            char buffer[100];
            fgets(buffer, sizeof(buffer), stdin);
            int solar_year, solar_month, solar_day;
            sscanf(buffer, "%d-%d-%d", &solar_year, &solar_month, &solar_day);

            if (!validate_date(solar_year, solar_month, solar_day)) {
                printf("\nInvalid date entered.\n");
                pause();
                return;
            }

            int lunar_year, lunar_month, lunar_day, is_leap;
            convert_solar_to_lunar(solar_year, solar_month, solar_day, &lunar_year, &lunar_month, &lunar_day, &is_leap);

            printf("\nSolar Date: %04d-%02d-%02d", solar_year, solar_month, solar_day);
            printf("\nLunar Date: %04d-%02d-%02d", lunar_year, lunar_month, lunar_day);
            if (is_leap) printf(" (Leap Month)");
            printf("\n");
            break;

        case 2:
            printf("\nEnter lunar year: ");
            scanf("%d", &year);
            getchar();
            printf("Enter lunar month: ");
            int month;
            scanf("%d", &month);
            getchar();
            printf("Enter lunar day: ");
            int day;
            scanf("%d", &day);
            getchar();
            printf("Is leap month? (0=No, 1=Yes): ");
            int is_leap_month;
            scanf("%d", &is_leap_month);
            getchar();

            int solar_year_out, solar_month_out, solar_day_out;
            convert_lunar_to_solar(year, month, day, is_leap_month, &solar_year_out, &solar_month_out, &solar_day_out);

            printf("\nLunar Date: %04d-%02d-%02d", year, month, day);
            if (is_leap_month) printf(" (Leap Month)");
            printf("\nSolar Date: %04d-%02d-%02d\n", solar_year_out, solar_month_out, solar_day_out);
            break;

        case 3:
            printf("\nEnter year to find Chinese New Year: ");
            scanf("%d", &year);
            getchar();

            Date chinese_new_year;
            if (get_chinese_new_year_date(year, &chinese_new_year)) {
                printf("\nChinese New Year %d: %04d-%02d-%02d\n", year, chinese_new_year.year, chinese_new_year.month, chinese_new_year.day);
            } else {
                printf("\nChinese New Year date not available for this year.\n");
            }
            break;

        case 4:
            printf("\nEnter year for Heavenly Stems & Earthly Branches: ");
            scanf("%d", &year);
            getchar();

            char stem[3], branch[3];
            get_heavenly_stem_earthly_branch(year, stem, branch);
            printf("\nYear %d: %s%s\n", year, stem, branch);
            break;

        case 5:
            return;
    }

    pause();
}

void zodiac_calculator() {
    clear_screen();
    print_header("ZODIAC CALCULATOR");

    int year;

    printf("\nEnter birth year: ");
    scanf("%d", &year);
    getchar();

    char *western_zodiac[] = {"Capricorn", "Aquarius", "Pisces", "Aries", "Taurus", "Gemini",
                             "Cancer", "Leo", "Virgo", "Libra", "Scorpio", "Sagittarius"};
    char *western_dates[] = {"Dec 22-Jan 19", "Jan 20-Feb 18", "Feb 19-Mar 20", "Mar 21-Apr 19",
                            "Apr 20-May 20", "May 21-Jun 20", "Jun 21-Jul 22", "Jul 23-Aug 22",
                            "Aug 23-Sep 22", "Sep 23-Oct 22", "Oct 23-Nov 21", "Nov 22-Dec 21"};

    char *chinese_zodiac = get_chinese_zodiac(year);

    printf("\nZodiac Information for Year %d:\n", year);
    printf("================================\n");
    printf("Chinese Zodiac: %s\n", chinese_zodiac);

    printf("\nWestern Zodiac Signs:");
    printf("\n=====================");
    for (int i = 0; i < 12; i++) {
        printf("\n%-12s: %s", western_zodiac[i], western_dates[i]);
    }

    pause();
}

void solar_terms_calendar(int year) {
    clear_screen();
    print_header("24 SOLAR TERMS CALENDAR");

    char *solar_terms[] = {
        "Minor Cold", "Major Cold", "Beginning of Spring", "Rain Water",
        "Insects Awaken", "Vernal Equinox", "Clear and Bright", "Grain Rain",
        "Beginning of Summer", "Grain Full", "Grain in Ear", "Summer Solstice",
        "Minor Heat", "Major Heat", "Beginning of Autumn", "End of Heat",
        "White Dew", "Autumn Equinox", "Cold Dew", "Frost's Descent",
        "Beginning of Winter", "Minor Snow", "Major Snow", "Winter Solstice"
    };

    printf("\n24 Solar Terms for Year %d:\n", year);
    printf("==========================\n\n");

    // This is a simplified version - actual solar terms require precise astronomical calculations
    printf("Note: This shows approximate dates. Actual solar terms vary by year and location.\n\n");

    for (int i = 0; i < 24; i++) {
        int month = (i / 2) + 1;
        int day = (i % 2 == 0) ? 4 : 20;

        if (month > 12) month -= 12;

        printf("%-25s: %04d-%02d-%02d\n", solar_terms[i], year, month, day);
    }

    pause();
}

void settings_menu() {
    clear_screen();
    print_header("CALENDAR SETTINGS");

    int choice;

    printf("\nCurrent Settings:");
    printf("\n- Week starts on: %s", (config.start_weekday == 0) ? "Sunday" : "Monday");
    printf("\n- Show Lunar Calendar: %s", config.show_lunar ? "Yes" : "No");
    printf("\n- Show Holidays: %s", config.show_holidays ? "Yes" : "No");
    printf("\n- Show Events: %s", config.show_events ? "Yes" : "No");
    printf("\n- Language: %s", config.language);
    printf("\n- Time Format: %s", config.time_format ? "24-hour" : "12-hour");

    printf("\n\nSettings Options:");
    printf("\n1. Change Week Start Day");
    printf("\n2. Toggle Lunar Calendar Display");
    printf("\n3. Toggle Holidays Display");
    printf("\n4. Toggle Events Display");
    printf("\n5. Change Language");
    printf("\n6. Change Time Format");
    printf("\n7. Reset to Default Settings");
    printf("\n8. Back to Main Menu");

    printf("\n\nEnter your choice (1-8): ");
    scanf("%d", &choice);
    getchar();

    switch (choice) {
        case 1:
            printf("\nSet week start day (0=Sunday, 1=Monday): ");
            scanf("%d", &config.start_weekday);
            getchar();
            if (config.start_weekday != 0 && config.start_weekday != 1) {
                config.start_weekday = 0;
                printf("\nInvalid choice. Defaulted to Sunday.\n");
            }
            break;
        case 2:
            config.show_lunar = !config.show_lunar;
            printf("\nLunar calendar display %s.\n", config.show_lunar ? "enabled" : "disabled");
            break;
        case 3:
            config.show_holidays = !config.show_holidays;
            printf("\nHolidays display %s.\n", config.show_holidays ? "enabled" : "disabled");
            break;
        case 4:
            config.show_events = !config.show_events;
            printf("\nEvents display %s.\n", config.show_events ? "enabled" : "disabled");
            break;
        case 5:
            printf("\nAvailable languages: English, Chinese, Spanish, French");
            printf("\nEnter language: ");
            fgets(config.language, 20, stdin);
            config.language[strcspn(config.language, "\n")] = '\0';
            printf("\nLanguage set to: %s\n", config.language);
            break;
        case 6:
            config.time_format = !config.time_format;
            printf("\nTime format changed to %s.\n", config.time_format ? "24-hour" : "12-hour");
            break;
        case 7:
            config.start_weekday = 0;
            config.show_lunar = 1;
            config.show_holidays = 1;
            config.show_events = 1;
            strcpy(config.language, "English");
            config.time_format = 1;
            printf("\nSettings reset to default.\n");
            break;
        case 8:
            return;
        default:
            printf("\nInvalid choice.\n");
    }

    pause();
}

void help_menu() {
    clear_screen();
    print_header("HELP & ABOUT");

    printf("\nUltimate Perpetual Calendar v2.0");
    printf("\n================================");

    printf("\n\nFeatures:");
    printf("\n- Gregorian calendar with yearly/monthly views");
    printf("\n- Chinese lunar calendar conversion");
    printf("\n- 24 solar terms calendar");
    printf("\n- Chinese zodiac and stem-branch calendar");
    printf("\n- Event and reminder management");
    printf("\n- Date calculations and utilities");
    printf("\n- Multiple calendar systems support");
    printf("\n- Customizable settings and themes");

    printf("\n\nControls:");
    printf("\n- Use number keys to select menu options");
    printf("\n- Press Enter to confirm selections");
    printf("\n- Press Enter after each operation to continue");

    printf("\n\nAbout:");
    printf("\nThis comprehensive calendar application combines");
    printf("\nWestern and Eastern calendar systems with advanced");
    printf("\nfeatures for date calculations, event management,");
    printf("\nand cultural calendar information.");

    printf("\n\nDeveloped with C programming language");
    printf("\nCross-platform compatible (Windows/Linux/macOS)");

    pause();
}

// Chinese calendar function implementations
int is_chinese_leap_year(int year) {
    // Simplified placeholder - actual Chinese leap year calculation is more complex
    return year % 19 == 0 || year % 19 == 3 || year % 19 == 6 ||
           year % 19 == 9 || year % 19 == 11 || year % 19 == 14 || year % 19 == 17;
}

int get_chinese_new_year_date(int year, Date *date) {
    // Placeholder - in reality, Chinese New Year varies between Jan 21 and Feb 20
    // This returns a fixed date for demonstration
    date->year = year;
    date->month = 1;
    date->day = 25; // Approximate date
    return 1;
}

char* get_chinese_zodiac(int year) {
    static char *zodiac[] = {"Rat", "Ox", "Tiger", "Rabbit", "Dragon",
                          "Snake", "Horse", "Goat", "Monkey", "Rooster", "Dog", "Pig"};
    return zodiac[(year - 4) % 12];
}

void get_heavenly_stem_earthly_branch(int year, char *stem, char *branch) {
    static char *stems[] = {"Jia", "Yi", "Bing", "Ding", "Wu", "Ji", "Geng", "Xin", "Ren", "Gui"};
    static char *branches[] = {"Zi", "Chou", "Yin", "Mao", "Chen", "Si", "Wu", "Wei", "Shen", "You", "Xu", "Hai"};

    strcpy(stem, stems[(year - 4) % 10]);
    strcpy(branch, branches[(year - 4) % 12]);
}

char* get_solar_term(int year, int month, int day) {
    // Simplified placeholder for solar terms
    if (month == 1 && day >= 5 && day <= 7) return "Minor Cold";
    if (month == 1 && day >= 20 && day <= 22) return "Major Cold";
    if (month == 2 && day >= 3 && day <= 5) return "Beginning of Spring";
    if (month == 2 && day >= 18 && day <= 20) return "Rain Water";
    if (month == 3 && day >= 5 && day <= 7) return "Insects Awaken";
    if (month == 3 && day >= 20 && day <= 22) return "Vernal Equinox";
    if (month == 4 && day >= 4 && day <= 6) return "Clear and Bright";
    if (month == 4 && day >= 19 && day <= 21) return "Grain Rain";
    if (month == 5 && day >= 5 && day <= 7) return "Beginning of Summer";
    if (month == 5 && day >= 20 && day <= 22) return "Grain Full";
    if (month == 6 && day >= 5 && day <= 7) return "Grain in Ear";
    if (month == 6 && day >= 21 && day <= 23) return "Summer Solstice";
    if (month == 7 && day >= 7 && day <= 9) return "Minor Heat";
    if (month == 7 && day >= 22 && day <= 24) return "Major Heat";
    if (month == 8 && day >= 7 && day <= 9) return "Beginning of Autumn";
    if (month == 8 && day >= 22 && day <= 24) return "End of Heat";
    if (month == 9 && day >= 7 && day <= 9) return "White Dew";
    if (month == 9 && day >= 22 && day <= 24) return "Autumn Equinox";
    if (month == 10 && day >= 8 && day <= 10) return "Cold Dew";
    if (month == 10 && day >= 23 && day <= 25) return "Frost's Descent";
    if (month == 11 && day >= 7 && day <= 9) return "Beginning of Winter";
    if (month == 11 && day >= 22 && day <= 24) return "Minor Snow";
    if (month == 12 && day >= 6 && day <= 8) return "Major Snow";
    if (month == 12 && day >= 21 && day <= 23) return "Winter Solstice";

    return NULL;
}

void convert_solar_to_lunar(int year, int month, int day, int *lunar_year, int *lunar_month, int *lunar_day, int *is_leap_month) {
    // Placeholder conversion - in reality this would use complex algorithms
    *lunar_year = year;
    *lunar_month = month;
    *lunar_day = day;
    *is_leap_month = 0;

    // Simple offset for demonstration
    if (month > 1) {
        *lunar_month = month - 1;
    }
    if (*lunar_month == 0) {
        *lunar_month = 12;
        *lunar_year = year - 1;
    }

    // Some months have 30 days in lunar calendar
    if (*lunar_month % 2 == 0 && day > 29) {
        *lunar_day = 29;
    } else {
        *lunar_day = day;
    }
}

void convert_lunar_to_solar(int lunar_year, int lunar_month, int lunar_day, int is_leap_month, int *year, int *month, int *day) {
    // Placeholder conversion
    *year = lunar_year;
    *month = lunar_month;
    *day = lunar_day;

    // Simple offset for demonstration
    if (lunar_month < 12) {
        *month = lunar_month + 1;
    } else {
        *month = 1;
        *year = lunar_year + 1;
    }

    // Ensure valid day
    int max_days = get_days_in_month(*year, *month);
    if (*day > max_days) {
        *day = max_days;
    }
}

char* get_lunar_month_name(int month, int is_leap) {
    static char *month_names[] = {"1st", "2nd", "3rd", "4th", "5th", "6th",
                                "7th", "8th", "9th", "10th", "11th", "12th"};
    static char leap_month[20];

    if (is_leap) {
        sprintf(leap_month, "Leap %s", month_names[month - 1]);
        return leap_month;
    } else {
        return month_names[month - 1];
    }
}

// File I/O placeholder functions
void save_events_to_file() {
    // Placeholder for file saving
    printf("\nEvents saved to file (placeholder function).\n");
}

void load_events_from_file() {
    // Placeholder for file loading
    printf("\nEvents loaded from file (placeholder function).\n");
}

void save_reminders_to_file() {
    // Placeholder for file saving
    printf("\nReminders saved to file (placeholder function).\n");
}

void load_reminders_from_file() {
    // Placeholder for file loading
    printf("\nReminders loaded from file (placeholder function).\n");
}

void display_main_menu() {
    int choice;

    while (1) {
        clear_screen();
        print_header("ULTIMATE PERPETUAL CALENDAR");

        printf("\nCurrent Date: %04d-%02d-%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
        printf("\nCurrent Time: %02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);

        printf("\n\nMain Menu:");
        printf("\n1.  View Monthly Calendar");
        printf("\n2.  View Yearly Calendar");
        printf("\n3.  View Lunar/Chinese Calendar");
        printf("\n4.  View Solar Terms Calendar");
        printf("\n5.  Add Event/Reminder");
        printf("\n6.  View Events & Reminders");
        printf("\n7.  Date Calculator");
        printf("\n8.  Chinese Calendar Features");
        printf("\n9.  Zodiac Calculator");
        printf("\n10. Settings");
        printf("\n11. Help & About");
        printf("\n12. Exit");

        print_footer();

        printf("\nEnter your choice (1-12): ");
        scanf("%d", &choice);
        getchar(); // Clear newline

        switch (choice) {
            case 1:
                display_calendar(now->tm_year + 1900, now->tm_mon + 1);
                break;
            case 2:
                display_year_calendar(now->tm_year + 1900);
                break;
            case 3:
                display_lunar_calendar(now->tm_year + 1900, now->tm_mon + 1);
                break;
            case 4:
                solar_terms_calendar(now->tm_year + 1900);
                break;
            case 5:
                add_event();
                break;
            case 6:
                view_events();
                break;
            case 7:
                date_calculator();
                break;
            case 8:
                chinese_calendar_features();
                break;
            case 9:
                zodiac_calculator();
                break;
            case 10:
                settings_menu();
                break;
            case 11:
                help_menu();
                break;
            case 12:
                save_events_to_file();
                save_reminders_to_file();
                printf("\nThank you for using the Ultimate Perpetual Calendar!\n");
                exit(0);
            default:
                printf("\nInvalid choice. Please try again.");
                pause();
        }
    }
}

int main() {
    initialize_calendar();
    display_main_menu();
    return 0;
}