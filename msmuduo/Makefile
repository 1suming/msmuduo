include ../make_begin

base_dir=${cur_dir}/..


CC=g++
INC_PATH=-I./ -I${base_dir}  
CFLAGS=-g 

PCH_H=stdafx.h
PCH		=	stdafx.h.gch

all=$(PCH) test 

pch:$(PCH)

$(PCH):$(PCH_H)
	$(CC) $(CFLAGS) $^
	
test:test.cpp
	$(CC) $(CFLAGS) -o $@ $^
	
clean:
	rm -rf $(all)
