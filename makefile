all: bouncer.cpp
	g++ -o all bouncer.cpp `pkg-config --cflags --libs libavcodec libavformat libswscale libavutil`
	
movie: 
	ffmpeg -r 30  -i frame%d.cool -pix_fmt rgb24 move.mp4
	ffmpeg -r 30 -i move.mp4 -i Bounce_Audio.mp2 -codec copy  -shortest movie.mp4
	rm -f move.mp4
clean:
	rm -f all debug *.cool a.out *.mp4 *.mp3 *.mp2 audio

# we used this command to compile the code, create the images, make the movie, and play it, all in one command. 
# proved to be useful for debugging. 
run: all
	make all
	./all space.jpg
	rm -f movie.mp4
	make movie
	ffplay movie.mp4