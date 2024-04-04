/* SM.EXEC
   Logger functions
   anton.bondarenko@gmail.com */

#ifndef LOGGER_H
#define LOGGER_H



#define TL_REPORT_LN_ADJUSTMENT 1





//////// report

enum severity {
    ERROR,
    EVENT
};
typedef enum severity severity_t;

#define REPORT(severity, message) report(severity, (message), __FILE__, __LINE__, __func__)

int report(severity_t severity, const char *message, const char *file, int line, const char *function);

#endif //LOGGER_H
