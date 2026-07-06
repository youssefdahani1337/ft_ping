# ft_ping - ICMP Echo Request Implementation

A lightweight C implementation of the UNIX `ping` utility that sends ICMP Echo Request packets to test network connectivity and measure round-trip time to remote hosts.

## Table of Contents
- [How It Works](#how-it-works)
- [Code Architecture](#code-architecture)
- [Packet Flow](#packet-flow)
- [Key Components](#key-components)
- [Usage](#usage)
- [Building](#building)

---

## How It Works

### ICMP Protocol Overview
`ft_ping` implements the **Internet Control Message Protocol (ICMP)**, specifically:
- **ICMP Echo Request (Type 8)**: Sent to remote host
- **ICMP Echo Reply (Type 0)**: Response from remote host
- Optionally handles error responses (Unreachable, Time Exceeded)

### The Ping Process

```
1. Parse arguments and initialize state
2. Resolve hostname to IP address (DNS)
3. Create raw ICMP socket
4. Send loop:
   - Build ICMP packet (header + payload)
   - Calculate checksum
   - Send via raw socket
   - Measure time
5. Receive loop (parallel):
   - Receive reply packet
   - Parse IP header and ICMP payload
   - Calculate round-trip time
   - Display results
6. On completion: print statistics and exit
```

---

## Code Architecture

### File Structure

```
ft_ping/
├── includes/ft_ping.h          # Header with types and declarations
├── src/
│   ├── main.c                  # Entry point, main event loop
│   ├── parser.c                # Argument parsing and initialization
│   ├── resolve_host.c          # DNS resolution (getaddrinfo)
│   ├── connect_socket.c        # Raw socket creation and setup
│   ├── prepare_msg.c           # ICMP packet construction
│   ├── recv_msg.c              # Packet reception
│   └── utils.c                 # Display, statistics, checksum
├── Makefile                    # Build configuration
└── README.md
```

### Core Data Structures

#### `t_ping` - Command State
```c
typedef struct s_ping {
    char *host_str;              // Hostname/IP string
    struct sockaddr_in *target;  // Destination address
    size_t packet_transmitted;   // Counter: packets sent
    size_t packet_received;      // Counter: packets received
    int verbose;                 // -v flag for debug output
    int numeric;                 // -n flag (no DNS lookup)
    unsigned int timeout;        // -W socket timeout (seconds)
    unsigned int deadline;       // -w stop after N seconds
    int ttl;                     // --ttl hop limit
    int sockfd;                  // Raw socket file descriptor
    t_recv_packet *recv_packet;  // Received packet buffer
    char *send_buf;              // ICMP packet buffer
    size_t payload_size;         // -s payload size
    uint16_t pid;                // Process ID (ICMP ID)
    uint16_t seq;                // Sequence counter
    struct timespec start_time;  // Deadline tracking
} t_ping;
```

#### `t_recv_packet` - Receive Buffer
```c
typedef struct s_recv_packet {
    int packet_size;                     // Received bytes
    char recv_buf[1024];                 // Raw packet data
    struct sockaddr_in recv_addr;        // Source address
} t_recv_packet;
```

---

## Packet Flow

### 1. Initialization (`main.c` → `parser.c`)
```
main()
  ├─ Parse command-line arguments
  │   └─ handle flags: -v, -n, -s, -W, -w, --ttl
  ├─ Initialize ping state (init_args)
  └─ Validate hostname provided
```

### 2. DNS Resolution (`resolve_host.c`)
```
resolve_host()
  ├─ Call getaddrinfo(hostname, NULL, &hints, &result)
  ├─ Extract IPv4 address from addrinfo result
  ├─ Store in ping->target (sockaddr_in struct)
  └─ Free addrinfo result
```

Error handling: If hostname cannot resolve → exit with code 68

### 3. Socket Creation (`connect_socket.c`)
```
connect_socket()
  ├─ socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)
  │   └─ Raw socket for ICMP (requires root/CAP_NET_RAW)
  ├─ setsockopt(SO_RCVTIMEO)
  │   └─ Set receive timeout (avoid blocking forever)
  ├─ setsockopt(IP_TTL) if --ttl specified
  │   └─ Set IP Time-To-Live field
  └─ Store sockfd for later use
```

Error handling: If socket creation fails → exit with code 69

### 4. Packet Transmission (`prepare_msg.c` → `send_ping()`)

#### ICMP Echo Request Packet Format:
```
[IP Header (20 bytes) - added by kernel]
[ICMP Header (8 bytes)]
  ├─ Type: 8 (Echo Request)
  ├─ Code: 0
  ├─ Checksum: calculated
  ├─ ID: process PID
  └─ Sequence: incremented each packet
[Payload (default 56 bytes)]
  └─ Filled with 'Y' character
```

#### Construction Process:
```c
load_new_msg(cmd)
  ├─ Allocate send_buf = ICMP_HEADER_SIZE + payload_size
  ├─ Zero-fill entire buffer
  ├─ Cast to struct icmp
  ├─ Set type=ICMP_ECHO, code=0
  ├─ Set ID (convert to network byte order: htons)
  ├─ Set Sequence (convert to network byte order: htons)
  ├─ Fill payload with data
  └─ Calculate checksum (16-bit one's complement sum)

calculate_checksum(buffer, length)
  ├─ Treat buffer as array of 16-bit words
  ├─ Sum all words
  ├─ Handle odd byte if needed
  ├─ Fold overflow bits
  └─ Return bitwise complement (~sum)
```

#### Sending:
```c
send_ping(cmd)
  ├─ Call sendto() with raw socket
  ├─ Verify all bytes sent (sendto return == bytes_sent)
  ├─ Increment packet counter
  └─ Increment sequence number for next packet
```

### 5. Packet Reception (`recv_msg.c`)

```c
recv_msg(cmd)
  ├─ Call recvfrom() on socket
  ├─ Store source IP in recv_addr
  ├─ Handle timeouts gracefully (EAGAIN/EWOULDBLOCK)
  └─ Store packet size (<=0 indicates timeout)
```

**Key Point**: Raw socket returns complete IP packet including header

### 6. Packet Parsing & Display (`utils.c`)

#### `display_ping_result()` - Validation
```
Validate received packet:
  ├─ Check packet size ≥ sizeof(IP header) + sizeof(ICMP header)
  ├─ Parse IP header: icmp_offset = ip_header_length * 4
  ├─ Validate IP header length is reasonable
  └─ Extract ICMP packet from correct offset
```

#### `handle_icmp_packet()` - Route by Type
```c
switch (icmp->icmp_type) {
    case ICMP_ECHOREPLY:       // Type 0
        ├─ Verify it's our packet (ID matches)
        ├─ Calculate round-trip time
        ├─ Display: "X bytes from Y: icmp_seq=Z ttl=T time=Dms"
        └─ Increment packet_received counter
    
    case ICMP_UNREACH:         // Type 3 (Destination unreachable)
        ├─ Decode error code (network unreachable, host unreachable, etc)
        ├─ Default output: "From X: Network Unreachable"
        └─ Verbose output: "From X: Network Unreachable (code=0)"
    
    case ICMP_TIMXCEED:        // Type 11 (Time exceeded)
        ├─ Decode error code (transit, reassembly)
        ├─ Default output: "From X: TTL exceeded in transit"
        └─ Verbose output: "From X: TTL exceeded in transit (code=0)"
    
    default:
        └─ Display: "From X: ICMP type=Y [code=Z if verbose]"
}
```

#### Time Calculation
```c
elapsed_ms = (end_time - start_time) in milliseconds
           = (tv_sec difference * 1000) + (tv_nsec difference / 1e6)
```

### 7. Statistics & Cleanup (`utils.c` → `main.c`)

```c
print_ping_statistics(cmd)
  ├─ Print: "X packets transmitted, Y packets received"
  ├─ Calculate: packet_loss% = (X-Y)/X * 100
  └─ Example: "2 packets transmitted, 2 packets received, 0% packet loss"

cleanup(cmd)
  ├─ close(sockfd)
  ├─ free(target address)
  ├─ free(send buffer)
  ├─ free(receive packet)
  └─ free(command state)
```

---

## Key Components

### Signal Handling
```c
handle_sigint(SIGINT) → g_stop = 1
```
- Gracefully stops the ping loop when user presses `Ctrl+C`
- Ensures statistics are printed before exit

### Deadline Management
```c
deadline_expired(cmd)
  ├─ If deadline=0: return false (infinite)
  ├─ Else: check if (current_time - start_time) >= deadline
  └─ Allows "-w N" to stop after N seconds
```

### DNS Resolution
```c
getaddrinfo(hostname, NULL, &hints, &result)
  ├─ Hints: ai_family=AF_INET (IPv4 only)
  ├─ Result: array of addrinfo structs
  └─ Extract: result->ai_addr (sockaddr_in with IP)
```

### Network Byte Order
- **htons()**: Host-to-Network byte order (little-endian → big-endian)
- Used for ICMP ID and Sequence fields
- **ntohs()**: Reverse operation

### Verbose Mode
- `-v` flag enables debug output
- Error messages show error codes
- Debug info on ICMP send/receive

---

## Usage

### Basic Usage
```bash
sudo ./ft_ping google.com
```

### With Options
```bash
sudo ./ft_ping -v -n -s 100 -w 5 google.com
```

**Options:**
- `-v`: Verbose (show error codes, debug info)
- `-n`: Numeric output only (skip DNS reverse lookup)
- `-s SIZE`: Payload size (default 56, max 65507)
- `-W TIMEOUT`: Receive timeout in seconds (default 1)
- `-w DEADLINE`: Stop after N seconds
- `--ttl VALUE`: Set IP Time-To-Live (default varies by OS)

---

## Building

### Prerequisites
- GCC or Clang
- Root access or `CAP_NET_RAW` capability
- Standard C library

### Compile
```bash
make
```

### Set Capabilities (Alternative to sudo)
```bash
sudo setcap cap_net_raw+ep ./ft_ping
./ft_ping google.com  # No sudo needed
```

### Clean
```bash
make clean          # Remove object files
make fclean         # Remove objects + binary
```

---

## Error Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Memory allocation failure |
| 2 | Invalid arguments |
| 68 | Cannot resolve hostname |
| 69 | Cannot create socket (need root/CAP_NET_RAW) |

---

## Technical Details

### Checksum Calculation (RFC 792)
1. Treat ICMP packet as array of 16-bit words
2. Sum all words
3. If odd byte remains, add it as 8-bit value (shifted)
4. Fold 32-bit result into 16-bit by adding carries
5. Return one's complement of result

### Raw Socket Behavior
- Returns complete IP packet from recvfrom()
- IP header included in received data
- Must parse IP header to find ICMP payload
- Requires root/CAP_NET_RAW capability

### Buffer Management
- MAX_RECV_BUFFER = 1024 bytes (sufficient for ICMP + IP headers)
- Validates all buffer access to prevent overflow
- Bounds checking on IP header offset calculations

---

## Code Quality

✓ Proper error handling throughout  
✓ Memory cleanup on exit  
✓ Buffer overflow protection  
✓ Signal handling for clean shutdown  
✓ Verbose mode for debugging  
✓ Consistent code style and formatting  
✓ Comments on complex logic