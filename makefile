all: bouncer.cpp
	g++ -o all bouncer.cpp `pkg-config --cflags --libs libavcodec`
#compile ffmpeg needed libraries???
debug: 
	g++ -o debug -g bouncer.cpp 
#movie: 
# Put the movie code here	


clean:
	rm -f all debug *.cool a.out
