#include "GalleryGrid.h"

GalleryGrid::GalleryGrid(int cellWidth, int cellHeight, int cellsPerRow, std::function<Gtk::Widget*(int)> loadFunction, std::function<void(int)> releaseFunction)
    : LoadAt(loadFunction), ReleaseAt(releaseFunction)
{
    this->cellWidth = cellWidth;
    this->cellHeight = cellHeight;
    this->cellsPerRow = cellsPerRow;

    int photoRows = this->cellHeight;
    this->layout = Gtk::manage(new Gtk::Layout());
    this->layout->set_size_request(500, 200);
    this->pack_start(*layout, true, true, 0);

    this->adjustment = Gtk::Adjustment::create(0, 0, photoRows * 100, 1, 20, 400);

    this->scrollbar = Gtk::manage(new Gtk::Scrollbar(adjustment, Gtk::ORIENTATION_VERTICAL));
    this->pack_end(*scrollbar, Gtk::PackOptions::PACK_SHRINK);
    this->scrollbar->show();

    this->layout->add_events(Gdk::SCROLL_MASK);
    this->layout->signal_scroll_event().connect([=](GdkEventScroll* e){
        auto scrollValue = this->adjustment->get_value();
        scrollValue += e->delta_y * 20;
        this->adjustment->set_value(scrollValue);
        return false;
    });
    this->layout->show_all();

    this->adjustment->property_value().signal_changed().connect(sigc::mem_fun(*this, &GalleryGrid::OnScroll));
    this->OnScroll();
}

void GalleryGrid::SetCapacity(int amount)
{
    this->widgetListSize = amount;
    this->adjustment->set_upper(this->IndexToAxis(this->widgetListSize + 4));
}

void GalleryGrid::ScrollToIndex(int index)
{
    this->adjustment->set_value(IndexToAxis(index));
}


const Glib::RefPtr<Gtk::Adjustment> GalleryGrid::GetAdjustment() const
{
    return this->adjustment;
}

void GalleryGrid::Reload()
{
    this->lock.lock();
    for (auto &photo : this->galleryElements) {
        this->layout->remove(*(photo));
    }
    this->lock.unlock();

    this->galleryElements = std::vector<Gtk::Widget*>();

    this->ReleaseAt(-1);

    auto topAndBottomIndex = this->GetTopAndBottomIndices();
    this->LoadPhotos(topAndBottomIndex.first, topAndBottomIndex.second);
}

void GalleryGrid::OnScroll()
{
    auto val = this->adjustment->property_value().get_value();
    int value = (int)val;
    int valueDiff = value - this->lastScrolledIndex;
    this->lastScrolledIndex = value;
    int lastLoadedPhotoPosition = 0;

    // Calculations
    auto topAndBottomIndex = this->GetTopAndBottomIndices();
    int topVisibleIndex = topAndBottomIndex.first;
    int bottomVisibleIndex = topAndBottomIndex.second;

    firstStoredIndex = topVisibleIndex;
    lastStoredIndex = bottomVisibleIndex;

    const int removeIndexSmithLatchValue = 8;
    // Move of existing
    int first_y_axis = std::numeric_limits<int>::max();
    int last_y_axis = 0;
    for (int i = this->galleryElements.size() - 1; i>=0; --i) {
        auto photo = this->galleryElements[i];
        this->lock.lock();
        int position_x = this->layout->child_property_x(*photo);
        int position_y = this->layout->child_property_y(*photo) - valueDiff;
        this->lock.unlock();
        int rowIndex = this->AxisToIndex(position_y + value);
        int index = this->AxisCoordsToIndex(position_x, position_y + value);
        this->lock.lock();
        if (rowIndex < topVisibleIndex - removeIndexSmithLatchValue ||
            rowIndex > bottomVisibleIndex + removeIndexSmithLatchValue) {
            photo->hide();
            this->layout->remove(*(photo));
            this->galleryElements.erase(this->galleryElements.begin()+i);
            this->ReleaseAt(index);
        } else {
            this->layout->move(*(photo), position_x, position_y);
            last_y_axis = std::max(last_y_axis, position_y);
            first_y_axis = std::min(first_y_axis, position_y);
        }
        this->lock.unlock();
    }

    // When big jump occured and everything was removed
    if (first_y_axis > last_y_axis && last_y_axis == 0) {
        this->Reload();
        return;
    }

    // Recalculate y axis to first and last index
    int lastIndex = this->AxisToIndex(last_y_axis + value) + 4;
    int firstIndex = this->AxisToIndex(first_y_axis + value);

    if (topVisibleIndex <= lastIndex && topVisibleIndex >= firstIndex)
    {
        LoadPhotos(lastIndex, bottomVisibleIndex);
    }
    else if (bottomVisibleIndex <= lastIndex && bottomVisibleIndex >= firstIndex)
    {
        LoadPhotos(topVisibleIndex, firstIndex);
    }
    else if (bottomVisibleIndex >= lastIndex && topVisibleIndex <= firstIndex)
    {
        LoadPhotos(topVisibleIndex, firstIndex);
        LoadPhotos(lastIndex, bottomVisibleIndex);
    }
    else
    {
        LoadPhotos(topVisibleIndex, bottomVisibleIndex);
    }
}

void GalleryGrid::LoadPhotos(int firstVisibleItem, int lastVisibleItem)
{
    if (firstVisibleItem >= lastVisibleItem) return;

    auto val = this->adjustment->property_value().get_value();
    int value = (int)val;

    int adjustmentUpper = (int) this->adjustment->get_upper();
    int maxKnownPhotoRow = adjustmentUpper;

    for (int i = std::max(0, firstVisibleItem); i < lastVisibleItem; ++i) {
        if (i > this->widgetListSize)
        {
            break;
        }

        Gtk::Widget *widget = this->LoadAt(i);
        if (widget == nullptr) {
            break;
        }

        int position = IndexToAxis(i) - value;
        int positionX = IndexToXAxis(i);
        this->lock.lock();
        this->layout->put(*widget, positionX, position);
        this->lock.unlock();
        this->galleryElements.push_back(widget);
        widget->show();
        if (IndexToAxis(i + 4) > maxKnownPhotoRow) {
            maxKnownPhotoRow = IndexToAxis(i + 4);
        }
    }

    if (maxKnownPhotoRow > adjustmentUpper)
    {
        //this->adjustment->set_upper(maxKnownPhotoRow);
    }
}

int GalleryGrid::IndexToAxis(int index)
{
    int axis = (index / 4) * this->cellHeight;
    return axis;
}

int GalleryGrid::IndexToXAxis(int index)
{
    int axis = (index % 4) * this->cellWidth;
    return axis;
}

int GalleryGrid::AxisToIndex(int axis)
{
    return 4 * floor(1.0 * axis / this->cellHeight);
}

int GalleryGrid::AxisCoordsToIndex(int x, int y)
{
    int baseIndex = this->AxisToIndex(y);
    baseIndex += x/this->cellWidth;

    return baseIndex;
}

std::pair<unsigned int, unsigned int> GalleryGrid::GetTopAndBottomIndices()
{
    auto val = this->adjustment->property_value().get_value();
    int value = (int)val;

    int topVisibleIndex = 4*floor(std::max(0, this->AxisToIndex(value) - 10)/4);
    int bottomVisibleIndex = 4*floor((this->AxisToIndex(value) + 20)/4);

    return { topVisibleIndex, bottomVisibleIndex };
}
