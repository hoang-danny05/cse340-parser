it: 
	g++ project2.cc lexer.cc inputbuf.cc 

test1:
	make it 
	./a.out < tests/test05.txt 1
	rm ./a.out

test2:
	make it 
	./a.out < tests/test05.txt 2
	rm ./a.out

test3:
	make it 
	./a.out < tests/test05.txt 3
	rm ./a.out

test4:
	make it 
	./a.out < tests/test11.txt 4
	rm ./a.out

test5:
	make it 
	./a.out < tests/test09.txt 5
	rm ./a.out

fulltest:
	make it 
	bash test_01.sh	

finaltest:
	make it 
	bash test_01.sh	

clean:
	rm ./a.out
	rm program

program: 
	g++ -o program project2.cc lexer.cc inputbuf.cc CFG.cc
