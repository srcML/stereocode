// A call on a parameter whose object the caller shares changes the parameter, unless it calls a query method,
// and a call on a global changes the global (Free-Function-Stereotypes wiki, command, global-command, and wrapper)

#include <memory>
#include <string>
#include <vector>

struct Widget {
    int width;
    void reset() { width = 0; }
    int area() const { return width * width; }
};

std::vector<int> g_log;
const std::string g_name = "widget";
const Widget* g_view = nullptr;
Widget* const g_spare = nullptr;

struct Registry {
    static std::vector<int> s_items;
};

// @stereotype command
void resetWidget(Widget& widget) { widget.reset(); }

// @stereotype command
void resetThrough(Widget* widget) { widget->reset(); }

// @stereotype command
void resetFixed(Widget* const widget) { widget->reset(); }

// @stereotype command
void resetFirst(Widget widgets[]) { widgets->reset(); }

// A std::shared_ptr shares its object with the caller, which a call or a write through -> may change, while a call through . uses the copy of the pointer
// @stereotype command
void resetShared(std::shared_ptr<Widget> widget) { widget->reset(); }

// @stereotype command
void resetSharedReference(const std::shared_ptr<Widget>& widget) { widget->reset(); }

// @stereotype command
void clearShared(std::shared_ptr<Widget> widget) { widget->width = 0; }

// @stereotype wrapper
void dropShared(std::shared_ptr<Widget> widget) { widget.reset(); }

// @stereotype wrapper
void resetWeak(std::weak_ptr<Widget> widget) { if (auto shared = widget.lock()) shared->reset(); }

// A std::shared_ptr to a const object and a std::unique_ptr, which the caller hands over, share no object that the function may change
// @stereotype property wrapper
int areaShared(std::shared_ptr<const Widget> widget) { return widget->area(); }

// @stereotype wrapper
void resetOwned(std::unique_ptr<Widget> widget) { widget->reset(); }

// A reference or a pointer to const and a copy share no object that the function may change
// @stereotype property wrapper
int areaOf(const Widget& widget) { return widget.area(); }

// @stereotype property wrapper
int areaThrough(Widget const* widget) { return widget->area(); }

// @stereotype wrapper
void resetCopy(Widget widget) { widget.reset(); }

// A query method reads the parameter
// @stereotype property wrapper
int sizeOf(std::vector<int>& values) { return values.size(); }

// @stereotype global-command wrapper
void appendLog(int value) { g_log.push_back(value); }

// @stereotype global-command wrapper
void addItem(int value) { Registry::s_items.push_back(value); }

// A call on a const global changes no global, unlike a call through a const pointer to an object that is not const
// @stereotype wrapper
std::string shortName() { return g_name.substr(1); }

// @stereotype wrapper
int viewArea() { return g_view->area(); }

// @stereotype global-command wrapper
void resetSpare() { g_spare->reset(); }
