include  ../../../make_begin

base_dir=${cur_dir}/../../..


 # ${base_dir}/msmuduo/base
BASE_LIST_CPP	= $(shell find   ./../  -name  "*.cpp")
NET_LIST_CPP    = $(shell find ${base_dir}/msmuduo/net -name "*.cpp")

dir:
	@echo $(base_dir)
	@echo $(BASE_LIST_CPP)

CC=g++
INC_PATH= -I${base_dir}  
CFLAGS=-g 
LDFLAGS= ${base_dir}/lib/msmuduo_base.a -lpthread 


BINS=lock_test thread_test ThreadPool_test


PCH_H=stdafx.h
# PCH		=	stdafx.h.gch

all:$(BINS)

$(BINS):
	$(CC) $(CFLAGS) $(INC_PATH) -o $@ $(filter %.cpp,$^) $(LDFLAGS)
	
	
#$(filter <pattern...>,<text> )

clean:
	rm -rf $(BINS) *.o
	
lock_test:lock_test.cpp 

thread_test:thread_test.cpp 

ThreadPool_test:ThreadPool_test.cpp

