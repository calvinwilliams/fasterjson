#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "fasterjson.h"

static long _GetFileSize(char *filename)
{
	struct stat stat_buf;
	int ret;

	ret=stat(filename,&stat_buf);
	
	if( ret == -1 )
		return -1;
	
	return stat_buf.st_size;
}

static int ReadEntireFile( char *filename , char *mode , char *buf , long *bufsize )
{
	FILE	*fp = NULL ;
	long	filesize ;
	long	lret ;
	
	if( filename == NULL )
		return -1;
	if( strcmp( filename , "" ) == 0 )
		return -1;
	
	filesize = _GetFileSize( filename ) ;
	if( filesize  < 0 )
		return -2;
	
	fp = fopen( filename , mode ) ;
	if( fp == NULL )
		return -3;
	
	if( filesize <= (*bufsize) )
	{
		lret = fread( buf , sizeof(char) , filesize , fp ) ;
		if( lret < filesize )
		{
			fclose( fp );
			return -4;
		}
		
		(*bufsize) = filesize ;
		
		fclose( fp );
		
		return 0;
	}
	else
	{
		lret = fread( buf , sizeof(char) , (*bufsize) , fp ) ;
		if( lret < (*bufsize) )
		{
			fclose( fp );
			return -4;
		}
		
		fclose( fp );
		
		return 1;
	}
}

static int ReadEntireFileSafely( char *filename , char *mode , char **pbuf , long *pbufsize )
{
	long	filesize ;
	
	int	nret ;
	
	filesize = _GetFileSize( filename );
	
	(*pbuf) = (char*)malloc( filesize + 1 ) ;
	if( (*pbuf) == NULL )
		return -1;
	memset( (*pbuf) , 0x00 , filesize + 1 );
	
	nret = ReadEntireFile( filename , mode , (*pbuf) , & filesize ) ;
	if( nret )
	{
		free( (*pbuf) );
		(*pbuf) = NULL ;
		return nret;
	}
	else
	{
		if( pbufsize )
			(*pbufsize) = filesize ;
		return 0;
	}
}

funcCallbackOnJsonNode CallbackOnJsonNode ;
int CallbackOnJsonNode( int type , char *jpath , int jpath_len , int jpath_size , char *nodename , int nodename_len , char *content , int content_len , void *p )
{
	if( type & FASTERJSON_NODE_BRANCH )
	{
		if( type & FASTERJSON_NODE_ENTER )
		{
			printf( "ENTER-BRANCH p[%s] jpath[%.*s] nodename[%.*s]\n" , (char*)p , jpath_len , jpath , nodename_len , nodename );
		}
		else if( type & FASTERJSON_NODE_LEAVE )
		{
			printf( "LEAVE-BRANCH p[%s] jpath[%.*s] nodename[%.*s]\n" , (char*)p , jpath_len , jpath , nodename_len , nodename );
		}
	}
	else if( type & FASTERJSON_NODE_ARRAY )
	{
		if( type & FASTERJSON_NODE_ENTER )
		{
			printf( "ENTER-ARRAY  p[%s] jpath[%.*s] nodename[%.*s]\n" , (char*)p , jpath_len , jpath , nodename_len , nodename );
		}
		else if( type & FASTERJSON_NODE_LEAVE )
		{
			printf( "LEAVE-ARRAY  p[%s] jpath[%.*s] nodename[%.*s]\n" , (char*)p , jpath_len , jpath , nodename_len , nodename );
		}
	}
	else if( type & FASTERJSON_NODE_LEAF )
	{
		printf( "LEAF         p[%s] jpath[%.*s] nodename[%.*s] content[%.*s]\n" , (char*)p , jpath_len , jpath , nodename_len , nodename , content_len , content );
	}
	
	return 0;
}

int main( int argc , char *argv[] )
{
	char	jpath[ 1024 + 1 ] ;
	char	*json_buffer = NULL ;
	char	*p = "hello world" ;
	
	if( argc == 1 + 1 )
	{
		int		nret = 0 ;
		
		nret = ReadEntireFileSafely( argv[1] , "rb" , & json_buffer , NULL ) ;
		if( nret )
		{
			printf( "ReadEntireFileSafely[%s] failed[%d]\n" , argv[1] , nret );
			return nret;
		}
		
		memset( jpath , 0x00 , sizeof(jpath) );
		nret = TravelJsonBuffer( json_buffer , jpath , sizeof(jpath) , & CallbackOnJsonNode , p ) ;
		free( json_buffer );
		if( nret )
		{
			printf( "TravelJsonTree failed[%d]\n" , nret );
			return nret;
		}
	}
	else
	{
		printf( "USAGE : test_fastjson json_pathfilename\n" );
		exit(7);
	}
	
	return 0;
}
