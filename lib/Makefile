CC=g++

all: galleryGrid.o
	ar rvs galleryGrid.a galleryGrid.o

galleryGrid.o: GalleryGrid.cpp
	$(CC) -c -o galleryGrid.o GalleryGrid.cpp `pkg-config gtkmm-3.0 --cflags --libs`
