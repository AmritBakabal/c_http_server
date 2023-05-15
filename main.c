#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "miti/include/miti.h"
#define MITIBUFFERSIZE 32
#include "die.h"

/**
 * @brief be careful to work with dates before 1970
 * parsing date before 1970 is unreliable.
 * We have to reset (hr, min, sec) of the `struct tm` after
 * parsing date (YYYY-MM-DD, etc.) from string buffer.
 */

/**
 * @brief struct for miti; year[1975-2100],
 * day[1-32], month[1-12], weekday[1-7]
 *
 */
struct miti
{
    int year;
    int mon;
    int day;
    int wday;
};

void miti_to_date(struct tm *date, struct miti *miti);
void date_to_miti(struct tm *date, struct miti *miti);
int is_leap_year(int year);
int diff_date(struct tm *d1, struct tm *d2);
void strpmiti(const char *miti_str, struct miti *miti_tm);
void strfmiti(char *buffer, size_t n, struct miti *miti, int enable_weekday);
int test_miti_validity(struct miti *miti);
void make_calendar(int mitiYear, int mitiMonth, int mitiDay, int totalDays,
                   int firstWeekDay);

// copy:

int test(int argc, char *argv[])
{
    time_t T = time(NULL);
    struct tm current_date;
    struct tm *date;
    char buffer[MITIBUFFERSIZE];
    int yr, mon, day;
    int is_valid_date = 1;
    // strptime("2010-10-23", "%Y-%m-%d", &current_date);

    // int status;
    // char *date_formats[] = {"%Y/%m/%d", "%Y-%m-%d","%Y.%m.%d"};

    struct miti *miti = (struct miti *)malloc(sizeof(struct miti));
    // printf("argc: %d\n", argc);
    // if(argc == 3) {
    //   printf("%s\n", argv[2]);
    // }
    // if (argc == 3 && strcmp(argv[2], "ad") == 0) {
    if (argc > 2 && strcmp(argv[1], "ad") == 0)
    {
        /**
         * @brief Taken command line argument as BS.
         *
         */
        sscanf(argv[2], "%d%*[^0-9]%d%*[^0-9]%d", &yr, &mon, &day);
        snprintf(buffer, MITIBUFFERSIZE, "%d-%d-%d", yr, mon, day);
        // printf("Buffer: %s\n", buffer);
        miti->year = yr;
        miti->mon = mon;
        miti->day = day;
        // char *ret = strptime(buffer, "%Y-%m-%d", &current_date);

        // status = test_miti_validity(miti);
        // printf("status = %d\n", status);
        if (test_miti_validity(miti) != 1)
        {
            fprintf(stderr, "Invalid or unsupported miti provided.\n");
            is_valid_date = 0;
        }
        // fprintf(stderr, "Invalid or unsupported")

        // if (ret == NULL) {
        //   printf("Error occured while parsing date\n");
        //   is_valid_date = 0;
        // }
        if (is_valid_date != 0)
        {
            date = (struct tm *)malloc(sizeof(struct tm));
            miti_to_date(date, miti);
            strftime(buffer, 32, "%Y-%m-%d", date);
            printf("AD: %s\n", buffer);
            free(date);
        }
    }
    else if (argc > 1 && strcmp(argv[1], "patro") == 0)
    {
        /**
         * @brief Show miti calendar for the given <year> <month>
         *
         */
        int current_day;
        if (argc == 4)
        {
            sscanf(argv[2], "%d", &miti->year);
            sscanf(argv[3], "%d", &miti->mon);
            miti->day = 1;
            current_day = 1;
        }
        else
        {
            T = time(NULL);
            localtime_r(&T, &current_date);
            // current_date = localtime_s(&T);
            // strftime(buffer, MITIBUFFERSIZE, "%Y-%m-%d-%u", &current_date);
            // printf("else date: %s\n", buffer);
            date_to_miti(&current_date, miti);
            // date = (struct tm *)malloc(sizeof(struct tm));
            current_day = miti->day;
            miti->day = 1;
        }
        if (test_miti_validity(miti) == 1)
        {
            date = (struct tm *)malloc(sizeof(struct tm));
            // printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
            // strfmiti(buffer, MITIBUFFERSIZE, miti, 1);
            // printf("miti: %s\n", buffer);
            // strftime(buffer, MITIBUFFERSIZE, "%Y-%m-%d-%u", date);
            // printf("date: %s\n", buffer);
            // printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
            miti_to_date(date, miti);
            // strftime(buffer, 32, "%Y-%m-%d", date);
            make_calendar(miti->year, miti->mon, current_day,
                          miti_table[miti->year - MITI_FIRST_YEAR][miti->mon - 1],
                          date->tm_wday);
            free(date);
        }
    }
    else if (argc == 2)
    {
        printf("Help:\n");
        printf("$ miti - Show today's miti\n");
        printf("$ miti ad <miti> - Convert given miti to AD\n");
        printf("$ miti bs <date> - Convert given date to BS\n");
        printf(
            "$ miti patro <year> <month> - Show Nepali calendar for given "
            "year/month\n");
    }
    else
    {
        if (argc == 1)
        {
            /**
             * @brief No cmd arguments; return today's miti
             *
             */
            T = time(NULL);
            current_date = *localtime(&T);
            // strptime("1996-09-28", "%Y-%m-%d", &current_date);
        }
        else if (argc > 2 && strcmp(argv[1], "bs") == 0)
        {
            /**
             * @brief Taken AD date from command line argument.
             * Accept dates with different types of separators
             * discard the separators with; %*[character class].
             * Any non-numeric character can be a separator.
             */
            sscanf(argv[2], "%d%*[^0-9]%d%*[^0-9]%d", &yr, &mon, &day);
            snprintf(buffer, MITIBUFFERSIZE, "%d-%d-%d", yr, mon, day);
            // printf("Buffer: %s\n", buffer);
            char *ret = strptime(buffer, "%Y-%m-%d", &current_date);
            if (ret == NULL)
            {
                printf("Error occured while parsing date\n");
                is_valid_date = 0;
            }
        }
        /**
         * @brief Conversion of AD to BS.
         *
         */
        if (is_valid_date != 0)
        {
            date_to_miti(&current_date, miti);
            strfmiti(buffer, MITIBUFFERSIZE, miti, 0);
            if (miti->year == 0)
            {
                fprintf(stderr, "Miti Out of Range. Cant be greater than 2100 BS.\n");
            }
            else
            {
                printf("BS: %d %s %d\n", miti->year, months[miti->mon - 1], miti->day);
            }
        }
    }
    // struct tm *date = miti_to_date()
    // struct miti miti;
    // strpmiti("2053-06-12", &miti);
    // current_date = miti_to_date(&miti);
    // strfmiti(buffer, 32, miti);
    // printf("Date: %s\n", buffer);
    // printf("%d-%d-%d-%d\n", miti->year, miti->mon, miti->day, miti->wday);
    free(miti);
    return 0;
}

/**
 * @brief `given_date_tm` is converted into BS and then stored in `struct miti
 * *miti`
 *
 * @param given_date_tm
 * @param miti
 */
void date_to_miti(struct tm *given_date_tm, struct miti *miti)
{
    // First day of the calendar database in `struct tm`.
    struct tm start_ad_tm;
    char buffer[MITIBUFFERSIZE];
    strptime(START_AD, "%Y-%m-%d", &start_ad_tm);

    // strftime(buffer, MITIBUFFERSIZE, "%Y-%m-%d-%u", given_date_tm);
    // printf("inside date_to_miti(): %s\n", buffer);
    /**
     * @brief Calculation of miti;
     * First, calculate the total # of days from a FIXED start date
     * to the given date.
     * Then, add the total days to a FIXED start miti to get
     * the final miti.
     */
    int remaining_days = diff_date(given_date_tm, &start_ad_tm);
    // printf("DIFF: ")

    /// @brief Initial miti year from month array
    int INIT_MITI_YEAR;
    // printf("total days: %d\n", remaining_days);

    strpmiti(START_BS, miti);
    miti->wday = given_date_tm->tm_wday + 1;
    INIT_MITI_YEAR = miti->year;

    int month_length;
    while (remaining_days > 0)
    {
        if (miti->year - INIT_MITI_YEAR >= YEAR_ARRAY_SIZE)
        {
            /**
             * @brief conversion date is out of range.
             * This program supports miti upto the year 2100 BS.
             * Set the miti as 0000-00-00-00 to indicate conversion failure.
             */
            miti->year = 0;
            miti->mon = 0;
            miti->day = 0;
            miti->wday = 0;
            remaining_days = 0;
            // printf("breaked");
            break;
        }
        // printf("total_days: %d\n", remaining_days);
        month_length = miti_table[miti->year - INIT_MITI_YEAR][miti->mon - 1];
        if (remaining_days + miti->day <= month_length)
        {
            /**
             * @brief add remaining few days to at most fill (but not overfill)
             * current month
             *
             */
            miti->day += remaining_days;
            remaining_days = 0;
            // printf("REM: %d, partially: %d")
        }
        else
        {
            /**
             * @brief current month filled; go to first day of next month
             *
             */
            remaining_days -= (month_length - miti->day + 1);
            miti->day = 1;
            miti->mon++;
            if (miti->mon > 12)
            {
                miti->mon = 1;
                miti->year++;
            }
        }
    }
}

void miti_to_date(struct tm *date, struct miti *miti)
{
    char buffer[32];
    // strfmiti(buffer, 32, miti, 0);
    // printf("Miti: %s\n", buffer);
    int diff_year = miti->year - MITI_FIRST_YEAR;
    int diff_month = miti->mon;
    int total_days = 0;
    // add all the days in the year since 1975 upto given miti year.
    for (int y = 0; y < diff_year; y++)
    {
        for (int m = 0; m < 12; m++)
        {
            total_days += miti_table[y][m];
        }
    }
    // printf("total_days: %d\n", total_days);
    // add all the days of the months of given year upto given month
    for (int m = 0; m < miti->mon - 1; m++)
    {
        total_days += miti_table[miti->year - MITI_FIRST_YEAR][m];
    }
    // finally, add days of the given miti
    total_days += miti->day - 1;
    // printf("total_days: %d\n", total_days);

    // struct tm *date = (struct tm *)malloc(sizeof(struct tm));
    strptime(START_AD, "%Y-%m-%d", date);
    // printf("year: %d mon: %d day: %d\n", date->tm_year, date->tm_mon,
    // date->tm_mday);
    date->tm_mday += total_days;
    /**
     * @brief
     * IMPORTANT !!!
     * Zero out the hour, min, sec of date because
     * the date is prior 1970; otherwise garbage result
     */
    date->tm_hour = 0;
    date->tm_min = 0;
    date->tm_sec = 0;
    mktime(date);
    // printf("year: %d mon: %d day: %d\n", date->tm_year, date->tm_mon,
    // date->tm_mday);
    miti->wday = date->tm_wday + 1;
    // strfmiti(buffer, 32, miti, 0);
    // printf("Miti: %s\n", buffer);
}

/**
 * @brief Returns 1 if given year is a leap year (366 days) else 0.
 *
 * @param year
 * @return int
 */
int is_leap_year(int year)
{
    return year % 100 == 0 && year % 400 == 0 || year % 100 != 0 && year % 4 == 0;
}

/**
 * @brief Returns the difference of two dates (struct tm *) in days.
 *
 * @param d1 struct tm *
 * @param d2 struct tm *
 * @return int
 */
int diff_date(struct tm *d1, struct tm *d2)
{
    int swap = 0;

    if (d1->tm_year == d2->tm_year)
    {
        swap = d1->tm_yday - d2->tm_yday;
    }
    else
        swap = d1->tm_year - d2->tm_year;
    if (swap < 0)
    {
        struct tm *temp = d2;
        d2 = d1;
        d1 = temp;
    }
    /// Here d1 = latter date, d2 = earlier date

    int total_days = 0;
    // add all the days of the years in between d1 and d2
    for (int year = d2->tm_year + 1; year < d1->tm_year; year++)
    {
        total_days += 365;
        if (is_leap_year(year + 1900))
            total_days++;
    }
    if (d1->tm_year == d2->tm_year)
    {
        total_days += d1->tm_yday - d2->tm_yday;
    }
    else
    {
        total_days += 364 - d2->tm_yday;
        if (is_leap_year(d2->tm_year + 1900))
            total_days++;
        total_days += d1->tm_yday + 1;
    }
    return total_days;
}

/**
 * @brief Prints a console calendar with colored saturdays
 *
 * @param mitiYear
 * @param mitiMonth
 * @param mitiDay
 * @param totalDays in a month
 * @param firstWeekDay
 */
void make_calendar(int mitiYear, int mitiMonth, int mitiDay, int totalDays,
                   int firstWeekDay)
{
    char headline[20];

    // printf("first_weekday: %d\n", firstWeekDay);

    // print headline
    printf("           %d %s\n", mitiYear, months[mitiMonth - 1]);
    printf("---------------------------------\n");
    // print weekdays
    // printf(" ");
    for (int i = 0; i < 7; i++)
    {
        if (i == 6)
        {
            printf("\033[1;31m");
        }
        printf("%s  ", weekDays[i]);
        printf("\033[0m");
    }
    printf("\n");

    // print first week (partial)
    int currentDay = 1;
    // adding 35 (multiples of 7) is equivalent to itself (mod 7)
    // int firstWeekDay = ((todayWeekDay + 35) - (mitiDay - 1)) % 7;
    for (int i = 0; i < 7; i++)
    {
        if (i >= firstWeekDay)
        {
            if (i == 6)
            {
                // red foreground-color in saturday
                printf("\033[1;31m");
            }
            if (currentDay == mitiDay)
            {
                // yellow foreground in today
                printf("\033[1;4;33m");
            }
            if (currentDay < 10)
            {
                printf(" ");
            }
            printf("%d", currentDay++);
            printf("\033[0m");
            printf("   ");
        }
        else
        {
            printf("     ");
        }
    }
    printf("\n");
    // write remaining weeks
    for (int w = 1; w <= 5; w++)
    {
        for (int i = 0; i < 7; i++)
        {
            if (currentDay > totalDays)
            {
                if (i > 0)
                {
                    printf("\n");
                }
                return;
            }
            if (i == 6)
            {
                // red foreground color in saturday
                printf("\033[1;31m");
            }
            if (currentDay == mitiDay)
            {
                // yellow foreground in today
                printf("\033[1;4;33m");
            }
            if (currentDay < 10)
            {
                printf(" ");
            }
            printf("%d", currentDay++);
            printf("\033[0m");
            printf("   ");
        }
        printf("\n");
    }
}

/**
 * @brief Parse given miti string and store in the given `struct miti *`.
 *
 * @param miti_str "2050-11-22" format
 * @param miti_tm
 */
void strpmiti(const char *miti_str, struct miti *miti_tm)
{
    sscanf(miti_str, "%d-%d-%d", &miti_tm->year, &miti_tm->mon, &miti_tm->day);
}

/**
 * @brief Formats the given `struct miti *` to '%Y-%m-%d-%w'
 *
 * @param buffer
 * @param n
 * @param miti
 */
void strfmiti(char *buffer, size_t n, struct miti *miti, int enable_weekday)
{
    int c = snprintf(buffer, n, "%d-%02d-%02d-%d", miti->year, miti->mon,
                     miti->day, miti->wday);
    if (enable_weekday == 0)
    {
        /**
         * @brief truncate the weekday from miti string.
         *
         */
        buffer[c - 2] = '\0';
    }
}

/**
 * @brief Returns 1 if given miti is valid.
 *
 * @param miti
 * @return int
 */
int test_miti_validity(struct miti *miti)
{
    return miti->year >= MITI_FIRST_YEAR &&
           miti->year <= MITI_LAST_YEAR + miti->mon > 0 && miti->mon < 13 &&
           miti->day > 0 &&
           miti->day <= miti_table[miti->year - MITI_FIRST_YEAR][miti->mon - 1];
}

int main()
{
    int reuse_address = 1;
    struct miti *miti = (struct miti *)malloc(sizeof(struct miti));
    // create socket:
    int serverSocket;
    int port = 8000;

    char display_name[100];
    // buffer for snprintf:
    int buffer_size = 1024;
    char buffer[buffer_size];
    // server response protocol: rfc 7230
    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body> I server %d Clients."
                "\nToday is %s</body></html>";

    // file descriptor for the new socket is returned
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverSocket != -1 || DIE("socket error");

    //reuse already bound address for this socket
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,  &reuse_address,sizeof(reuse_address)) == 0 || DIE("setsockopt error");

    // ip :
    struct sockaddr_in Internet_address = {
        // man 7 ip (linux):
        .sin_family = AF_INET, /* address family: AF_INET */
        // host to network order (short = 2 bit)
        .sin_port = htons(port), /* port in network byte order */
        // ifconfig inet address:
        .sin_addr.s_addr = htonl(INADDR_ANY) /* internet address */
    };

    // bind:
    bind(serverSocket, (struct sockaddr *)&Internet_address, sizeof(struct sockaddr_in)) != -1 || DIE("bind error");

    // listen:
    listen(serverSocket, 10) != -1 || DIE("listen error");

    printf("****listening on %d****\n", port);

    // accept:
    int client_socket;
    int client_count = 0;

    while (1)
    {
        client_socket = accept(serverSocket, NULL, NULL);
        client_socket != -1 || DIE("accept error");

        printf("client connected\n");

        // Printing raccoon to client:
        /*
        if(client_count <= 10){
            //display_name is a pointer.
            // strcpy(display_name = "Baby Raccoon"); -> understanding
            strcpy(display_name, "<h3>Baby Raccoon</h3>");
        }else {
            strcpy(display_name, "<h1>Rocket Raccoon</h1>");
        }
        */
        char mitiString[100];
        {
            char buffer[MITIBUFFERSIZE];
            time_t T = time(NULL);
            struct tm current_date = *localtime(&T);
            date_to_miti(&current_date, miti);
            strfmiti(buffer, MITIBUFFERSIZE, miti, 0);
            if (miti->year == 0)
            {
                snprintf(mitiString, 100, "Miti Out of Range. Cant be greater than 2100 BS.\n");
            }
            else
            {
                snprintf(mitiString, 100, "BS: %d %s %d\n", miti->year, months[miti->mon - 1], miti->day);
                printf("Miti: %s\n", mitiString);
            }
        }

        snprintf(buffer, buffer_size, msg, client_count, mitiString);

        // send:
        ssize_t byte_status = send(client_socket, buffer, strlen(buffer), MSG_CONFIRM);
        byte_status != -1 || DIE("send error");

        client_count = client_count + 1;

        close(client_socket) == 0 || DIE("close error");
    }
    close(serverSocket);
    /*
    struct sockaddr {
               sa_family_t sa_family;
               char        sa_data[14];
           }
    */

    return 0;
}