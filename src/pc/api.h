#ifndef __API_H__
#define __API_H__

#include "heartbeat.h"
#include "response.h"

int getSentHeartbeats(lHeartbeat**, int);
int getReceivedMessages(int, int, lHeartbeat**, lResponse**);

#endif
