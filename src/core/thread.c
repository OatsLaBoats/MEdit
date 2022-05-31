#include "thread.h"
#include "utils.h"

int create_thread(ThreadProc proc, void *args)
{
    return_if_win(CreateThread(0, 0, proc, args, 0, NULL) == NULL, "Failed to create thread", -1);
    return 0;
}
