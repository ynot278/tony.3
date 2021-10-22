#define N_LICENSE 19 //Maximum number of processes
#define TIMER 100 //Number of seconds before signal is called

int getlicense(void); //Blocks until a license is available.

int returnlicense(void); //Increments the number of available licenses.

int initlicense(void); //Performs any needed initialization of the license object.

int addtolicenses(int n); //Adds n licenses to the number available.

int removelicenses(int n); //Decrements the number of licenses by n.

void doCommand(); //execute command

void setupSharedMemory(); //sets up shared memory and attach

void createChild(); //forks a child and runs doCommand

void sig_handler(int signum); //signal handler 