include  ../../make_begin

base_dir=${cur_dir}/../..

CC=g++

lib_dir=${base_dir}/lib

base_SRCS=lock.cpp \
	Thread.cpp \
	ThreadPool.cpp \
	CountDownLatch.cpp 
	
	
base_OBJS=$(base_SRCS:.cpp=.o)

%.o : %.cpp
	$(CC) -c $^ -o $(lib_dir)/$@


msmuduo_base.a: $(base_OBJS)
	cd $(lib_dir) && ar -crvs   $(lib_dir)/$@  $(base_OBJS) 

	
clean:
	rm -rf  $(lib_dir)/msmuduo_base.a
	cd $(lib_dir) && rm -rf $(base_OBJS)
	