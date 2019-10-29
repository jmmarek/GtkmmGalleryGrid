#include "GalleryGrid.h"
#include <iostream>
#include <map>

static std::map<int, Gtk::Widget*> loadedWidgets;

Gtk::Widget* LoadWidget(int index)
{
    auto newLabel = new Gtk::Label("Label " + std::to_string(index));
    loadedWidgets[index] = newLabel;
    std::cout << "Loading widget " << index << std::endl;
    return newLabel;
}

void FreeWidget(int index)
{
    std::cout << "Deleting widget " << index << std::endl;
    delete loadedWidgets[index];
}

int main(int argc, char **argv) {
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.examples.base");

    Gtk::Window window;
    window.set_default_size(200, 200);

    GalleryGrid galleryGrid(100, 100, 3, LoadWidget, FreeWidget);
    window.add(galleryGrid);
    galleryGrid.show_all();

    return app->run(window);
}
