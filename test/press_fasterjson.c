#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

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
	return 0;
}

int main( int argc , char *argv[] )
{
	char	jpath[ 1024 + 1 ] ;
	char	*json_buffer = NULL ;
	int	json_len ;
	
	if( argc == 1 + 2 )
	{
		char		*json_buffer_bak = NULL ;
		int		c , count ;
		char		*p = "hello world" ;
		int		nret = 0 ;
		
		nret = ReadEntireFileSafely( argv[1] , "rb" , & json_buffer_bak , NULL ) ;
		if( nret )
		{
			printf( "ReadEntireFileSafely[%s] failed[%d]\n" , argv[1] , nret );
			return nret;
		}
		
		json_buffer = strdup( json_buffer_bak ) ;
		if( json_buffer == NULL )
		{
			printf( "strdup failed , errno[%d]\n" , errno );
			return 1;
		}
		json_len = strlen(json_buffer) ;
		
		count = atol(argv[2]) ;
		if( count > 0 )
		{
			time_t	t1 , t2 ;
			
			time( & t1 );
			for( c = 0 ; c < count ; c++ )
			{
				json_buffer = json_buffer_bak ;
				memset( jpath , 0x00 , sizeof(jpath) );
				nret = TravelJsonBuffer( json_buffer , jpath , sizeof(jpath) , NULL , (void*)p ) ;
				if( nret && nret != FASTERJSON_INFO_END_OF_BUFFER )
				{
					printf( "TravelJsonTree failed[%d]\n" , nret );
					free( json_buffer );
					return nret;
				}
			}
			for( c = 0 ; c < count ; c++ )
			{
				memcpy( json_buffer , json_buffer_bak , json_len );
			}
			time( & t2 );
			printf( "Elapse %d seconds\n" , (int)(t2-t1) );
		}
		else
		{
			int	len ;
			
			count = -count ;
			len = 0 ;
			for( c = 0 ; c < count ; c++ )
			{
				len += strlen( json_buffer_bak + c % 10 ) ;
			}
			printf( "len[%d]\n" , len );
		}
		free( json_buffer );
	}
	else
	{
		printf( "USAGE : press_fastjson json_pathfilename press_count\n" );
		exit(7);
	}
	
	return 0;
}
