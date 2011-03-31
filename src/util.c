/*
 * Copyright (c) 2006-2009 Bjorn Andersson <flex@kryo.se>, Erik Ekman <yarrick@kryo.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include "common.h"
#if defined(TARGET_OS_IPHONE)
#include <string.h>
#include <resolv.h>

void ip_from_sockaddr(struct sockaddr *sockaddr, char *buffer, int count)
{
    struct sockaddr_in *ipv4;
    struct sockaddr_in6 *ipv6;
    
    memset((void *)buffer, 0, count);
    switch (sockaddr->sa_family) {
        case AF_INET:
            ipv4 = (struct sockaddr_in *)sockaddr;
            inet_ntop(sockaddr->sa_family, (void *)&(ipv4->sin_addr), buffer, sizeof(buffer) - 1);
            break;
        case AF_INET6:
            ipv6 = (struct sockaddr_in6 *)sockaddr;
            inet_ntop(sockaddr->sa_family, (void *)&(ipv6->sin6_addr), buffer, sizeof(buffer) - 1);
            break;
        case AF_UNSPEC:
            break;
        default:
            break;
    }
}

#endif // TARGET_OS_IPHONE

char *
get_resolvconf_addr()
{
	static char addr[16];
	char *rv;
#ifdef WINDOWS32
	FIXED_INFO  *fixed_info;
	ULONG       buflen;
	DWORD       ret;
    
	rv = NULL;
	fixed_info = malloc(sizeof(FIXED_INFO));
	buflen = sizeof(FIXED_INFO);
    
	if (GetNetworkParams(fixed_info, &buflen) == ERROR_BUFFER_OVERFLOW) {
		/* official ugly api workaround */
		free(fixed_info);
		fixed_info = malloc(buflen);
	}
    
	ret = GetNetworkParams(fixed_info, &buflen);
	if (ret == NO_ERROR) {
		strncpy(addr, fixed_info->DnsServerList.IpAddress.String, sizeof(addr));
		addr[15] = 0;
		rv = addr;
	}
	free(fixed_info);
#elif defined(TARGET_OS_IPHONE)
    if ((_res.options & RES_INIT) == 0) res_init();
    
    int i;
    for (i = 0; i < _res.nscount; i++) {
        inet_ntop(_res.nsaddr_list[i].sin_family, (void *)&(_res.nsaddr_list[i].sin_addr), addr, sizeof(addr) - 1);
        if (addr[0] != 0) {
            rv = addr;
            break;
        }
    }
#else
	char buf[80];
	FILE *fp;
	
	rv = NULL;
    
	if ((fp = fopen("/var/run/resolv.conf", "r")) == NULL) 
		err(1, "/etc/resolv.conf");
	
	while (feof(fp) == 0) {
		fgets(buf, sizeof(buf), fp);
        
		if (sscanf(buf, "nameserver %15s", addr) == 1) {
			rv = addr;
			break;
		}
	}
	
	fclose(fp);
#endif
	return rv;
}

