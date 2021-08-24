#pragma once
#include <string>
#include "Handle.hh"

class Event
{
private:
    AutoCloseHandle m_handle;

public:
    /**
     * Creates or opens a named or unnamed event object.
     * 
     * @param name The name of the event object.
     * The name is limited to MAX_PATH characters. Name comparison is case sensitive.
     * @param initialState If this parameter is TRUE, the initial state of the event object is signaled;
     * otherwise, it is nonsignaled.
     * @param manualReset If this parameter is TRUE, the function creates a manual-reset event object,
     * which requires the use of the ResetEvent function to set the event state to nonsignaled.
     * If this parameter is FALSE, the function creates an auto-reset event object,
     * and system automatically resets the event state to nonsignaled after a single waiting thread has been released.
     */
    bool Create(const std::string &name, bool initialState = false, bool manualReset = false);

    /**
     * Opens an existing named event object.
     * 
     * @param name The name of the event to be opened. Name comparisons are case sensitive.
     */
    bool Open(const std::string &name);

    /**
     * Sets the specified event object to the signaled state.
     */
    bool Signal();

    /**
     * Sets the specified event object to the nonsignaled state.
     */
    bool Reset();
};