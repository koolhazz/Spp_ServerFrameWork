#ifndef _BENCHAPI_H
#define _BENCHAPI_H
#include <sys/cdefs.h>
#include <sys/types.h>
#include <arpa/inet.h>

#if 0
__BEGIN_DECLS
#undef __attr_cdecl__
/*
#define __attr_cdecl__ __attribute__((__cdecl__))
*/
#define __attr_cdecl__ /* */

#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __builtin_expect(x, expected_value) (x)
#endif

#undef __attr_pure__
#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __attr_pure__   /* */
#else
#define __attr_pure__   __attribute__((__pure__))
#endif

#undef __attr_nonnull__
#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __attr_nonnull__(x)     /* */
#else
#define __attr_nonnull__(x) __attribute__((__nonnull__(x)))
#endif

#ifndef likely
#define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

#define __init          __attribute__ ((__section__ (".init.text")))
#define __initdata      __attribute__ ((__section__ (".init.data")))
#define __exitdata      __attribute__ ((__section__(".exit.data")))
#define __exit_call     __attribute_used__ __attribute__ ((__section__ (".exitcall.exit")))

#ifdef MODULE
#define __exit          __attribute__ ((__section__(".exit.text")))
#else
#define __exit          __attribute_used__ __attribute__ ((__section__(".exit.text")))
#endif

#define APP_EMERG 		0  /* system is unusable               */
#define APP_ALERT		1  /* action must be taken immediately */
#define APP_CRIT		2  /* critical conditions              */
#define APP_ERROR		3  /* error conditions                 */
#define APP_WARNING		4  /* warning conditions               */
#define APP_NOTICE		5  /* normal but significant condition */
#define APP_INFO		6  /* informational                    */
#define APP_DEBUG		7  /* debug-level messages             */
#define APP_TRACE		8  /* trace-level messages             */

#define DETAIL(level, fmt, args...) \
	write_log (level, "[%s][%d]%s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__ , ##args) 
	
#define SIMPLY(level, fmt, args...) write_log(level, fmt "\n" , ##args)

#define ERROR_LOG(fmt, args...)	DETAIL(APP_ERROR, fmt , ##args)
#define CRIT_LOG(fmt, args...)	DETAIL(APP_CRIT, fmt , ##args)
#define ALERT_LOG(fmt, args...)	DETAIL(APP_ALERT, fmt , ##args)
#define EMERG_LOG(fmt, args...)	DETAIL(APP_EMERG, fmt , ##args)

#define WARN_LOG(fmt, args...)	SIMPLY(APP_WARNING, fmt , ##args)
#define NOTI_LOG(fmt, args...)	SIMPLY(APP_NOTICE, fmt , ##args)
#define INFO_LOG(fmt, args...)	SIMPLY(APP_INFO, fmt , ##args)
#define DEBUG_LOG(fmt, args...)	SIMPLY(APP_DEBUG, fmt , ##args)
#define BOOT_LOG(OK, fmt , args...) do{ \
	boot_log(OK, 0, fmt , ##args); \
	return OK; \
}while (0)

#define BOOT_LOG2(OK, n, fmt , args...) do{ \
	boot_log(OK, n, fmt , ##args); \
	return OK; \
}while (0)

#define BOOT_LOG_NORETURN(OK, fmt , args...) do{ \
	boot_log(OK, 0, fmt , ##args); \
}while (0)



#define ERROR_RETURN(X, Y) do{ \
	ERROR_LOG X; \
	return Y; \
}while (0)

#ifdef DEBUG
#define TRACE_LOG(fmt , args...)	SIMPLY(APP_TRACE, fmt, ##args)
#else
#define TRACE_LOG(fmt , args...)	
#endif

/*
 * config api
 */
__attr_cdecl__ __attr_nonnull__(1)
extern int config_init (const char* file); 

__attr_cdecl__ __attr_nonnull__(1)
extern char* config_get_strval (const char* key);

__attr_cdecl__ __attr_nonnull__(1)
extern int config_get_intval (const char* key, int def);

__attr_cdecl__ __attr_nonnull__(1)
extern int mmap_config_file (const char* file, char **buf);
/*
 * log api
 */
__attr_cdecl__ __attr_nonnull__(1)
extern int log_init (const char* dir, int level, u_int size, const char* fix_name);

__attr_cdecl__ __attr_nonnull__(2)
extern void write_log (int lvl, const char* fmt, ...);

__attr_cdecl__ __attr_nonnull__(3)
extern void boot_log (int OK, int dummy, const char* fmt, ...);

/*
 * socket api && return code
 */

/* safe tcp recv return code define: */
#define TCP_RECV_DISCONN_BY_PEER    0
#define TCP_RECV_FAIL               -1
#define TCP_RECVING                 -2

__attr_cdecl__ __attr_nonnull__(1)
extern int safe_socket_listen (struct sockaddr_in *servaddr, int type);

__attr_cdecl__ __attr_nonnull__(2)
extern int safe_tcp_accept (int sockfd, struct sockaddr_in *peer);

__attr_cdecl__ __attr_nonnull__(2)
extern int safe_tcp_recv_n (int sockfd, void *buf, int total);

__attr_cdecl__ __attr_nonnull__(2)
extern int safe_tcp_send_n (int sockfd, void *buf, int total);

__attr_cdecl__ __attr_nonnull__(2)
extern int safe_tcp_recv (int sockfd, void *buf, int bufsize);

__attr_cdecl__ __attr_nonnull__(1)
extern int safe_tcp_connect (const char *ipaddr, u_short port);

__END_DECLS
#endif

#endif

