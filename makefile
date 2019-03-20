all: 
	g++ -o all bouncer.cpp
#compile ffmpeg needed libraries???
debug: 
	g++ -o debug -g bouncer.cpp 
#movie: 
# Put the movie code here	


clean:
	rm -f all debug *.cool a.out
