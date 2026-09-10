#include "clipboard_timer.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

#include <sodium.h>

#include "clipboard.hpp"

namespace clipboard_timer
{
    namespace
    {
        std::mutex timerMutex;
        std::string activeText;
    }

    void clearAfter(
        const std::string &text,
        unsigned int seconds)
    {
        {
            std::lock_guard<std::mutex> lock(timerMutex);
            activeText = text;
        }

        std::thread(
            [text, seconds]()
            {
                std::this_thread::sleep_for(
                    std::chrono::seconds(seconds));

                try
                {
                    std::lock_guard<std::mutex> lock(timerMutex);
                    if (activeText == text && clipboard::contains(text))
                    {
                        clipboard::clear();
                        sodium_memzero(activeText.data(), activeText.size());
                        activeText.clear();
                    }
                }
                catch (const std::exception &e)
                {
                    std::cerr
                        << "WARNING: "
                        << "Could not clear clipboard: "
                        << e.what()
                        << '\n';
                }
                std::string clearedText = text;
                sodium_memzero(clearedText.data(), clearedText.size());
            })
            .detach();
    }

    void cancel()
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        try
        {
            if (!activeText.empty() && clipboard::contains(activeText))
            {
                clipboard::clear();
            }
        }
        catch (const std::exception &)
        {
            // Locking must still clear the in-memory ownership state.
        }
        sodium_memzero(activeText.data(), activeText.size());
        activeText.clear();
    }

}