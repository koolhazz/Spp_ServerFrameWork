
#ifndef _BENCHAPIPLUS_H_
#define _BENCHAPIPLUS_H_

typedef int		(*spp_handle_init_t)		 	 (void*, void*);//插件初始化
typedef int		(*spp_handle_input_t)		        (unsigned int, void*, void*);//检查包的完整性
typedef int		(*spp_handle_route_t)		 (unsigned int, void*, void*);//数据包路由
typedef void		(*spp_handle_fini_t)		  	 (void*, void*);//析构资源

typedef int		(*spp_handle_local_process_t)(unsigned, void*, void*);//业务处理

typedef int           (*spp_handle_event_route_t)(unsigned int,unsigned int,void *,void *);//网络事件路由类型
typedef int           (*spp_handle_queue_full_t) (unsigned int ,unsigned int, void * , void * );//内存管道满的处理
typedef int           (*spp_handle_preprocess_t)(unsigned int,unsigned int, void*, void*);//业务预处理
typedef int		(*spp_handle_process_t)	  	(unsigned int,unsigned int, void*, void*);//业务处理
typedef int 		(*spp_handle_reloadconfig_t)(void *,void *,void *);//重新载入业务组件的配置文件


typedef struct 
{ 
	void *handle;
	spp_handle_init_t		spp_handle_init;
	spp_handle_input_t		spp_handle_input;
	spp_handle_route_t		spp_handle_route;
	spp_handle_queue_full_t  spp_handle_queue_full;
	spp_handle_process_t	spp_handle_process;
	spp_handle_reloadconfig_t spp_handle_reloadconfig;
	spp_handle_event_route_t spp_handle_event_route;
	spp_handle_fini_t		spp_handle_fini;
}spp_dll_func_t;
extern spp_dll_func_t sppdll;

#endif

