#include "fasterjson.h"

#ifndef STRCMP
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef MAX
#define MAX(_a_,_b_) (_a_>_b_?_a_:_b_)
#endif

int __FASTERJSON_VERSION_1_0_1 ;

#define FASTERJSON_TOKEN_EOF		-1
#define FASTERJSON_TOKEN_LBB		1	/* { */
#define FASTERJSON_TOKEN_RBB		2	/* } */
#define FASTERJSON_TOKEN_LSB		3	/* [ */
#define FASTERJSON_TOKEN_RSB		4	/* ] */
#define FASTERJSON_TOKEN_COLON		5	/* : */
#define FASTERJSON_TOKEN_COMMA		6	/* , */
#define FASTERJSON_TOKEN_TEXT		9

#define TOKENJSON(_base_,_begin_,_len_,_tag_,_eof_ret_)			\
	do								\
	{								\
		if( (_base_) == NULL )					\
		{							\
			return _eof_ret_;				\
		}							\
		(_tag_) = 0 ;						\
		while(1)						\
		{							\
			for( ; *(_base_) ; (_base_)++ )			\
			{						\
				if( ! strchr( " \t\r\n" , *(_base_) ) )	\
					break;				\
			}						\
			if( *(_base_) == '\0' )				\
			{						\
				return _eof_ret_;			\
			}						\
			else if( (_base_)[0] == '/' && (_base_)[1] == '*' )		\
			{								\
				for( (_base_)+=4 ; *(_base_) ; (_base_)++ )		\
				{							\
					if( (_base_)[0] == '*' && (_base_)[1] == '/' )	\
						break;					\
				}							\
				if( *(_base_) == '\0' )					\
				{							\
					return _eof_ret_;				\
				}							\
				(_base_)+=2;						\
				continue;						\
			}								\
			else if( (_base_)[0] == '/' && (_base_)[1] == '/' )		\
			{								\
				for( (_base_)+=4 ; *(_base_) ; (_base_)++ )		\
				{							\
					if( (_base_)[0] == '\n' )			\
						break;					\
				}							\
				if( *(_base_) == '\0' )					\
				{							\
					return _eof_ret_;				\
				}							\
				(_base_)+=1;						\
				continue;						\
			}								\
			break;						\
		}							\
		if( (_tag_) )						\
			break;						\
		if( (_base_)[0] == '{' )				\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			(_tag_) = FASTERJSON_TOKEN_LBB ;		\
			break;						\
		}							\
		if( (_base_)[0] == '}' )				\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			(_tag_) = FASTERJSON_TOKEN_RBB ;		\
			break;						\
		}							\
		if( (_base_)[0] == '[' )				\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			(_tag_) = FASTERJSON_TOKEN_LSB ;		\
			break;						\
		}							\
		if( (_base_)[0] == ']' )				\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			(_tag_) = FASTERJSON_TOKEN_RSB ;		\
			break;						\
		}							\
		if( (_base_)[0] == ':' )				\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			(_tag_) = FASTERJSON_TOKEN_COLON ;		\
			break;						\
		}							\
		if( (_base_)[0] == ',' )				\
		{							\
			(_begin_) = (_base_) ;				\
			(_len_) = 1 ;					\
			(_base_)++;					\
			(_tag_) = FASTERJSON_TOKEN_COMMA ;		\
			break;						\
		}							\
		(_begin_) = (_base_) ;					\
		if( *(_begin_) == '"' || *(_begin_) == '\'' )		\
		{							\
			(_begin_)++;					\
			(_base_)++;					\
			for( ; *(_base_) ; (_base_)++ )			\
			{						\
				if( (unsigned char)*(_base_) > 127 )	\
					continue;			\
				if( *(_base_) == '\\' )			\
				{					\
					(_base_)++;			\
					continue;			\
				}					\
				if( *(_base_) == *(begin-1) )		\
				{					\
					(_len_) = (_base_) - (_begin_) ;	\
					(_tag_) = FASTERJSON_TOKEN_TEXT ;	\
					(_base_)++;			\
					break;				\
				}					\
			}						\
			if( *(_base_) == '\0' )				\
			{						\
				return _eof_ret_;			\
			}						\
		}							\
		else							\
		{							\
			(_base_)++;					\
			for( ; *(_base_) ; (_base_)++ )			\
			{						\
				if( (unsigned char)*(_base_) > 127 )	\
					continue;			\
				if( strchr( " \t\r\n{}[]:" , *(_base_) ) )	\
				{						\
					(_len_) = (_base_) - (_begin_) ;	\
					(_tag_) = FASTERJSON_TOKEN_TEXT ;	\
					break;				\
				}					\
			}						\
			if( *(_base_) == '\0' )				\
			{						\
				return _eof_ret_;			\
			}						\
		}							\
	}								\
	while(0);							\

static int _TravelJsonArrayBuffer( register char **json_ptr , char *jpath , int jpath_len , int jpath_size
	, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonBranch
	, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonBranch
	 , funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonArray
	 , funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonArray
	, funcCallbackOnJsonNode *pfuncCallbackOnJsonLeaf
	, void *p , char *array_nodename , int array_nodename_len );
static int _TravelJsonLeafBuffer( register char **json_ptr , char *jpath , int jpath_len , int jpath_size
	, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonBranch
	, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonBranch
	 , funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonArray
	 , funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonArray
	, funcCallbackOnJsonNode *pfuncCallbackOnJsonLeaf
	, void *p );
static int _TravelJsonBuffer( register char **json_ptr , char *jpath , int jpath_len , int jpath_size
	, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonBranch
	, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonBranch
	 , funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonArray
	 , funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonArray
	, funcCallbackOnJsonNode *pfuncCallbackOnJsonLeaf
	, void *p );

static int _TravelJsonArrayBuffer( register char **json_ptr , char *jpath , int jpath_len , int jpath_size
	, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonBranch
	, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonBranch
	 , funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonArray
	 , funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonArray
	, funcCallbackOnJsonNode *pfuncCallbackOnJsonLeaf
	, void *p , char *array_nodename , int array_nodename_len )
{
	char		*begin ;
	int		len ;
	signed char	tag ;
	
	char		*content = NULL ;
	int		content_len ;
	
	int		jpath_newlen = 0 ;
	
	int		nret = 0 ;
	
	while(1)
	{
		TOKENJSON(*json_ptr,begin,len,tag,FASTERJSON_INFO_END_OF_BUFFER)
		if( tag == FASTERJSON_TOKEN_LBB )
		{
			if( pfuncCallbackOnEnterJsonBranch )
			{
				nret = (*pfuncCallbackOnEnterJsonBranch)( FASTERJSON_NODE_ENTER | FASTERJSON_NODE_BRANCH , jpath , jpath_len , jpath_size , array_nodename , array_nodename_len , NULL , 0 , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			
			nret = _TravelJsonLeafBuffer( json_ptr , jpath , jpath_len , jpath_size
						, pfuncCallbackOnEnterJsonBranch
						, pfuncCallbackOnLeaveJsonBranch
						, pfuncCallbackOnEnterJsonArray
						, pfuncCallbackOnLeaveJsonArray
						, pfuncCallbackOnJsonLeaf
						, p ) ;
			if( nret )
				return nret;
			
			if( pfuncCallbackOnLeaveJsonBranch )
			{
				nret = (*pfuncCallbackOnLeaveJsonBranch)( FASTERJSON_NODE_LEAVE | FASTERJSON_NODE_BRANCH , jpath , jpath_len , jpath_size , array_nodename , array_nodename_len , NULL , 0 , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
		}
		else if( tag == FASTERJSON_TOKEN_TEXT )
		{
			content = begin ;
			content_len = len ;
			
			if( jpath )
			{
				if( jpath_len + 1 + 1 < jpath_size-1 - 1 )
				{
					sprintf( jpath + jpath_len , "/." );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else if( jpath_len + 1 + 1 <= jpath_size-1 )
				{
					sprintf( jpath + jpath_len , "/*" );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else
				{
					jpath_newlen = jpath_len ;
				}
			}
			
			if( pfuncCallbackOnJsonLeaf )
			{
				nret = (*pfuncCallbackOnJsonLeaf)( FASTERJSON_NODE_LEAF , jpath , jpath_newlen , jpath_size , NULL , 0 , content , content_len , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
		}
		else
		{
			return FASTERJSON_ERROR_JSON_INVALID;
		}
		
		TOKENJSON(*json_ptr,begin,len,tag,FASTERJSON_ERROR_JSON_INVALID)
		if( tag == FASTERJSON_TOKEN_COMMA )
		{
		}
		else if( tag == FASTERJSON_TOKEN_RSB )
		{
			break;
		}
		else if( tag == FASTERJSON_TOKEN_RBB )
		{
			break;
		}
		else
		{
			return FASTERJSON_ERROR_JSON_INVALID;
		}
	}
	
	return 0;
}

static int _TravelJsonLeafBuffer( register char **json_ptr , char *jpath , int jpath_len , int jpath_size
	, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonBranch
	, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonBranch
	, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonArray
	, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonArray
	, funcCallbackOnJsonNode *pfuncCallbackOnJsonLeaf
	, void *p )
{
	char		*begin ;
	int		len ;
	signed char	tag ;
	
	char		*nodename = NULL ;
	int		nodename_len ;
	char		*content = NULL ;
	int		content_len ;
	
	int		jpath_newlen = 0 ;
	
	int		nret = 0 ;
	
	while(1)
	{
		TOKENJSON(*json_ptr,begin,len,tag,FASTERJSON_INFO_END_OF_BUFFER)
		if( tag != FASTERJSON_TOKEN_TEXT )
			return FASTERJSON_ERROR_JSON_INVALID;
		
		nodename = begin ;
		nodename_len = len ;
		
		TOKENJSON(*json_ptr,begin,len,tag,FASTERJSON_ERROR_JSON_INVALID)
		if( tag == FASTERJSON_TOKEN_COLON )
		{
		}
		else if( tag == FASTERJSON_TOKEN_COMMA )
		{
			content = begin ;
			content_len = len ;
			
			if( jpath )
			{
				if( jpath_len + 1 + 1 < jpath_size-1 - 1 )
				{
					sprintf( jpath + jpath_len , "/." );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else if( jpath_len + 1 + 1 <= jpath_size-1 )
				{
					sprintf( jpath + jpath_len , "/*" );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else
				{
					jpath_newlen = jpath_len ;
				}
			}
			
			if( pfuncCallbackOnJsonLeaf )
			{
				nret = (*pfuncCallbackOnJsonLeaf)( FASTERJSON_NODE_LEAF , jpath , jpath_newlen , jpath_size , NULL , 0 , content , content_len , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			
			continue;
		}
		else if( tag == FASTERJSON_TOKEN_RBB )
		{
			content = begin ;
			content_len = len ;
			
			if( jpath )
			{
				if( jpath_len + 1 + 1 < jpath_size-1 - 1 )
				{
					sprintf( jpath + jpath_len , "/." );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else if( jpath_len + 1 + 1 <= jpath_size-1 )
				{
					sprintf( jpath + jpath_len , "/*" );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else
				{
					jpath_newlen = jpath_len ;
				}
			}
			
			if( pfuncCallbackOnJsonLeaf )
			{
				nret = (*pfuncCallbackOnJsonLeaf)( FASTERJSON_NODE_LEAF , jpath , jpath_newlen , jpath_size , NULL , 0 , content , content_len , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			
			break;
		}
		else if( tag == FASTERJSON_TOKEN_RSB )
		{
			content = begin ;
			content_len = len ;
			
			if( jpath )
			{
				if( jpath_len + 1 + 1 < jpath_size-1 - 1 )
				{
					sprintf( jpath + jpath_len , "/." );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else if( jpath_len + 1 + 1 <= jpath_size-1 )
				{
					sprintf( jpath + jpath_len , "/*" );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else
				{
					jpath_newlen = jpath_len ;
				}
			}
			
			if( pfuncCallbackOnJsonLeaf )
			{
				nret = (*pfuncCallbackOnJsonLeaf)( FASTERJSON_NODE_LEAF , jpath , jpath_newlen , jpath_size , NULL , 0 , content , content_len , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			
			break;
		}
		else
		{
			return FASTERJSON_ERROR_JSON_INVALID;
		}
		
		if( jpath )
		{
			if( jpath_len + 1 + nodename_len < jpath_size-1 - 1 )
			{
				sprintf( jpath + jpath_len , "/%.*s" , (int)nodename_len , nodename );
				jpath_newlen = jpath_len + 1 + nodename_len ;
			}
			else if( jpath_len + 1 + 1 <= jpath_size-1 )
			{
				sprintf( jpath + jpath_len , "/*" );
				jpath_newlen = jpath_len + 1 + 1 ;
			}
			else
			{
				jpath_newlen = jpath_len ;
			}
		}
		
		TOKENJSON(*json_ptr,begin,len,tag,FASTERJSON_ERROR_JSON_INVALID)
		if( tag == FASTERJSON_TOKEN_TEXT )
		{
			content = begin ;
			content_len = len ;
			
			if( pfuncCallbackOnJsonLeaf )
			{
				nret = (*pfuncCallbackOnJsonLeaf)( FASTERJSON_NODE_LEAF , jpath , jpath_newlen , jpath_size , nodename , nodename_len , content , content_len , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
		}
		else if( tag == FASTERJSON_TOKEN_LBB )
		{
			if( pfuncCallbackOnEnterJsonBranch )
			{
				nret = (*pfuncCallbackOnEnterJsonBranch)( FASTERJSON_NODE_ENTER | FASTERJSON_NODE_BRANCH , jpath , jpath_newlen , jpath_size , nodename , nodename_len , NULL , 0 , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			
			nret = _TravelJsonLeafBuffer( json_ptr , jpath , jpath_newlen , jpath_size
						, pfuncCallbackOnEnterJsonBranch
						, pfuncCallbackOnLeaveJsonBranch
						, pfuncCallbackOnEnterJsonArray
						, pfuncCallbackOnLeaveJsonArray
						, pfuncCallbackOnJsonLeaf
						, p ) ;
			if( nret )
				return nret;
			
			if( jpath )
			{
				if( jpath_len + 1 + nodename_len < jpath_size-1 - 1 )
				{
					sprintf( jpath + jpath_len , "/%.*s" , (int)nodename_len , nodename );
					jpath_newlen = jpath_len + 1 + nodename_len ;
				}
				else if( jpath_len + 1 + 1 <= jpath_size-1 )
				{
					sprintf( jpath + jpath_len , "/*" );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else
				{
					jpath_newlen = jpath_len ;
				}
			}
			
			if( pfuncCallbackOnLeaveJsonBranch )
			{
				nret = (*pfuncCallbackOnLeaveJsonBranch)( FASTERJSON_NODE_LEAVE | FASTERJSON_NODE_BRANCH , jpath , jpath_newlen , jpath_size , nodename , nodename_len , NULL , 0 , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
		}
		else if( tag == FASTERJSON_TOKEN_LSB )
		{
			nodename = begin ;
			nodename_len = len ;
			
			if( pfuncCallbackOnEnterJsonArray )
			{
				nret = (*pfuncCallbackOnEnterJsonArray)( FASTERJSON_NODE_ENTER | FASTERJSON_NODE_ARRAY , jpath , jpath_newlen , jpath_size , nodename , nodename_len , NULL , 0 , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
			
			nret = _TravelJsonArrayBuffer( json_ptr , jpath , jpath_newlen , jpath_size
						, pfuncCallbackOnEnterJsonBranch
						, pfuncCallbackOnLeaveJsonBranch
						, pfuncCallbackOnEnterJsonArray
						, pfuncCallbackOnLeaveJsonArray
						, pfuncCallbackOnJsonLeaf
						, p , nodename , nodename_len ) ;
			if( nret )
				return nret;
			
			if( jpath )
			{
				if( jpath_len + 1 + nodename_len < jpath_size-1 - 1 )
				{
					sprintf( jpath + jpath_len , "/%.*s" , (int)nodename_len , nodename );
					jpath_newlen = jpath_len + 1 + nodename_len ;
				}
				else if( jpath_len + 1 + 1 <= jpath_size-1 )
				{
					sprintf( jpath + jpath_len , "/*" );
					jpath_newlen = jpath_len + 1 + 1 ;
				}
				else
				{
					jpath_newlen = jpath_len ;
				}
			}
			
			if( pfuncCallbackOnLeaveJsonArray )
			{
				nret = (*pfuncCallbackOnLeaveJsonArray)( FASTERJSON_NODE_LEAVE | FASTERJSON_NODE_ARRAY , jpath , jpath_newlen , jpath_size , nodename , nodename_len , NULL , 0 , p ) ;
				if( nret > 0 )
					break;
				else if( nret < 0 )
					return nret;
			}
		}
		else
		{
			return FASTERJSON_ERROR_JSON_INVALID;
		}
		
		TOKENJSON(*json_ptr,begin,len,tag,FASTERJSON_ERROR_JSON_INVALID)
		if( tag == FASTERJSON_TOKEN_COMMA )
		{
		}
		else if( tag == FASTERJSON_TOKEN_RBB )
		{
			break;
		}
		else
		{
			return FASTERJSON_ERROR_JSON_INVALID;
		}
	}
	
	return 0;
}

static int _TravelJsonBuffer( register char **json_ptr , char *jpath , int jpath_len , int jpath_size
	, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonBranch
	, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonBranch
	 , funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonArray
	 , funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonArray
	, funcCallbackOnJsonNode *pfuncCallbackOnJsonLeaf
	, void *p )
{
	char		*begin ;
	int		len ;
	signed char	tag ;
	
	TOKENJSON(*json_ptr,begin,len,tag,FASTERJSON_INFO_END_OF_BUFFER)
	if( tag == FASTERJSON_TOKEN_LBB )
	{
		return _TravelJsonLeafBuffer( json_ptr , jpath , jpath_len , jpath_size
					, pfuncCallbackOnEnterJsonBranch
					, pfuncCallbackOnEnterJsonBranch
					, pfuncCallbackOnEnterJsonBranch
					, pfuncCallbackOnEnterJsonBranch
					, pfuncCallbackOnEnterJsonBranch
					, p );
	}
	else
	{
		return FASTERJSON_ERROR_JSON_INVALID;
	}
}

int TravelJsonBuffer( char *json_buffer , char *jpath , int jpath_size
		    , funcCallbackOnJsonNode *pfuncCallbackOnJsonNode
		    , void *p )
{
	char		*json_ptr = json_buffer ;
	
	return _TravelJsonBuffer( & json_ptr , jpath , 0 , jpath_size
				, pfuncCallbackOnJsonNode
				, pfuncCallbackOnJsonNode
				, pfuncCallbackOnJsonNode
				, pfuncCallbackOnJsonNode
				, pfuncCallbackOnJsonNode
				, p );
}

int TravelJsonBuffer4( char *json_buffer , char *jpath , int jpath_size
		, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonBranch
		, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonBranch
		, funcCallbackOnJsonNode *pfuncCallbackOnEnterJsonArray
		, funcCallbackOnJsonNode *pfuncCallbackOnLeaveJsonArray
		, funcCallbackOnJsonNode *pfuncCallbackOnJsonLeaf
		, void *p )
{
	char		*json_ptr = json_buffer ;
	
	return _TravelJsonBuffer( & json_ptr , jpath , 0 , jpath_size
				, pfuncCallbackOnEnterJsonBranch
				, pfuncCallbackOnLeaveJsonBranch
				, pfuncCallbackOnEnterJsonArray
				, pfuncCallbackOnLeaveJsonArray
				, pfuncCallbackOnJsonLeaf
				, p );
}

