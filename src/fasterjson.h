#ifndef _H_FASTERJSON_
#define _H_FASTERJSON_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* util */

#if ( defined _WIN32 )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC		_declspec(dllexport)
#endif
#elif ( defined __unix ) || ( defined __linux__ )
#ifndef _WINDLL_FUNC
#define _WINDLL_FUNC
#endif
#endif

#define JSONESCAPE_EXPAND(_buf_,_buf_len_,_buf_remain_len_)		\
	if( (_buf_len_) > 0 )						\
	{								\
		int	_newlen_ ;					\
		for( _newlen_ = 0 ; _newlen_ < (_buf_len_) ; )		\
		{							\
			if( *(_buf_) == '\"' )				\
			{						\
				if( (_buf_remain_len_) < 2-1 )		\
				{					\
					(_buf_len_) = -1 ;		\
					break;				\
				}					\
				memmove( (_buf_)+2 , (_buf_)+1 , (_buf_len_)-_newlen_-1 );	\
				memcpy( (_buf_) , "\\\"" , 2 );		\
				(_buf_) += 2 ;				\
				(_buf_len_) += 2-1 ;			\
				_newlen_ += 2 ;				\
				(_buf_remain_len_) -= 2-1 ;		\
			}						\
			else						\
			{						\
				_newlen_++;				\
				(_buf_)++;				\
				(_buf_remain_len_)--;			\
			}						\
		}							\
	}								\

#define JSONUNESCAPE_FOLD(_src_,_src_len_,_dst_)			\
	if( (_src_len_) > 0 )						\
	{								\
		char	*_p_src_ = (_src_) ;				\
		char	*_p_src_end_ = (_src_) + (_src_len_) - 1 ;	\
		char	*_p_dst_ = (_dst_) ;				\
		for( (_src_len_) = 0 ; (_p_src_) <= _p_src_end_ ; )	\
		{							\
			if( strncmp( (_p_src_) , "\\\"" , 2 ) == 0 )	\
			{						\
				*(_p_dst_) = '\"' ;			\
				(_p_dst_)++;				\
				(_p_src_) += 2 ;			\
				(_src_len_)++;				\
			}						\
			else						\
			{						\
				*(_p_dst_) = *(_p_src_) ;		\
				(_p_dst_)++;				\
				(_p_src_)++;				\
				(_src_len_)++;				\
			}						\
		}							\
		*(_p_dst_) = '\0' ;					\
	}								\

/* fastjson */

#define FASTERJSON_ERROR_INTERNAL	-11
#define FASTERJSON_ERROR_JSON_INVALID	-12
#define FASTERJSON_INFO_END_OF_BUFFER	13
#define FASTERJSON_ERROR_FILENAME	-14

#define FASTERJSON_NODE_BRANCH		0x10
#define FASTERJSON_NODE_LEAF		0x20
#define FASTERJSON_NODE_ARRAY		0x40
#define FASTERJSON_NODE_ENTER		0x01
#define FASTERJSON_NODE_LEAVE		0x02

typedef int funcCallbackOnJsonNode( int type , char *jpath , int jpath_len , int jpath_size , char *node , int node_len , char *content , int content_len , void *p );
_WINDLL_FUNC int TravelJsonBuffer( char *json_buffer , char *jpath , int jpath_size
				, funcCallbackOnJsonNode *pfuncCallbackOnJsonNode
				, void *p );
_WINDLL_FUNC int TravelJsonBuffer4( char *json_buffer , char *jpath , int jpath_size
				, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonBranch
				, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonBranch
				, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonArray
				, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonArray
				, funcCallbackOnJsonNode *pfuncCallbackOnJsonLeaf
				, void *p );

#ifdef __cplusplus
}
#endif

#endif

