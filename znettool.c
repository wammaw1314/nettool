#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>
#include <limits.h>
#include <ctype.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

enum
{
	IFSTATUS_ERR = -1,
	IFSTATUS_DOWN = 0,
	IFSTATUS_UP = 1,
};

static void set_ifreq_to_ifname(struct ifreq *ifreq, const char *if_name)
{
	memset(ifreq, 0, sizeof(struct ifreq));
	strncpy(ifreq->ifr_name, if_name, sizeof(ifreq->ifr_name) - 1);
}

static int get_netlink_status_ethtool(lua_State *L)
{
	const char *if_name = luaL_checkstring(L, 1);
	int skfd;
	struct ifreq ifreq;
	struct ethtool_value edata;

	set_ifreq_to_ifname(&ifreq, if_name);

	edata.cmd = ETHTOOL_GLINK;
	ifreq.ifr_data = (void*) &edata;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "socket err");
		return 2;
	}

	if (ioctl(skfd, SIOCETHTOOL, &ifreq) < 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "ETHTOOL_GLINK ERR");
		return 2;
	}

	lua_pushboolean(L, edata.data ? IFSTATUS_UP : IFSTATUS_DOWN);
	return 1;
}

static int get_netlink_status_mii(lua_State *L)
{
	/* char buffer instead of bona-fide struct avoids aliasing warning */
	const char *if_name = luaL_checkstring(L, 1);
	int skfd;
	char buf[sizeof(struct ifreq)];
	struct ifreq *const ifreq = (void *)buf;

	struct mii_ioctl_data *mii = (void *)&ifreq->ifr_data;

	set_ifreq_to_ifname(ifreq, if_name);

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) == 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "socket err");
		return 2;
	}

	if (ioctl(skfd, SIOCGMIIPHY, ifreq) < 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "SIOCGMIIPHY ERR");
		return 2;
	}

	mii->reg_num = 1;

	if (ioctl(skfd, SIOCGMIIREG, ifreq) < 0)
	{
		lua_pushnil(L);
		lua_pushstring(L, "SIOCGMIIREG ERR");
		return 2;
	}

	lua_pushboolean(L, (mii->val_out & 0x0004) ? IFSTATUS_UP : IFSTATUS_DOWN);
	return 1;
}

static const luaL_Reg R[] =
{
	{"get_link_stat_mii",get_netlink_status_mii},
	{"get_link_stat_etl",get_netlink_status_ethtool},
	{ NULL, NULL }
};

int luaopen_znettool(lua_State *L)
{
#if LUA_VERSION_NUM < 502
	luaL_register(L,"znettool", R);
#else
	lua_newtable(L);
	luaL_setfuncs(L, R, 0);
#endif
	return 1;
}
