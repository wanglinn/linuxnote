/* user defined macro */

#define RUN_SUCCESS      1
#define RUN_FAIL         0

#define MSECOND          1000000

#define RECORD_TIME_START() \
    struct timeval timestart, timeend;  \
    gettimeofday(&timestart,NULL)

#define RECORD_TIME_END()          \
    gettimeofday(&timeend,NULL);   \
    float timeuse = MSECOND * (timeend.tv_sec - timestart.tv_sec) +  \
                    timeend.tv_usec - timestart.tv_usec;             \
    timeuse /= MSECOND;                                              \
    fprintf(stdout, "time use %f seconds\n", timeuse)

/* user defined type */

typedef signed char            int8;
typedef signed short int       int16;
typedef signed int             int32;
typedef signed long long int   int64;

typedef unsigned char          uint8;
typedef unsigned short int     uint16;
typedef unsigned int           uint32;
typedef unsigned long long int uint64;
