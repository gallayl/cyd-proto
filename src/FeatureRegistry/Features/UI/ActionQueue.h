#pragma once

#include <functional>

namespace UI {
    // queue a callback to be executed later on the main UI loop.
    // this is used by interactive widgets so they can request screen
    // changes (or other side‑effects) without mutating the UI hierarchy
    // while it is being traversed. calling code should stash the action
    // before the next touch event is processed by invoking
    // `executeQueuedActions()`.
    void queueAction(std::function<void()> action);

    // execute any callbacks that have been queued. after calling this
    // the queue will be emptied.
    void executeQueuedActions();
} // namespace UI
