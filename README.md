# Ultimate Perpetual Calendar v2.0 – Technical Documentation

## 1. Project Overview

**Ultimate Perpetual Calendar v2.0** is a sophisticated, cross-platform terminal application that bridges Western (Gregorian) and Eastern (Chinese Lunar) calendar systems. Developed in standard C, it provides a comprehensive suite of tools for date visualization, event management, cultural calendar conversion, and astronomical calculations (Solar Terms).

The application features a modular architecture with a robust input validation engine, ANSI-colored terminal UI, and persistent data storage capabilities. It serves as both a practical daily utility and an educational tool for understanding complex calendrical systems, including the 60-year Heavenly Stem and Earthly Branch cycle.

### 1.1 Core Objectives
*   **Cultural Integration:** Seamlessly integrate Gregorian and Chinese Lunar calendars, providing accurate conversions and cultural context (Zodiac, Solar Terms).
*   **Productivity Enhancement:** Offer event and reminder management with a simple, text-based interface.
*   **Educational Value:** Visualize complex concepts like Solar Terms and Lunar Leap Months through an interactive CLI.
*   **Cross-Platform Portability:** Ensure consistent behavior on Windows, Linux, and macOS using conditional compilation for system commands.

### 1.2 Key Features
*   **Dual-Calendar Display:** Monthly and yearly views that simultaneously show Solar dates and corresponding Lunar dates/Zodiac information.
*   **24 Solar Terms Engine:** Calculates and displays traditional Chinese solar terms (e.g., "Vernal Equinox", "Winter Solstice") for any given year.
*   **Event & Reminder System:** Allows users to add, view, and manage events with optional recurring flags and reminder timers.
*   **Date Utilities:** Includes calculators for day-of-week determination, date differences, and leap year verification.
*   **Customizable Settings:** Users can toggle lunar display, change week start days (Sunday/Monday), and switch between 12/24-hour time formats.

---

## 2. System Architecture

### 2.1 Data Structures

The application relies on several key structures to manage temporal data and configuration:

```c
typedef struct {
    int year;
    int month;
    int day;
} Date;

typedef struct {
    int hour;
    int minute;
    int second;
} Time;

typedef struct {
    Date date;
    char description[MAX_EVENT_DESC];
    int is_recurring;
    int reminder_minutes;
} Event;

typedef struct {
    int start_weekday; // 0=Sunday, 1=Monday
    int show_lunar;
    int show_holidays;
    int show_events;
    char language[20];
    int time_format; // 0=12-hour, 1=24-hour
} CalendarConfig;
```

*   **Global State:** The application uses global arrays `events[]` and `reminders[]` to store user data in memory during runtime.
*   **Configuration:** A global `CalendarConfig` struct maintains user preferences, which are reset to defaults upon restart (unless file I/O is implemented).

### 2.2 Module Structure

| Module | Function Name | Key Capabilities |
| :--- | :--- | :--- |
| **Core Loop** | `display_main_menu()` | Main dispatch loop, handles user navigation. |
| **Visualization** | `display_calendar()`, `display_year_calendar()` | Renders monthly/yearly grids with ANSI colors. |
| **Lunar Engine** | `display_lunar_calendar()`, `convert_solar_to_lunar()` | Handles Solar-Lunar conversion and Zodiac lookup. |
| **Solar Terms** | `solar_terms_calendar()`, `get_solar_term()` | Maps dates to the 24 traditional solar terms. |
| **Management** | `add_event()`, `view_events()` | CRUD operations for user events. |
| **Utilities** | `date_calculator()`, `zodiac_calculator()` | Date math and cultural zodiac lookups. |
| **Settings** | `settings_menu()` | Runtime configuration of display preferences. |

---

## 3. Detailed Module Analysis

### 3.1 Calendar Rendering Engine

The visualization modules (`display_calendar`, `display_year_calendar`) use a **Grid-Based Rendering Algorithm**:

1.  **Day of Week Calculation:** Uses Zeller’s Congruence or a similar algorithm (`calculate_day_of_week`) to determine the starting weekday of the month.
2.  **Padding:** Prints leading spaces for days before the 1st of the month.
3.  **Color Coding:**
    *   **Green (`\033[1;32m`)**: Current date.
    *   **Blue (`\033[1;34m`)**: Weekends (Saturday/Sunday).
    *   **Yellow (`\033[1;33m`)**: Days with scheduled events.
4.  **Lunar Overlay:** In `display_lunar_calendar`, each solar day cell is augmented with its lunar counterpart. Special markers (e.g., `*`) indicate Solar Terms.

### 3.2 Chinese Calendar Conversion Logic

The application implements a **Simplified Conversion Model** for Lunar dates.

*   **Zodiac Calculation:**
    ```c
    char* get_chinese_zodiac(int year) {
        static char *zodiac[] = {"Rat", "Ox", ...};
        return zodiac[(year - 4) % 12];
    }
    ```
*   **Heavenly Stems & Earthly Branches:**
    Uses modulo arithmetic on the year to determine the 60-year cycle components (e.g., "Jia-Zi").
*   **Solar Terms:**
    The `get_solar_term` function uses a lookup table based on approximate dates (e.g., March 20-22 for Vernal Equinox). *Note: For production-grade accuracy, this should be replaced with astronomical algorithms.*

### 3.3 Event Management System

*   **Storage:** Events are stored in a fixed-size array `Event events[MAX_EVENTS]`.
*   **Input Handling:** Uses `fgets` and `sscanf` to parse date strings (YYYY-MM-DD), ensuring robust handling of user input formats.
*   **Validation:** The `validate_date` function checks for logical consistency (e.g., February 30th is invalid) using `get_days_in_month`.

### 3.4 Date Calculator Utilities

*   **Day of Week:** Implements a standard algorithm to return 0-6 (Sunday-Saturday).
*   **Leap Year Check:**
    ```c
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    ```
*   **Date Difference:** Currently uses a simplified approximation (30 days/month). *Recommendation: Replace with Julian Day Number calculation for precision.*

---

## 4. User Interface & Experience

### 4.1 ANSI Color Palette
The application uses a consistent color scheme to enhance readability:

| Color | Code | Usage |
| :--- | :--- | :--- |
| **Green** | `\033[1;32m` | Current Date, Success Messages |
| **Blue** | `\033[1;34m` | Weekends, Headers |
| **Yellow** | `\033[1;33m` | Events, Solar Terms, Prompts |
| **Cyan** | `\033[1;36m` | Titles, Menu Options |
| **Red** | `\033[31m` | Errors, Exit Options |

### 4.2 Navigation Flow
1.  **Main Menu:** Presents 12 options ranging from "View Monthly Calendar" to "Settings".
2.  **Sub-Menus:** Specific features (e.g., Date Calculator) have their own nested menus.
3.  **Pause Mechanism:** After each operation, `pause()` waits for user input, preventing the screen from clearing immediately and allowing result review.

---

## 5. Compilation and Deployment

### 5.1 Prerequisites
*   **Compiler:** GCC, Clang, or MSVC.
*   **Standard Library:** Requires `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<time.h>`, `<ctype.h>`, `<math.h>`.
*   **OS:** Windows, Linux, or macOS.

### 5.2 Build Instructions

**On Linux/macOS:**
```bash
gcc main.c -o ultimate-calendar -lm
./ultimate-calendar
```

**On Windows (MinGW):**
```cmd
gcc main.c -o ultimate-calendar.exe
ultimate-calendar.exe
```

**On Windows (MSVC):**
```cmd
cl main.c
ultimate-calendar.exe
```

### 5.3 Cross-Platform Compatibility
The `clear_screen()` function uses preprocessor directives to ensure compatibility:
```c
#ifdef _WIN32
system("cls");
#else
system("clear");
#endif
```

---

## 6. Code Quality & Best Practices

### 6.1 Strengths
1.  **Modular Design:** Each feature is encapsulated in its own function, making the code easy to maintain.
2.  **Input Validation:** Extensive use of `validate_date` and buffer clearing prevents common crashes.
3.  **Visual Appeal:** Effective use of ANSI colors makes the terminal output engaging and easy to read.
4.  **Cultural Depth:** Inclusion of Lunar, Zodiac, and Solar Term data adds significant value over standard calendars.

### 6.2 Areas for Improvement
1.  **Lunar Accuracy:** The current `convert_solar_to_lunar` function is a placeholder. Integrating a library like `liblunar` or implementing the Shoushi Li algorithm would provide real-world accuracy.
2.  **File I/O:** `save_events_to_file` and `load_events_from_file` are currently placeholders. Implementing JSON or CSV serialization would make the event system persistent.
3.  **Date Math:** The `date_difference_calculator` uses a simplified 30-day month model. Using Julian Day Numbers would provide exact day counts.
4.  **Memory Safety:** Global arrays have fixed sizes (`MAX_EVENTS 100`). Dynamic allocation (linked lists) would allow unlimited events.

---

## 7. Future Roadmap

### 7.1 Short-Term Enhancements
*   **Persistent Storage:** Implement file I/O to save/load events and reminders.
*   **Accurate Lunar Conversion:** Integrate a precise astronomical algorithm for Lunar date calculation.
*   **Holiday Database:** Add a configurable list of national holidays for different countries.

### 7.2 Long-Term Vision
*   **GUI Frontend:** Port to GTK or Qt for a graphical interface with clickable dates.
*   **Network Sync:** Allow syncing events with Google Calendar or iCal via API.
*   **Astrological Features:** Add moon phase visualization and planetary positions.

---

## 8. Conclusion

**Ultimate Perpetual Calendar v2.0** is a robust, culturally rich, and visually appealing terminal application. It successfully bridges the gap between Western and Eastern calendrical systems while providing practical productivity tools. Its modular C architecture ensures portability and ease of extension. With improvements in lunar accuracy and data persistence, it has the potential to become a definitive open-source calendar tool for developers and cultural enthusiasts alike.
