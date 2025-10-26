it: 
	g++ project2.cc lexer.cc inputbuf.cc CFG.cc

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
	rm -f ./a.out*
	make it 
	./a.out < tests/test17.txt 5

test6:
	rm -f ./a.out*
	make it 
	./a.out < tests/test15.txt 6

debug:
	rm -f ./a.out*
	make it 
	#./a.out < tests/test12.txt 7
	./a.out < tests/test16.txt 7

fulltest:
	make it 
	bash test_p2.sh 1 && \
	bash test_p2.sh 2 && \
	bash test_p2.sh 3 && \
	bash test_p2.sh 4 && \
	bash test_p2.sh 5 && \
	bash test_p2.sh 6

finaltest:
	make it 
	bash test_01.sh	

clean:
	rm ./a.out
	rm program

program: 
	g++ -o program project2.cc lexer.cc inputbuf.cc CFG.cc
