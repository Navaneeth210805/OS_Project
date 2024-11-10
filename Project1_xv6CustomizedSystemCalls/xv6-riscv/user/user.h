struct stat;
#define SIGINT 2  // Define SIGINT signal number if not already defined
typedef unsigned int uint;
// System calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(const char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);
int cps(void);

// Signal handling
void signal(int signum, void (*handler)(int)); 
void handle_signals(void);  // Ensure this function is implemented somewhere
void sigint_default_handler(void);  // Default handler for SIGINT (if needed)

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...) __attribute__ ((format (printf, 2, 3)));
void printf(const char*, ...) __attribute__ ((format (printf, 1, 2)));
char* gets(char*, int max);
unsigned int strlen(const char*);  // Use unsigned int instead of uint
void* memset(void*, int, unsigned int);  // Use unsigned int instead of uint
int atoi(const char*);
int memcmp(const void *, const void *, unsigned int);  // Use unsigned int instead of uint
void *memcpy(void *, const void *, unsigned int);  // Use unsigned int instead of uint

// umalloc.c
void* malloc(unsigned int);  // Use unsigned int instead of uint
void free(void*);