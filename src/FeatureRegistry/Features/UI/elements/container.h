#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <algorithm>

// lightweight UI element framework for the touchscreen display
// elements live in the UI namespace so consumers can add their own

namespace UI
{

// base type every element should inherit from. it tracks whether the
// element is currently mounted and provides virtual hooks that can be
// overridden by subclasses.
class Element
{
public:
    virtual ~Element() = default;

    // position and size help with hit testing and drawing
    void setBounds(int ix, int iy, int iw, int ih)
    {
        x = ix;
        y = iy;
        width = iw;
        height = ih;
    }
    void getBounds(int &ox, int &oy, int &ow, int &oh) const
    {
        ox = x;
        oy = y;
        ow = width;
        oh = height;
    }
    bool contains(int px, int py) const
    {
        return px >= x && px < x + width && py >= y && py < y + height;
    }

    // lifecycle ----------------------------------------------------------
    virtual void mount()
    {
        mounted = true;
    }
    virtual void unmount()
    {
        mounted = false;
    }
    bool isMounted() const
    {
        return mounted;
    }

    // drawing hook; derived elements should override and perform
    // rendering when mounted.
    virtual void draw() {}

    // default event callbacks; containers generally forward events to
    // children but a standalone element can override these to react.
    virtual void onTouch(int px, int py) {}
    virtual void onTouchEnd(int px, int py) {}

protected:
    bool mounted{false};
    int x{0}, y{0}, width{0}, height{0};
};

// a container holds zero or more child elements and propagates lifecycle
// changes and touch events. it also allows attaching handlers at the
// container level (for example to implement a button area that spans
// multiple children).
class Container : public Element
{
public:
    using EventHandler = std::function<void(int /*x*/, int /*y*/)>;

    Container() = default;
    ~Container() override
    {
        clear();
    }

    // children management ------------------------------------------------
    // container takes ownership of the child
    void addChild(std::unique_ptr<Element> child)
    {
        if (!child)
            return;
        if (mounted)
        {
            child->mount();
        }
        children.push_back(std::move(child));
    }

    // remove a child by raw pointer; the element will be destroyed
    void removeChild(Element *child)
    {
        if (!child)
            return;
        auto it = std::find_if(children.begin(), children.end(),
                               [&](const std::unique_ptr<Element> &ptr) { return ptr.get() == child; });
        if (it != children.end())
        {
            if (mounted && child->isMounted())
            {
                child->unmount();
            }
            children.erase(it);
        }
    }

    void clear()
    {
        if (mounted)
        {
            for (auto &child : children)
            {
                if (child->isMounted())
                    child->unmount();
            }
        }
        children.clear();
    }

    // provide access to raw pointers for iteration (returns copy to avoid
    // reference invalidation if container is modified during draw)
    std::vector<Element *> getChildren() const
    {
        std::vector<Element *> out;
        out.reserve(children.size());
        for (auto &c : children)
            out.push_back(c.get());
        return out;
    }

    // iteration without allocation; callback must not modify children
    template <typename Fn> void forEachChild(Fn &&fn) const
    {
        for (auto &c : children)
            fn(c.get());
    }

    // drawing -------------------------------------------------------------
    void draw() override
    {
        if (!mounted)
            return;
        for (auto &childPtr : children)
        {
            if (childPtr->isMounted())
                childPtr->draw();
        }
    }

    // mounting/unmounting -------------------------------------------------
    void mount() override
    {
        if (mounted)
            return;
        Element::mount();
        for (auto &childPtr : children)
        {
            childPtr->mount();
        }
    }

    void unmount() override
    {
        if (!mounted)
            return;
        for (auto &childPtr : children)
        {
            childPtr->unmount();
        }
        Element::unmount();
    }

    // event handlers ------------------------------------------------------
    void setTouchHandler(EventHandler h)
    {
        touchHandler = std::move(h);
    }
    void setTouchEndHandler(EventHandler h)
    {
        touchEndHandler = std::move(h);
    }

    void handleTouch(int px, int py)
    {
        if (touchHandler)
            touchHandler(px, py);
        // forward only to children whose bounds contain the point
        for (auto &childPtr : children)
        {
            Element *child = childPtr.get();
            if (child->contains(px, py))
                child->onTouch(px, py);
        }
    }

    void handleTouchEnd(int px, int py)
    {
        if (touchEndHandler)
            touchEndHandler(px, py);
        for (auto &childPtr : children)
        {
            Element *child = childPtr.get();
            if (child->contains(px, py))
                child->onTouchEnd(px, py);
        }
    }

private:
    std::vector<std::unique_ptr<Element>> children;
    EventHandler touchHandler;
    EventHandler touchEndHandler;
};

} // namespace UI
