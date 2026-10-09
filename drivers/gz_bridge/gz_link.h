#ifndef GZ_LINK_H_

#include <stdbool.h>
#include "gz_message.pb.h"

typedef void (*gz_link_msg_cb)(GazeboMessage *msg);

bool gz_link_start(gz_link_msg_cb cb);
int gz_link_write(const GazeboMessage *msg);

#endif /* GZ_LINK_ */
