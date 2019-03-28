all: bouncer.cpp
	g++ -o all bouncer.cpp `pkg-config --cflags --libs libavcodec libavformat libswscale libavutil`
#compile ffmpeg needed libraries???
debug: bouncer.cpp 
	g++ -o debug -g bouncer.cpp `pkg-config --cflags --libs libavcodec libavformat libswscale libavutil`
movie: 
	ffmpeg -r 60  -i frame%d.cool -pix_fmt rgb24 movie.mp4


clean:
	rm -f all debug *.cool a.out *.mp4
