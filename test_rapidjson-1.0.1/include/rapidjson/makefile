ALL	=	press_rapidjson

all	:	$(ALL)

.cpp.o	:
	g++ -g -fPIC -Wall -Werror -fno-strict-aliasing -I. -c $<

press_rapidjson	:	press_rapidjson.o
	g++ -g -fPIC -Wall -Werror -fno-strict-aliasing -o $@ press_rapidjson.o

clean	:
	rm $(ALL) *.o

install	:

