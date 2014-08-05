Mesh Extensions to DHCP
Version 0.3.2
Last updated: 18/07/14
DRAFT

Changelog
16/07/14 - Initial draft written - Connor Wood (connorwood71@gmail.com)
17/07/14 - Some inconsistencies and ambiguities cleaned up, fields reorganized in heartbeat - Connor Wood (connorwood71@gmail.com)
18/07/14 - Added new field, to indicate if message is heartbeat pulse or heartbeat response, and fixed technical error: an octet is a byte, you fucking nimrod - Connor Wood (connorwood71@gmail.com)
05/08/14 - IP address fields unneeded in messages, provided by system already - Connor Wood (connorwood71@gmail.com)
05/08/14 - Cleaned up more IP address field remnants - Connor Wood (connorwood71@gmail.com)

Authors
Connor Wood (connorwood71@gmail.com)

Body
In the event of a new node coming online in any given mesh network, it is to automatically assume the role of the active DHCP server. In this instance, it is to begin transmitting heartbeat packets (defined below) on UDP port 4123 (subject to change).

When a DHCP request is made on the network, a client may set the option 230, Request Mesh Extensions, to length 0. In the event the active server sees this, and has mesh extensions active, it may reply with option 230 set, with body of length 8 octets (2 32 bit fields):

    0-4        5-8
+----------+--------+
| POSITION | LISTEN |
+----------+--------+

The Position field gives the position of the node in the global list, used to determine DHCP handoff in the event of failure or shutdown (see below). The Listen field is the IP address of the node to be listened to; this node will send a heartbeat, directed at the node being configured, which the current node must respond to. In the event that the node being listened for goes down, the active DHCP server must be informed, and the current node must begin listening to the node before it.

It is assumed that, upon successful configuration, the current node is to begin directing its heartbeat at the active DHCP server. In this sense, a circular buffer is maintained, in priority order, of which nodes are to assume control of the DHCP responsibility, in the event of node failure.

The heartbeat format is as below:
    0      1-4     5-8
+-------+-------+-------+---------+
| IDENT | FLAGS | MAGIC | OPTIONS |
+-------+-------+-------+---------+

Ident should be 0, to indicate a heartbeat pulse. The flags field is as follows:
             1111111111222222222233
 0 1 234567890123456789012345678901
+-+-+------------------------------+
|A|S|  RESERVED                    |
+-+-+------------------------------+

The A flag indicates that this server is the current, active DHCP server; this flag indicates the presence of the options field. The S flag indicates that this is the last heartbeat this server is sending before shutdown. The options field is as follows:
    0-4        5-n      n-n+4      n+4-m
+----------+--------+----------+----------+
| LEASELEN | LEASES | NUMNODES | NODELIST |
+----------+--------+----------+----------+

The Leaselen and Leases fields represent the length of the lease file in octets, 4 octets itself, and a full copy of the leases file, respectively. This particular packet is to be broadcast on the whole network.

Numnodes indicates the number of nodes on the network, and Nodelist is the IP of each node, sorted into order of failover. In this way, every node has a full copy of the node list, sorted to priority order, to be used in the event of node failure.

The Magic number is to be set to a random number at send time, to be echoed back. The response to this packet is as follows:
    0      1-4     5-8
+-------+-------+-------+--------+
| IDENT | FLAGS | MAGIC | ACTIVE |
+-------+-------+-------+--------+

Ident should be 1, to indicate a response; the Magic field is a copy of the magic number sent in the initial packet. The Flags field is defined as below:

             1111111111222222222233
 0 1 234567890123456789012345678901
+-+-+------------------------------+
|F|D|  RESERVED                    |
+-+-+------------------------------+

In the event the F flag is set, the node being listened to by the sending node has failed; in which case, the nodes either side of the failing node are now pointed at each other, and the active server should remove the failed node from its list of nodes. This should only be set in response to the active DHCP server. If the D flag is set, it means that the responding node is an active DHCP server; This should only be set if the received heartbeat had the A flag set, and indicates the server will send the Active fields as well (see below).

Each heartbeat is to be sent every 100ms, and in the event that 4 consecutive heartbeats have been missed, or lack a response, a node is assumed to have failed; in this case, the list of active nodes should be consulted by the relevant nodes, and the heartbeat signals pointed at the appropriate nodes; in the event that the failed node was the active DHCP server, the DHCP role is to move to the next active node on the list, which is to then not only continue transmitting a heartbeat signal to the next node in the list, but is to also begin broadcasting the heartbeat, as defined above.

In the event that a new node connects to a network, by default as per protocol DHCP will be active, and it will assume itself to be the active DHCP node. In the event that 2 networks merge, or a new node appears on the network, 2 active DHCP nodes will be on the same network. In this case, the first node to receive a broadcast heartbeat from the other, is to send the following response (Active fields):

     0-3         4
+----------+----------+
| NUMNODES | SHUTDOWN |
+----------+----------+

The Numnodes field is the number of nodes on its network, and the shutdown field can assume one of 2 values: 1 indicates that the node sending this response is shutting down, and 2 indicates the node receiving the response should shut down; this value is calculated by comparing the number of nodes active on each DHCP server - the server with the greater number of nodes should stay active. In the event that both servers have the same number, the server sending this response will generate a random value of either 1 or 2 for this field, the equivalent of a coin toss, to determine the shutdown.

The server to shutdown will send a final heartbeat - to inform the server remaining active of all parameters of the network, as well as to synchronise all nodes on the network - with its S flag set.

In the event of a DHCP server graceful shutdown, the next node in the priority list will wait a total of 400ms before assuming the role of active DHCP server; this is to give any other active server on the network chance to establish its heartbeat.

In the event that a node receives the shutdown flag from its active server, it is to wait until receiving a heartbeat from any active DHCP server on the net (assuming it does not, in this instance, assume the role itself). Once this occurs, it is to invalidate its own IP address, and request a new configuration from the new DHCP server.

In the event that an active DHCP server is to shut down, but remain on the network, it should, as above, wait until heartbeat is established from another active server, before invalidating its IP address, and requesting a new one as per normal DHCP rules.
