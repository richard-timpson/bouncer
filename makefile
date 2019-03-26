all: bouncer.cpp
	g++ -o all bouncer.cpp `pkg-config --cflags --libs libavcodec libavformat libswscale libavutil`
#compile ffmpeg needed libraries???
debug: bouncer.cpp 
	g++ -o debug -g bouncer.cpp `pkg-config --cflags --libs libavcodec libavformat libswscale libavutil`
#movie: 
# Put the movie code here	


clean:
	rm -f all debug *.cool a.out
