#include <gtkmm.h>
#include <iostream>
#include <mutex>

class GalleryGrid final : public Gtk::Box
{
public:
    GalleryGrid (int cellWidth, int cellHeight, int cellsPerRow, std::function<Gtk::Widget*(int)> loadFunction, std::function<void(int)> releaseFunction);
    virtual ~GalleryGrid () {};

    const Glib::RefPtr<Gtk::Adjustment> GetAdjustment() const;
    void ScrollToIndex(int index);
    void SetCapacity(int amount);

    void Reload();
private:
    std::mutex lock;
    Glib::RefPtr<Gtk::Adjustment> adjustment;

    void OnScroll();
    void LoadPhotos(int firstVisibleItem, int lastVisibleItem);

    std::function<Gtk::Widget*(int)> LoadAt;
    std::function<void(int)> ReleaseAt;

    int widgetListSize = 100;
    int firstStoredIndex = 0;
    int lastStoredIndex = 0;
    int lastScrolledIndex = 0;

    Gtk::Layout *layout;
    Gtk::Scrollbar *scrollbar;

    std::vector<Gtk::Widget*> galleryElements;

    int IndexToAxis(int index);
    int IndexToXAxis(int index);
    int AxisToIndex(int axis);
    int AxisCoordsToIndex(int x, int y);

    std::pair<unsigned int, unsigned int> GetTopAndBottomIndices();

    int cellWidth;
    int cellHeight;
    int cellsPerRow;
};
