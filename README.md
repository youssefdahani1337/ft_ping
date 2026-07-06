# ft_ping

A reimplementation of the Unix `ping` utility in C.

`ft_ping` is an educational networking project whose goal is to understand how ICMP works, how packets travel through an IP network, and how diagnostic tools communicate with remote hosts using raw sockets.

---

# Table of Contents

* [Introduction](#introduction)
* [What is Ping?](#what-is-ping)
* [How Ping Works](#how-ping-works)
* [The IP Protocol](#the-ip-protocol)
* [The ICMP Protocol](#the-icmp-protocol)
* [ICMP Echo Request / Echo Reply](#icmp-echo-request--echo-reply)
* [Packet Journey](#packet-journey)
* [Interpreting the Results](#interpreting-the-results)
* [Features](#features)
* [Building](#building)
* [Usage](#usage)
* [Examples](#examples)
* [Requirements](#requirements)
* [References](#references)

---

# Introduction

`ping` is one of the oldest and most widely used network diagnostic tools.

It allows a user to verify whether another machine is reachable over an IP network while also measuring the round-trip time (RTT) between the sender and the destination.

Although its output appears simple, `ping` relies on several important networking concepts including:

* IP routing
* ICMP messages
* Raw sockets
* Packet encapsulation
* Checksums
* Time-To-Live (TTL)
* DNS and reverse DNS

Implementing `ping` from scratch provides an excellent introduction to low-level network programming.

---

# What is Ping?

`ping` is a network utility that sends **ICMP Echo Request** packets to a remote host.

If the destination is reachable and configured to respond, it returns an **ICMP Echo Reply**.

For every successful reply, `ping` displays information such as:

* Number of bytes received
* Source address
* ICMP sequence number
* TTL (Time To Live)
* Round-trip time (RTT)

Example:

```text
64 bytes from google.com (142.250.184.206):
icmp_seq=1 ttl=117 time=31.2 ms
```

If no reply is received within the timeout period, a timeout message is displayed.

---

# How Ping Works

A typical exchange consists of four steps:

1. Resolve the destination hostname into an IPv4 address.
2. Build an ICMP Echo Request packet.
3. Send the packet through a raw socket.
4. Wait for an ICMP Echo Reply and measure the elapsed time.

```
Client                             Server
   |                                  |
   |------ ICMP Echo Request --------->|
   |                                  |
   |<------ ICMP Echo Reply -----------|
   |                                  |
```

The measured time between sending the request and receiving the reply is called the **Round-Trip Time (RTT)**.

---

# The IP Protocol

The **Internet Protocol (IP)** is responsible for delivering packets across interconnected networks.

Every ICMP packet is encapsulated inside an IPv4 packet before being transmitted.

```
+-------------------------+
| IPv4 Header             |
+-------------------------+
| ICMP Header             |
+-------------------------+
| Payload                 |
+-------------------------+
```

The IPv4 header contains important routing information such as:

| Field               | Description                     |
| ------------------- | ------------------------------- |
| Version             | IP protocol version (IPv4)      |
| IHL                 | Header length                   |
| Total Length        | Total packet size               |
| Identification      | Used for fragmentation          |
| Flags               | Fragmentation control           |
| Fragment Offset     | Fragment position               |
| TTL                 | Maximum number of hops          |
| Protocol            | Upper-layer protocol (ICMP = 1) |
| Header Checksum     | Detects header corruption       |
| Source Address      | Sender IP                       |
| Destination Address | Receiver IP                     |

The IP protocol is responsible only for transporting packets between hosts. It does not guarantee delivery.

---

# The ICMP Protocol

**ICMP (Internet Control Message Protocol)** is a companion protocol to IP.

Unlike TCP or UDP, ICMP is not used for transporting application data.

Instead, it is used for:

* Network diagnostics
* Error reporting
* Connectivity testing
* Route discovery

Examples of ICMP messages include:

* Echo Request
* Echo Reply
* Destination Unreachable
* Time Exceeded
* Redirect
* Parameter Problem

Because ICMP operates directly on top of IP, its protocol number inside the IPv4 header is:

```
Protocol = 1
```

---

# ICMP Echo Request / Echo Reply

`ping` primarily uses two ICMP message types:

| Type | Name         |
| ---- | ------------ |
| 8    | Echo Request |
| 0    | Echo Reply   |

An Echo Request contains:

* Type
* Code
* Checksum
* Identifier
* Sequence Number
* Payload

Example:

```
Echo Request

+----------------+
| Type = 8       |
| Code = 0       |
| Checksum       |
| Identifier     |
| Sequence       |
| Payload        |
+----------------+
```

The destination host copies almost the entire packet and sends it back as an Echo Reply.

---

# Packet Journey

The complete packet flow looks like this:

```
Application
      │
      ▼
ICMP Echo Request
      │
      ▼
IPv4 Packet
      │
      ▼
Ethernet Frame
      │
      ▼
Network
      │
      ▼
Remote Host
      │
      ▼
ICMP Echo Reply
```

Every router along the path decrements the packet's **TTL**.

If the TTL reaches zero before arriving at the destination, the router discards the packet and returns an **ICMP Time Exceeded** message.

---

# Interpreting the Results

Example output:

```text
64 bytes from google.com (142.250.184.206):
icmp_seq=4 ttl=117 time=32.4 ms
```

Meaning:

| Field           | Description                                |
| --------------- | ------------------------------------------ |
| 64 bytes        | Size of the received ICMP packet           |
| google.com      | Hostname                                   |
| 142.250.184.206 | Source IP address                          |
| icmp_seq=4      | Echo Request sequence number               |
| ttl=117         | Remaining Time-To-Live of the reply packet |
| time=32.4 ms    | Round-trip time                            |

---

# Features

This implementation supports:

* ICMP Echo Request / Echo Reply
* IPv4
* Hostname resolution
* Reverse DNS lookup
* Packet loss statistics
* RTT measurement
* ICMP error handling
* Configurable payload size
* Configurable timeout
* Configurable deadline
* Configurable TTL
* Verbose mode
* Numeric output mode

---

# Building

Clone the repository and build using `make`.

```bash
make
```

Clean object files:

```bash
make clean
```

Remove all generated files:

```bash
make fclean
```

Rebuild:

```bash
make re
```

---

# Usage

Running `ft_ping` generally requires administrative privileges because it creates raw sockets.

```bash
sudo ./ft_ping [OPTIONS] HOST
```

## Available options

| Option        | Description                                      |
| ------------- | ------------------------------------------------ |
| `-v`          | Verbose output                                   |
| `-n`          | Numeric output only (disable reverse DNS lookup) |
| `-s SIZE`     | Payload size in bytes                            |
| `-W TIMEOUT`  | Timeout waiting for each reply (seconds)         |
| `-w DEADLINE` | Stop after the specified number of seconds       |
| `--ttl VALUE` | Set the IP Time-To-Live value                    |

---

# Examples

Basic ping:

```bash
sudo ./ft_ping google.com
```

Numeric mode:

```bash
sudo ./ft_ping -n google.com
```

Verbose mode:

```bash
sudo ./ft_ping -v google.com
```

Custom payload:

```bash
sudo ./ft_ping -s 128 google.com
```

Custom timeout:

```bash
sudo ./ft_ping -W 2 google.com
```

Stop after 10 seconds:

```bash
sudo ./ft_ping -w 10 google.com
```

Send packets with a TTL of 32:

```bash
sudo ./ft_ping --ttl=32 google.com
```

---

# Requirements

* Unix-like operating system
* C compiler
* Raw socket support
* Root privileges (or equivalent capabilities on supported systems)

---

# References

* RFC 791 — Internet Protocol (IPv4)
* RFC 792 — Internet Control Message Protocol (ICMP)
* RFC 1071 — Computing the Internet Checksum
* RFC 1122 — Requirements for Internet Hosts
* Linux `ping(8)` manual
* GNU Inetutils documentation
