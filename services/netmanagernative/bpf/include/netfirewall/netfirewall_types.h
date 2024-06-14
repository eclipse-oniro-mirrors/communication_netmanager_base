/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NET_FIREWALL_TYPES_H
#define NET_FIREWALL_TYPES_H

#include <linux/types.h>
#include <linux/bpf.h>
#include <sys/socket.h>

#ifdef __cplusplus
#include <netinet/in.h>
#else
#include <linux/in6.h>
#endif

#define USER_ID_DIVIDOR 200000
#define DEFAULT_USER_ID 100
#define BITMAP_LEN 64

struct bitmap {
    __u32 val[BITMAP_LEN];
};

typedef __u32 *bitmap_ptr;
typedef __u32 bitmap_t[BITMAP_LEN];
#define BITMAP_BITS (BITMAP_LEN * 32)

enum stream_dir {
    INVALID = -1,
    INGRESS = 1,
    EGRESS,
};

enum event_type {
    EVENT_INVALID = -1,
    EVENT_INTERCEPT = 1,
    EVENT_DEBUG,
    EVENT_TUPLE_DEBUG,
};

enum debug_type {
    DBG_UNSPEC = 0,
    DBG_GENERIC, /* Generic, no message, useful to dump random integers */
    DBG_LOOKUP_FAIL,
    DBG_MATCH_SADDR,
    DBG_MATCH_DADDR,
    DBG_MATCH_SPORT,
    DBG_MATCH_DPORT,
    DBG_MATCH_PROTO,
    DBG_MATCH_APPUID,
    DBG_MATCH_UID,
    DBG_ACTION_KEY,
    DBG_MATCH_ACTION,
    DBG_TCP_FLAGS,
    DBG_CT_LOOKUP,
};

struct debug_event {
    enum debug_type type;
    enum stream_dir dir;
    __u32 arg1;
    __u32 arg2;
    __u32 arg3;
    __u32 arg4;
    __u32 arg5;
};

struct intercept_event {
    enum stream_dir dir;
    __u32 family;
    __u8 protocol;
    union {
        struct {
            __be32 saddr;
            __be32 daddr;
        } ipv4;
        struct {
            struct in6_addr saddr;
            struct in6_addr daddr;
        } ipv6;
    };
    __be16 sport;
    __be16 dport;
    __u32 appuid;
};

struct match_tuple {
    enum stream_dir dir;
    __u32 family;
    __u8 protocol;
    union {
        struct {
            __be32 saddr;
            __be32 daddr;
        } ipv4;
        struct {
            struct in6_addr saddr;
            struct in6_addr daddr;
        } ipv6;
    };
    __be16 sport;
    __be16 dport;
    __u32 appuid;
    __u32 uid;
    __u16 rst;
};


struct event {
    enum event_type type;
    union {
        struct debug_event debug;
        struct intercept_event intercept;
        struct match_tuple tuple;
    };
    __u32 len;
};

typedef __be32 ip4_key;
typedef struct in6_addr ip6_key;
typedef struct bitmap action_key;
typedef enum sk_action action_val;
typedef __u8 proto_key;
typedef __u32 appuid_key;
typedef __u32 uid_key;

typedef enum {
    CURRENT_USER_ID_KEY = 1,
} current_user_id_key;

typedef enum {
    DEFAULT_ACT_IN_KEY = 1,
    DEFAULT_ACT_OUT_KEY = 2,
} default_action_key;

struct ipv4_lpm_key {
        __u32 prefixlen;
        ip4_key data;
};

struct ipv6_lpm_key {
        __u32 prefixlen;
        ip6_key data;
};

struct port_segment {
    __u16 start;
    __u16 end;
};

#endif // NET_FIREWALL_TYPES_H
