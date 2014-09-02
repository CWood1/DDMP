#ifndef __API_H__
#define __API_H__

#include "heartbeat.h"
#include "response.h"

#include <dhcpext/stream.h>

void getSentHeartbeats(lHeartbeat**, tStream*);
int getReceivedMessages(tStream*, tStream*, lHeartbeat**, lResponse**);

#endif
