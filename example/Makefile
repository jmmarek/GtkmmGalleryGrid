CC=g++

all: galleryGrid.o
	$(CC) main.cpp galleryGrid.o -o gridSample -I ../lib `pkg-config gtkmm-3.0 --cflags --libs`

galleryGrid.o: ../lib/GalleryGrid.cpp
	$(CC) -c -o galleryGrid.o ../lib/GalleryGrid.cpp `pkg-config gtkmm-3.0 --cflags --libs`
