#pragma once

#include <chrono>

#include "cli.hpp"
#include "vault.hpp"

namespace session
{

    class Session
    {
    public:
        explicit Session(vault::Vault vault);
        ~Session();

        void run();

    private:
        void processCommand(
            const cli::Arguments &arguments);

        bool shouldAutoLock() const;

        void updateActivity();

        void lock();

        vault::Vault vault_;

        std::chrono::steady_clock::time_point lastActivity_;

        std::chrono::seconds timeout_;
    };

}